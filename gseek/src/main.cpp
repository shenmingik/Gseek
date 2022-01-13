#if 1
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <functional>
#include <iostream>
#include "global.hpp"
#include "gseek.hpp"
#include "gtime.hpp"
#include "webpage_load.hpp"
#include "wikipage_load.hpp"

using namespace std::placeholders;
using namespace std;

void print_help() {
  cout << "---------- welcome use gseek ----------" << endl;
  cout << "   -f 文件路径" << endl;
  cout << "   -n 导入的文件数量" << endl;
  cout << "   -z gseek的压缩方式" << endl;
  cout << "   -h 帮助文档" << endl;
  cout << "   -b 设置倒排索引缓冲区大小，默认2048" << endl;
  cout << "   -s 不采用短语检索" << endl;
  cout << "   -q 查询魔数，输入查询短语" << endl;
}

/**
 * @brief gseek程序的入口
 *
 * @param[in] argc 参数个数
 * @param[in] argv 具体参数
 * @return int 成功返回0，失败返回-1
 * @note 输入参数：-p 文件路径 -n 导入的文件数量 -z文件压缩方式 -h 帮助
 */
int main(int argc, char **argv) {
  char *file_path = nullptr;   //导入文件的路径
  int file_num = 0;            //导入的文档数量
  int max_document_count = 0;  //倒排索引能容纳的最大的文件数
  bool enable_phrase_search = true;  //是否进行短语检索

  std::string compress_method_str;  //压缩方法名
  std::string query_str;            //查询的短语

  //# 解析传入字符串

  int ch;
  extern char *optarg;
  extern int optind;
  while ((ch = getopt(argc, argv, "f:n:z:h:b:s:q")) != -1) {
    switch (ch) {
      case 'f': {
        //获取输入文件的路径
        file_path = optarg;
        cout << file_path << endl;
        break;
      }
      case 'n': {
        //获取输入文件的数量
        file_num = atoi(optarg);
        cout << file_num << endl;
        break;
      }
      case 'h': {
        print_help();
        break;
      }
      case 'b': {
        max_document_count = DEFAULT_MAX_DOCUMENT_COUNT;
        break;
      }
      case 's': {
        enable_phrase_search = false;
        break;
      }
      case 'z': {
        compress_method_str = optarg;
        break;
      }
      case 'q': {
        query_str = optarg;
        break;
      }
      default: {
        cout << "invaild optarg!" << endl;
        break;
      }
    }
  }

  struct stat state;
  // stat检测输入的最后一个参数(数据库名称)是否存在
  if (file_path != nullptr && !stat(argv[optind], &state)) {
    cout << argv[optind] << " has exist!" << endl;
    return -1;
  }

  //定义全局环境配置变量
  print_time_between_two_times();
  Gseek gseek(max_document_count, enable_phrase_search, argv[optind]);

  if (file_path) {
    gseek.parse_compress_method(compress_method_str);
    gseek.transaction_begin();

    unique_ptr<WebPageLoad> page_load(new WikiPageLoad());
    if (!page_load->load_webpage(file_path, max_document_count,
                                 bind(&Gseek::add_document, &gseek, _1, _2))) {
      //倒排索引归并至数据库
      gseek.add_document(nullptr, nullptr);
      gseek.transcation_end();
    } else {
      gseek.transaction_rollback();
    }
  }

  if (!query_str.empty()) {
    string compress_method = gseek.get_settings("compress_method");
    gseek.parse_compress_method(compress_method);

    vector<SearchResult> vec = gseek.search(query_str);
    for (int i = 0; i < MAX_DOCUMENTS && i < vec.size(); i++) {
      cout << "document id: " << vec[i].document_id
           << " score: " << vec[i].score
           << " title: " << gseek.get_document_title(vec[i].document_id)
           << endl;
    }
  }
}
#endif