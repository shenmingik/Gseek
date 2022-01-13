#include "wikipage_load.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <expat.h>

using namespace std;

#define BUFF_SIZE 512
#define LOAD_BUFF_SIZE 8192
#define DEFAULT_MAX_ARTICAL_COUNT 128

/**
 * @brief 遇到XML的其实标签时调用的函数
 *
 * @param[in] user_data wiki解析器运行环境
 * @param[in] el XML标签的名字
 * @param attr XML标签的属性列表
 */
static void XMLCALL load_start(void* user_data, const XML_Char* el,
                               const XML_Char* attr[]) {
  WikiStruct* wiki = (WikiStruct*)user_data;
  switch (wiki->wiki_status_) {
    case WEB_DOCUMENT: {
      if (!strcmp(el, "page")) {
        wiki->wiki_status_ = WEB_PAGE;
      }
      break;
    }
    case WEB_PAGE: {
      if (!strcmp(el, "title")) {
        wiki->wiki_status_ = WEB_PAGE_TITLE;
      } else if (!strcmp(el, "id")) {
        wiki->wiki_status_ = WEB_PAGE_ID;
      } else if (!strcmp(el, "revision")) {
        wiki->wiki_status_ = WEB_PAGE_REVISION;
      }
      break;
    }
    case WEB_PAGE_TITLE:
    case WEB_PAGE_ID:
      break;
    case WEB_PAGE_REVISION: {
      if (!strcmp(el, "text")) {
        wiki->wiki_status_ = WEB_PAGE_REVISION_TEXT;
      }
      break;
    }
    case WEB_PAGE_REVISION_TEXT: {
      break;
    }
    default: { break; }
  }
}

/**
 * @brief 遇到XML的结束标签时被调用的函数
 *
 * @param[in] user_data wiki解析器的运行环境
 * @param[in] el XML标签的名字
 */
static void XMLCALL load_end(void* user_data, const XML_Char* el) {
  WikiStruct* wiki = (WikiStruct*)user_data;
  switch (wiki->wiki_status_) {
    case WEB_DOCUMENT: {
      break;
    }
    case WEB_PAGE: {
      if (!strcmp(el, "page")) {
        wiki->wiki_status_ = WEB_DOCUMENT;
      }
      break;
    }
    case WEB_PAGE_TITLE: {
      if (!strcmp(el, "title")) {
        wiki->wiki_status_ = WEB_PAGE;
      }
      break;
    }
    case WEB_PAGE_ID: {
      if (!strcmp(el, "id")) {
        wiki->wiki_status_ = WEB_PAGE;
      }
      break;
    }
    case WEB_PAGE_REVISION: {
      if (!strcmp(el, "revision")) {
        wiki->wiki_status_ = WEB_PAGE;
      }
      break;
    }
    case WEB_PAGE_REVISION_TEXT: {
      if (!strcmp(el, "text")) {
        wiki->wiki_status_ = WEB_PAGE_REVISION;
        if (wiki->max_article_count_ < 0 ||
            wiki->article_count_ < wiki->max_article_count_) {
          wiki->callback_func_(wiki->title_.c_str(), wiki->body_.c_str());
        }

        wiki->article_count_++;
      }
      break;
    }
    default: { break; }
  }
}

/**
 * @brief 解析XML元素中的数据时被调用的函数
 *
 * @param[in] user_data wiki解析器的运行环境
 * @param[in] data 元素中的数据
 * @param[in] data_size 数据的大小
 */
static void XMLCALL element_func(void* user_data, const XML_Char* data,
                                 int data_size) {
  WikiStruct* wiki = (WikiStruct*)user_data;
  switch (wiki->wiki_status_) {
    case WEB_PAGE_TITLE: {
      char buff[BUFF_SIZE] = {0};
      memcpy(buff, data, data_size);
      wiki->title_ = buff;
      break;
    }
    case WEB_PAGE_REVISION_TEXT: {
      char buff[BUFF_SIZE] = {0};
      memcpy(buff, data, data_size);
      wiki->body_ = buff;
      break;
    }

    default:
      break;
  }
}

/**
 * @brief 加载wiki的副本，将其内容传递给指定函数
 *
 * @param[in] webpage_path wiki副本路径
 * @param[in] max_article_count 最多加载词条数
 * @param[in] callback 回调函数
 * @retval 0 成功
 * @retval 1 申请内存失败
 * @retval 2 打开文件失败
 * @retval 3 加载文件失败
 * @retval 4 解析xml文件失败
 */
int WikiPageLoad::load_webpage(const char* webpage_path, int max_article_count,
                             AddDocumentCallback callback) {
  //重置加载器参数
  FILE* fp;               //文件路径
  XML_Parser xml_parser;  // xml解析器
  char buffer[LOAD_BUFF_SIZE] = {0};

  //初始化wiki结构信息
  WikiStruct wiki;
  wiki.wiki_status_ = WEB_DOCUMENT;
  wiki.article_count_ = 0;
  wiki.max_article_count_ = max_article_count;
  wiki.callback_func_ = callback;

  if (!(xml_parser = XML_ParserCreate("UTF-8"))) {
    cout << "can't allocat xml utf-8 parser" << endl;
    return 1;
  }

  if (!(fp = fopen(webpage_path, "rb"))) {
    cout << "can't open file: " << webpage_path << endl;
    XML_ParserFree(xml_parser);
    return 2;
  }

  XML_SetElementHandler(xml_parser, load_start, load_end);
  XML_SetCharacterDataHandler(xml_parser, element_func);
  XML_SetUserData(xml_parser, (void*)&wiki);

  while (true) {
    int buffer_len;
    int done;

    buffer_len = (int)fread(buffer, 1, LOAD_BUFF_SIZE, fp);
    if (ferror(fp)) {
      cout << "wiki dump xml file read error!" << endl;
      fclose(fp);
      XML_ParserFree(xml_parser);
      return 3;
    }

    done = feof(fp);
    if (XML_Parse(xml_parser, buffer, buffer_len, done) == XML_STATUS_ERROR) {
      cout << "wiki dump xml file parse error!" << endl;
      fclose(fp);
      XML_ParserFree(xml_parser);
      return 4;
    }
    if (done ||
        (max_article_count >= 0 && max_article_count <= wiki.article_count_)) {
      break;
    }
  }
  fclose(fp);
  XML_ParserFree(xml_parser);
  return 0;
}