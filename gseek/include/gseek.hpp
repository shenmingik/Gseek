#pragma once
#include <memory>
#include <string>
#include <vector>
#include "compress.hpp"
#include "database.hpp"
#include "inverted_index.hpp"
#include "utf32.hpp"
#include "global.hpp"

class Gseek {
 public:
  Gseek(int max_document_count, bool enable_phrase_search, const char *db_path);

 public:
  void parse_compress_method(std::string compress_method_str);

  int transaction_begin();

  int transcation_end();

  int transaction_rollback();

  void add_document(const char *title, const char *body);

  int word_cutting(const UTF32Char *utf32_str, const UTF32Char *utf32_str_end,
                   const UTF32Char **retstr_strat);

  void word_to_inverted_index(const int document_id, std::string word,
                              const int word_position);

  void document_to_inverted_index(const int document_id, const UTF32Char *text,
                                  const unsigned int text_len);

  void update_inverted_index();

  std::string get_settings(std::string key);

  void set_created_document_count(int document_count);

  std::vector<SearchResult> search(std::string query_str);

  std::string get_document_title(int document_id);

private:
  const char *gseek_db_path = nullptr;        //数据库的路径
  std::unique_ptr<DataBase> gseek_database_;  //数据库元信息
 private:
  int word_len_;                    //词元长度
  CompressMethod compress_method_;  //倒排索引压缩方法
  bool enable_phrase_search_;       //是否进行短语检索

 private:
  int total_document_count_;  //整个倒排索引中的所有文档数
  int max_document_count_;  //倒排索引项中最大能容量的数，超过这个数量会引发和
  int created_index_document_count_;    //建立了索引的文档数
  InvertedIndex gseek_inverted_index_;  //倒排索引
};