#pragma once

#include <sqlite3.h>
#include <string>
#include "inverted_list.hpp"

class DataBase {
 public:
  DataBase(const char *db_path);

 public:
  void create_table(std::string sql_str);

  void create_index(std::string sql_str);

  void bind_stmt(std::string sql_str, sqlite3_stmt *&binded_stmt);

 public:
  std::string get_settings(std::string key);

  int update_settings(std::string key, std::string value);

  int transaction_begin();

  int transaction_end();

  int transaction_rollback();

  int get_word_id(std::string word, unsigned int insert,
                  int *document_word_count);

  std::string get_word(const int word_id);

  int add_document(std::string title, std::string body);

  int get_document_id(std::string title);

  std::string get_document_title(int document_id);

  InvertedList get_inverted_list(const int word_id);

  void update_inverted_list(int word_id, InvertedList &inverted_list);

  void update_document_count(int word_id, int document_count);

  int get_document_count();

private:
  sqlite3 *database_;  //数据库实例

 private:
  // sqllite3的sql语句
  //文档相关语句
  sqlite3_stmt *get_document_id_stmt_;
  sqlite3_stmt *get_document_title_stmt_;
  sqlite3_stmt *insert_document_stmt_;
  sqlite3_stmt *update_document_stmt_;

  sqlite3_stmt *get_document_count_stmt_;

  //关键词相关语句
  sqlite3_stmt *get_word_id_stmt_;
  sqlite3_stmt *get_word_stmt_;
  sqlite3_stmt *store_word_stmt_;
  sqlite3_stmt *update_document_count_stmt_;

  //倒排列表相关语句
  sqlite3_stmt *get_inverted_list_stmt_;
  sqlite3_stmt *replace_inverted_list_stmt_;

  //其他语句
  sqlite3_stmt *get_settings_stmt_;
  sqlite3_stmt *replace_setting_stmt_;
  sqlite3_stmt *begin_stmt_;
  sqlite3_stmt *commit_stmt_;
  sqlite3_stmt *rollback_stmt_;
};