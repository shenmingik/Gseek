#pragma once
#include <string>
#include "webpage_load.hpp"

typedef enum {
  WEB_DOCUMENT,           //其他状态
  WEB_PAGE,               // page标签
  WEB_PAGE_TITLE,         // page标签中的title标签
  WEB_PAGE_ID,            // page标签中的id标签
  WEB_PAGE_REVISION,      // page标签中的revision标签
  WEB_PAGE_REVISION_TEXT  // page标签中的revision标签中的text标签
} WikiStatus;

typedef struct {
  WikiStatus wiki_status_;  //正在读取词条XML标签的哪一部分
  std::string title_;       //词条标题
  std::string body_;        //词条正文
  int article_count_;       //经过解析的词条总数
  int max_article_count_;   //最多解析的词条数
  AddDocumentCallback callback_func_;  //解析函数后的回调函数
} WikiStruct;

class WikiPageLoad : public WebPageLoad {
 public:
  int load_webpage(const char *webpage_path, int max_article_count,
                   AddDocumentCallback callback);
};