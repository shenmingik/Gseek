#pragma once
#include <functional>

using AddDocumentCallback = std::function<void(const char *title, const char *body)>;

class WebPageLoad {
 public:
     virtual int load_webpage(const char *webpage_path, int max_article_count,
                              AddDocumentCallback callback) = 0;
};