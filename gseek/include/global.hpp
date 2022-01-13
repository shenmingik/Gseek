#pragma once
#include "utf32.hpp"

#define PRINT_BUFF 64  //打印缓冲区大小

#define N_GRAM 2  //分割的词元长度

#define DEFAULT_MAX_DOCUMENT_COUNT 2048  //默认最大文件数量

#define MAX_UTF8_SIZE 4  //用UTF-8表示一个unicode字符最多需要多少字节

#define MAX_DOCUMENTS 10  //最多显示的查询结果文档数

typedef struct SearchResult {
  int document_id;  //文档id
  double score;     //检索得分
};

int gseek_ignored_char(const UTF32Char utf32_char);