#pragma once

#include <stdint.h>

typedef uint32_t UTF32Char;
#define MAX_UTF8_SIZE 4 //表示用一个utf8来表示一个字符最多要多少字节

int utf8toutf32(const char *str, int str_size, UTF32Char **ustr, int *ustr_len);
char *utf32toutf8(const UTF32Char *ustr, int ustr_len, char *str, int *str_size);
int uchar2utf8_size(const UTF32Char *ustr, int ustr_len);