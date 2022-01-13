#include "database.hpp"
#include <iostream>
using namespace std;

/**
 * @brief Construct a new Data Base:: Data Base object
 *
 * @param[in] db_path 数据库的路径
 */
DataBase::DataBase(const char* db_path) {
  int return_val = 0;
  if ((return_val = sqlite3_open(db_path, &database_))) {
    cout << "can't open database: " << db_path << endl;
    return;
  }

  // * 创建表settings
  create_table(
      "CREATE TABLE settings("
      "key TEXT PRIMARY KEY,"
      "value TEXT"
      ");");

  // * 创建表documents
  create_table(
      "CREATE TABLE documents("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "title TEXT NOT NULL,"
      "body TEXT NOT NULL"
      ");");

  // * 创建表words
  create_table(
      "CREATE TABLE words("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "word TEXT NOT NULL,"
      "documents_count INT NOT NULL,"
      ");");

  create_table(
      "CREATE TABLE inverted_list("
      "word_id INTERGER,"
      "document_id INTERGER,"
      "wordNum_of_occurrences INTERGER,"
      "PRIMART KEY(word_id,document_id));");

  //* 创建words表的索引
  create_index("CREATE UNIQUE INDEX word_index ON words(word);");

  //* 创建documents表的suoyin
  create_index("CREATE UNIQUE INDEX title_index ON documents(title);");

  //绑定sql stmt语句
  bind_stmt("SELECT id FROM documents WHERE title = ?;", get_document_id_stmt_);
  bind_stmt("SELECT title FROM documents WHERE id = ?;",
            get_document_title_stmt_);
  bind_stmt("INSERT INTO documents (title,body) VALUES (?,?);",
            insert_document_stmt_);
  bind_stmt("UPDATE documents set body = ? WHERE id = ?;",
            update_document_stmt_);

  bind_stmt("SELECT id,documents_count FROM words WHERE word = ?;",
            get_word_id_stmt_);
  bind_stmt("SELECT word from words WHERE id = ?;", get_word_stmt_);
  bind_stmt(
      "INSERT OR IGNORE INTO words (word,documents_count) "
      "VALUES(?,0);",
      store_word_stmt_);
  bind_stmt("UPDATE words SET doucment=? WHERE id = ?;",
            update_document_count_stmt_);

  bind_stmt(
      "SELECT document_id,wordNum_of_occurrences FROM inverted_list WHERE "
      "word_id = ?;",
      get_inverted_list_stmt_);
  bind_stmt(
      "INSERT OR REPLACE INTO inverted_list "
      "(word_id,document_id,wordNum_of_occurrences) VALUES (?,?,?);",
      replace_inverted_list_stmt_);

  bind_stmt("SELECT value FROM settings WHERE key = ?;", get_settings_stmt_);
  bind_stmt("INSERT OR REPLACE INTO settings (key,value) VALUES (?,?);",
            replace_setting_stmt_);

  bind_stmt("SELECT COUNT(*) FROM documents;", get_document_count_stmt_);
  bind_stmt("BEGIN;", begin_stmt_);
  bind_stmt("COMMIT;", commit_stmt_);
  bind_stmt("ROLLBACK;", rollback_stmt_);
}

/**
 * @brief 创建表
 *
 * @param[in] sql_str 建表sql语句
 */
void DataBase::create_table(std::string sql_str) {
  sqlite3_exec(database_, sql_str.c_str(), nullptr, nullptr, nullptr);
}

/**
 * @brief 创建索引
 *
 * @param[in] sql_str 创建索引的语句
 */
void DataBase::create_index(std::string sql_str) {
  sqlite3_exec(database_, sql_str.c_str(), nullptr, nullptr, nullptr);
}

/**
 * @brief 绑定stmt句柄
 *
 * @param[in] sql_str stmt要绑定的语句
 * @param[in] binded_stmt 绑定的句柄函数
 */
void DataBase::bind_stmt(std::string sql_str, sqlite3_stmt*& binded_stmt) {
  sqlite3_prepare(database_, sql_str.c_str(), -1, &binded_stmt, nullptr);
}

/**
 * @brief 得到setting表中的key对应的value值
 *
 * @param[in] key 表中的key值
 * @return std::string key对应的value值
 * @retval "" 失败返回
 */
std::string DataBase::get_settings(std::string key) {
  sqlite3_reset(get_settings_stmt_);
  sqlite3_bind_text(get_settings_stmt_, 1, key.c_str(), key.size(),
                    SQLITE_STATIC);

  int retval = sqlite3_step(get_settings_stmt_);
  if (retval == SQLITE_ROW) {
    return (const char*)sqlite3_column_text(get_settings_stmt_, 0);
  }
  return "";
}

/**
 * @brief 更新setting表中的key对应的value值，如果没有对应的key就会新建一个
 *
 * @param[in] key 表中的key值
 * @param[in] value 表中的value值
 * @return int sqlite3_step的返回值
 */
int DataBase::update_settings(std::string key, std::string value) {
  //绑定replace_setting 句柄函数参数
  sqlite3_reset(replace_setting_stmt_);
  sqlite3_bind_text(replace_setting_stmt_, 1, key.c_str(), key.size(),
                    SQLITE_STATIC);
  sqlite3_bind_text(replace_setting_stmt_, 2, value.c_str(), value.size(),
                    SQLITE_STATIC);

  int return_val;
  //如果繁忙就不停的重试
  while ((return_val = sqlite3_step(replace_setting_stmt_)) == SQLITE_BUSY) {
  }

  switch (return_val) {
    case SQLITE_ERROR: {
      cout << "ERROR: " << sqlite3_errmsg(database_) << endl;
      break;
    }
    case SQLITE_MISUSE: {
      cout << "MISUSE: " << sqlite3_errmsg(database_) << endl;
      break;
    }
    default: { break; }
  }
  return return_val;
}

/**
 * @brief 开启事务
 *
 * @return int sqlite3_step 的返回值
 */
int DataBase::transaction_begin() { return sqlite3_step(begin_stmt_); }

/**
 * @brief 提交事务
 *
 * @return int sqlite3_step 的返回值
 */
int DataBase::transaction_end() { return sqlite3_step(commit_stmt_); }

/**
 * @brief 回滚事务
 *
 * @return int sqlite3_step 的返回值
 */
int DataBase::transaction_rollback() { return sqlite3_step(rollback_stmt_); }

/**
 * @brief 根据文档标题title获得文档编号
 *
 * @param[in] title 文档标题
 * @return int 文档编号
 * @retval 0 失败返回
 */
int DataBase::get_document_id(std::string title) {
  sqlite3_reset(get_document_id_stmt_);
  sqlite3_bind_text(get_document_id_stmt_, 1, title.c_str(), title.size(),
                    SQLITE_STATIC);
  int retval = sqlite3_step(get_document_id_stmt_);
  if (retval == SQLITE_ROW) {
    return sqlite3_column_int(get_document_id_stmt_, 0);
  } else {
    return 0;
  }
}

/**
 * @brief 根据指定的文档标号获得文档标题
 *
 * @param[in] document_id 文档编号
 * @return std::string 成功返回文档标题
 * @retval "" 错误返回
 */
std::string DataBase::get_document_title(int document_id) {
  sqlite3_reset(get_document_title_stmt_);
  sqlite3_bind_int(get_document_title_stmt_, 1, document_id);
  int retval = sqlite3_step(get_document_title_stmt_);

  if (retval == SQLITE_ROW) {
    return (const char*)sqlite3_column_text(get_document_title_stmt_, 0);
  } else {
    return "";
  }
}

/**
 * @brief 将文档添加到数据库表documents，如果文档存在则更新
 *
 * @param[in] title 文档标题
 * @param[in] body 文档内容
 * @return int sqlite_step的返回值
 */
int DataBase::add_document(std::string title, std::string body) {
  sqlite3_stmt* stmt;
  int document_id;
  int retval = 0;

  //如果数据库中有这个文档
  if ((document_id = get_document_id(title))) {
    stmt = update_document_stmt_;
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, body.c_str(), body.size(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, document_id);
  } else {
    //数据库中没有这个
    stmt = insert_document_stmt_;
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, title.c_str(), title.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, body.c_str(), body.size(), SQLITE_STATIC);
  }

  while ((retval = sqlite3_step(stmt)) == SQLITE_BUSY) {
  }

  if (retval == SQLITE_ERROR || retval == SQLITE_MISUSE) {
    cout << "error: " << sqlite3_errmsg(database_);
  }

  return retval;
}

/**
 * @brief 从words表中获取指定词元的id
 *
 * @param[in] word utf8表示的词元
 * @param[in] insert 如果词元没有出现的时候，时候要把该词元添加到表中
 * @param[out] document_word_count 出现过这个词元的文档数
 * @return int 词元的编号
 * @retval 0 失败返回
 */
int DataBase::get_word_id(std::string word, unsigned int insert,
                          int* document_word_count) {
  int retval = 0;
  if (insert) {
    sqlite3_reset(store_word_stmt_);
    sqlite3_bind_text(store_word_stmt_, 1, word.c_str(), word.size(),
                      SQLITE_STATIC);
    retval = sqlite3_step(store_word_stmt_);
  }

  sqlite3_reset(get_word_id_stmt_);
  sqlite3_bind_text(get_word_id_stmt_, 1, word.c_str(), word.size(),
                    SQLITE_STATIC);
  retval = sqlite3_step(get_word_id_stmt_);

  if (retval == SQLITE_ROW) {
    if (document_word_count) {
      *document_word_count = sqlite3_column_int(get_word_id_stmt_, 1);
    }
    return sqlite3_column_int(get_word_id_stmt_, 0);
  } else {
    //失败了
    if (document_word_count) {
      *document_word_count = 0;
    }
    return 0;
  }
}

/**
 * @brief 根据词元编号word_id获取词元
 *
 * @param[in] word_id 词元编号
 * @return std::string 词元
 * @retval "" 失败返回
 */
std::string DataBase::get_word(const int word_id) {
  int retval;
  sqlite3_reset(get_word_stmt_);
  sqlite3_bind_int(get_word_stmt_, 1, word_id);
  retval = sqlite3_step(get_word_stmt_);

  if (retval == SQLITE_ROW) {
    return (const char*)sqlite3_column_text(get_word_stmt_, 0);
  } else {
    return "";
  }
}

/**
 * @brief 得到数据库中的倒排索引信息
 *
 * @param[in] word_id 词元id
 * @return InvertedList 倒排索引
 */
InvertedList DataBase::get_inverted_list(int word_id) {
  sqlite3_reset(get_inverted_list_stmt_);
  sqlite3_bind_int(get_inverted_list_stmt_, 1, word_id);
  int retval = 0;
  InvertedList inverted_list;
  while ((retval = sqlite3_step(get_inverted_list_stmt_)) == SQLITE_ROW) {
    InvertedListElement element;
    element.document_id_ = sqlite3_column_int(get_inverted_list_stmt_, 0);
    element.wordNum_of_occurrences_ =
        sqlite3_column_int(get_inverted_list_stmt_, 1);
    inverted_list.push_back(element);
  }
  return inverted_list;
}

/**
 * @brief 更新数据库的倒排索引
 *
 * @param[in] word_id 词元id
 * @param[in] inverted_list 倒排索引
 */
void DataBase::update_inverted_list(int word_id, InvertedList& inverted_list) {
  auto vec_list = inverted_list.get_inverted_list();
  for (auto it = vec_list.begin(); it != vec_list.end(); it++) {
    const int document_id = it->document_id_;
    const int word_appears = it->wordNum_of_occurrences_;

    sqlite3_reset(replace_inverted_list_stmt_);
    sqlite3_bind_int(replace_inverted_list_stmt_, 1, word_id);
    sqlite3_bind_int(replace_inverted_list_stmt_, 2, document_id);
    sqlite3_bind_int(replace_inverted_list_stmt_, 3, word_appears);

    int retval = 0;
    while ((retval = sqlite3_step(replace_inverted_list_stmt_)) ==
           SQLITE_BUSY) {
    }

    if (retval == SQLITE_ERROR || retval == SQLITE_MISUSE) {
      cout << "errmsg: " << sqlite3_errmsg(database_);
      break;
    }
  }
}

/**
 * @brief 更新words表中document_count的数量
 *
 * @param[in] word_id 单词标号
 * @param[in] document_count 文档数量
 */
void DataBase::update_document_count(int word_id, int document_count) {
  sqlite3_reset(update_document_count_stmt_);
  sqlite3_bind_int(update_document_count_stmt_, 1, document_count);
  sqlite3_bind_int(update_document_count_stmt_, 2, word_id);

  int retval;
  while ((retval = sqlite3_step(update_document_count_stmt_) == SQLITE_BUSY)) {
  }

  switch (retval) {
    case SQLITE_ERROR: {
      cout << "ERROR: " << sqlite3_errmsg(database_) << endl;
      break;
    }
    case SQLITE_MISUSE: {
      cout << "MISUSE: " << sqlite3_errmsg(database_) << endl;
      break;
    }
    default: { break; }
  }
}

/**
 * @brief 得到数据库中的所有文档数量
 * 
 * @param[in] word_id 文档标号
 * @return int 倒排列表中的文档数
 */
int DataBase::get_document_count()
{
  sqlite3_reset(get_document_count_stmt_);
  int retval = sqlite3_step(get_document_count_stmt_);
  if(retval == SQLITE_ROW)
  {
    return sqlite3_column_int(get_document_count_stmt_, 0);
  }
  return -1;
}

