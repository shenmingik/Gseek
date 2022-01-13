#include "gseek.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include "compress.hpp"
#include "database.hpp"
#include "global.hpp"
#include "gtime.hpp"
#include "utf32.hpp"

using namespace std;

/**
 * @brief 得到TF词频计算因子
 *
 * @param[in] word_frequency 词频
 * @return double TF分值
 */
static double get_tf(int word_frequency) { return 1.0 + log(word_frequency); }

/**
 * @brief 得到IDF词频计算因子
 *
 * @param all_documents 所有的文档数
 * @param word_documents 出现过这个单词的文档数
 * @return double IDF分值
 */
static double get_idf(int all_documents, int word_documents) {
  return log(all_documents / word_documents);
}

/**
 * @brief Construct a new Gseek:: Gseek object
 *
 * @param[in] max_document_count 内存倒排索引文档数的最大值
 * @param[in] enable_phrase_search 是否进行短语检索
 * @param[in] db_path 数据库的路径
 */
Gseek::Gseek(int max_document_count, bool enable_phrase_search,
             const char *db_path)
    : gseek_database_(new DataBase(db_path)) {
  word_len_ = N_GRAM;
  max_document_count_ = max_document_count;
  enable_phrase_search_ = enable_phrase_search;
}

/**
 * @brief 解析传入的压缩方法
 *
 * @param[in] compress_method_str 字符串表示的压缩方法
 */
void Gseek::parse_compress_method(std::string compress_method_str) {
  compress_method_ = Compress::get_instance().get_method(compress_method_str);
  gseek_database_->update_settings("compress_method", compress_method_str);
}

/**
 * @brief 启动Gseek底层的数据库组件事务机制
 *
 * @return int sqlite3_step 的返回值
 */
int Gseek::transaction_begin() { return gseek_database_->transaction_begin(); }

/**
 * @brief 提交事务
 *
 * @return int sqlite3_step 的返回值
 */
int Gseek::transcation_end() { return gseek_database_->transaction_end(); }

/**
 * @brief 回滚事务
 *
 * @return int sqlite3_step 的返回值
 */
int Gseek::transaction_rollback() {
  return gseek_database_->transaction_rollback();
}

/**
 * @brief 设置已完成的
 *
 * @param document_count
 */
void Gseek::set_created_document_count(int document_count) {
  created_index_document_count_ = document_count;
}

/**
 * @brief 分割词元
 *
 * @param[in] utf32_str 输入的字符串
 * @param[in] utf32_str_end 输入字符串的最后位置
 * @param[out] retstr_strat 词元的起始位置
 * @return int 分割出来的词元长度
 */
int Gseek::word_cutting(const UTF32Char *utf32_str,
                        const UTF32Char *utf32_str_end,
                        const UTF32Char **retstr_strat) {
  const UTF32Char *p = utf32_str;
  //跳过忽略字符
  for (; utf32_str < utf32_str_end && gseek_ignored_char(*utf32_str);
       utf32_str++) {
  }

  //取出最多包含n个字符的词元
  for (int i = 0; i < word_len_ && p < utf32_str_end && !gseek_ignored_char(*p);
       i++, p++) {
  }

  *retstr_strat = utf32_str;
  return p - utf32_str;
}

/**
 * @brief 根据传入的词元，更新此词元对应的倒排索引
 *
 * @param document_id 词元出现的文档
 * @param word 词元
 * @param word_position 词元出现的位置(//TODO 后期算法改进会用到)
 */
void Gseek::word_to_inverted_index(const int document_id, std::string word,
                                   const int word_position) {
  int document_word_count;
  int word_id =
      gseek_database_->get_word_id(word, document_id, &document_word_count);

  gseek_inverted_index_.update_inverted_list(word_id, document_id);
}

/**
 * @brief 提取文档body内容，根据分词方法逐步构建倒排索引项
 *
 * @param[in] document_id 文档id
 * @param[in] text 文档内容
 * @param[in] text_len 文档内容的长度
 */
void Gseek::document_to_inverted_index(const int document_id,
                                       const UTF32Char *text,
                                       const unsigned int text_len) {
  int cur_text_len = 0;
  int cur_position = 0;
  const UTF32Char *cur_text = text;
  const UTF32Char *text_end = text + text_len;

  for (; (cur_text_len = word_cutting(cur_text, text_end, &cur_text));
       cur_text++, cur_position++) {
    if (text_len >= word_len_ || document_id) {
      int retval;
      int buffer_size = uchar2utf8_size(text, text_len);
      char text_utf8[buffer_size] = {0};
      int text_utf8_size;

      utf32toutf8(cur_text, cur_text_len, text_utf8, &text_utf8_size);
      word_to_inverted_index(document_id, text_utf8, cur_position);
    }
  }
}

/**
 * @brief 将文档添加到倒排索引中
 *
 * @param[in] title 文档标题
 * @param[in] body 文档正文
 */
void Gseek::add_document(const char *title, const char *body) {
  if (title && body) {
    UTF32Char *body32;
    int body32_len;
    int document_id;
    unsigned int title_size = strlen(title);
    unsigned int body_size = strlen(body);

    //文档添加至数据库
    gseek_database_->add_document(title, body);
    document_id = gseek_database_->get_document_id(title);

    if (!utf8toutf32(body, body_size, &body32, &body32_len)) {
      document_to_inverted_index(document_id, body32, body32_len);
      total_document_count_++;
      created_index_document_count_++;
      free(body32);
    }
  }

  //如果已建立索引的文档数达到阈值就将其更新至硬盘
  if (created_index_document_count_ >= max_document_count_ || !title) {
    cout << "index merge begin!" << endl;
    print_time_between_two_times();
    update_inverted_index();
    print_time_between_two_times();
    cout << "index merge end!" << endl;
  }
}

/**
 * @brief 更新倒排索引至数据库，此时内存中的倒排索引会被销毁
 *
 */
void Gseek::update_inverted_index() {
  std::map<WordId, InvertedList> &invert_index =
      gseek_inverted_index_.get_inverted_index();
  for (auto it = invert_index.begin(); it != invert_index.end(); it++) {
    //得到内存中的倒排索引项
    InvertedList &&inverted_list =
        gseek_database_->get_inverted_list(it->first);
    int document_count = it->second.merge_inverted_list(inverted_list);

    //再次存储至数据库中
    gseek_database_->update_inverted_list(it->first, it->second);

    //更新words表
    gseek_database_->update_document_count(it->first, document_count);

    //删除内存中的元素
    invert_index.erase(it);
  }
}

/**
 * @brief 得到设置中key对应的value
 *
 * @param key key值
 * @return std::string Value值
 */
std::string Gseek::get_settings(std::string key) {
  return gseek_database_->get_settings(key);
}

/**
 * @brief gseek进行搜索并返回根据TF*IDF框架的排序计算结果
 *
 * @param query_str 查询关键词
 * @return std::vector<SearchResult> 计算结果
 */
std::vector<SearchResult> Gseek::search(std::string query_str) {
  vector<SearchResult> ret_vec;

  // 1. 设置当前的文档数
  set_created_document_count(gseek_database_->get_document_count());

  int all_documents = created_index_document_count_;  //总文档数
  int word_documents;  //出现这个查询单词的文档数
  int word_id = gseek_database_->get_word_id(query_str, 0, &word_documents);

  //加载这个查询词的倒排列表至内存中
  InvertedList inverted_list = gseek_database_->get_inverted_list(word_id);
  if (inverted_list.is_empty()) {
    cout << "无此关键词: " << query_str << " 的查询结果";
    return ret_vec;
  }

  //开始根据TF*IDF算法计算文档相关性
  auto real_list = inverted_list.get_inverted_list();
  for (auto it = real_list.begin(); it != real_list.end(); it++) {
    SearchResult result;
    result.document_id = it->document_id_;
    result.score = (get_tf(it->wordNum_of_occurrences_) *
                    get_idf(all_documents, word_documents));
    ret_vec.emplace_back(result);
  }
  sort(ret_vec.begin(), ret_vec.end(),
       [](SearchResult &result1, SearchResult &result2) {
         return result1.score > result2.score;
       });

  return ret_vec;
}

/**
 * @brief 得到文档标题
 *
 * @param document_id 文档id
 * @return std::string 文档标题
 */
std::string Gseek::get_document_title(int document_id) {
  return gseek_database_->get_document_title(document_id);
}
