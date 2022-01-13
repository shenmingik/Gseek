#include "utf32.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * 首字节在0x80～0xFF中的UTF-8字符所需的字节数。0表示错误
 **/
const static unsigned char utf8_skip_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 80-8F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 90-9F */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* A0-AF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* B0-BF */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* C0-CF */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* D0-DF */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* E0-EF */
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0, /* F0-FF */
};

/**
 * 计算UTF-8字符串的长度
 * @param[in] str 输入的字符串（UTF-8）
 * @param[in] str_size 输入的字符串的字节数
 * @return UTF-8字符串的长度
 **/
static int utf8_len(const char *str, int str_size) {
  int len = 0;
  const char *str_end;
  for (str_end = str + str_size; str < str_end;) {
    if (*str >= 0) {
      str += 1;
      len++;
    } else {
      unsigned char s = utf8_skip_table[*str + 0x80];
      if (!s) {
        abort();
      }
      str += s;
      len++;
    }
  }
  return len;
}

/**
 * 将错误信息输出到标准错误输出
 * @param[in] format 可以传递给函数printf的格式字符串
 * @param[in] ... 要传递给格式说明符号（format specifications）的参数
 * @return 已输出的字节数
 */
int print_error(const char *format, ...) {
  int r;
  va_list l;

  va_start(l, format);
  vfprintf(stderr, format, l);
  r = fprintf(stderr, "\n");
  fflush(stderr);
  va_end(l);

  return r;
}

/**
 * 计算将字符串的编码由UTF-32转换为UTF-8时所需的字节数
 * @param[in] ustr 输入的字符串（UTF-32）
 * @param[in] ustr_len 输入的字符串的长度
 * @return 用UTF-8表示该字符串时所需的字节数
 **/
int uchar2utf8_size(const UTF32Char *ustr, int ustr_len) {
  int size = 0;
  const UTF32Char *ustr_end;
  for (ustr_end = ustr + ustr_len; ustr < ustr_end; ustr++) {
    if (*ustr < 0x800) {
      if (*ustr < 0x80) {
        size += 1;
      } else {
        size += 2;
      }
    } else {
      if (*ustr < 0x10000) {
        size += 3;
      } else if (*ustr < 0x200000) {
        size += 4;
      } else {
        abort();
      }
    }
  }
  return size;
}

/**
 * 将UTF-8的字符串转换为UTF-32的字符串
 * UTF-32的字符串存储在新分配的缓冲区中
 * @param[in] str 输入的字符串（UTF-8）
 * @param[in] str_size 输入的字符串的字节数。-1表示输入的是以NULL结尾的字符串
 * @param[out] ustr 转换后的字符串（UTF-32）。由调用方释放
 * @param[out] ustr_len 转换后的字符串的长度。调用时可将该参数设为NULL
 * @retval 0 成功
 */
int utf8toutf32(const char *str, int str_size, UTF32Char **ustr,
                int *ustr_len) {
  int ulen;
  ulen = utf8_len(str, str_size);
  if (ustr_len) {
    *ustr_len = ulen;
  }
  if (!ustr) {
    return 0;
  }
  if ((*ustr = (UTF32Char*)malloc(sizeof(UTF32Char) * ulen))) {
    UTF32Char *u;
    const char *str_end;
    for (u = *ustr, str_end = str + str_size; str < str_end;) {
      if (*str >= 0) {
        *u++ = *str;
        str += 1;
      } else {
        unsigned char s = utf8_skip_table[*str + 0x80];
        if (!s) {
          abort();
        }
        /* 从n字节的UTF-8字符的首字节取出后(7 - n)个比特 */
        *u = *str & ((1 << (7 - s)) - 1);
        /* 从n字节的UTF-8字符的剩余字节序列中每次取出6个比特 */
        for (str++, s--; s--; str++) {
          *u = *u << 6;
          *u |= *str & 0x3f;
        }
        u++;
      }
    }
  } else {
    print_error("cannot allocate memory on utf8toutf32.");
  }
  return 0;
}

/**
 * 将指定了长度的UTF-32的字符串转换为以NULL结尾的UTF-8的字符串
 * 需要在调用该函数的地方准备缓冲区，以存放作为转换结果的UTF-8的字符串
 * @param[in] ustr 输入的字符串（UTF-32）
 * @param[in] ustr_len 输入的字符串的长度。-1表示输入的是以NULL结尾的字符串
 * @param[in,out] str 存储转换后的字符串（UTF-8）的缓冲区
 *                    该缓冲区要足够大，不得小于ustr_len * MAX_UTF8_SIZE
 * @param[out] str_size 转换后的字符串的字节数。调用时可将该参数设为NULL
 * @return 转换后的UTF-8字符串
 */
char *utf32toutf8(const UTF32Char *ustr, int ustr_len, char *str,
                  int *str_size) {
  int sbuf_size;
  sbuf_size = uchar2utf8_size(ustr, ustr_len);
  if (str_size) {
    *str_size = sbuf_size;
  }
  if (!str) {
    return NULL;
  } else {
    char *sbuf;
    const UTF32Char *ustr_end;
    for (sbuf = str, ustr_end = ustr + ustr_len; ustr < ustr_end; ustr++) {
      if (*ustr < 0x800) {
        if (*ustr < 0x80) {
          *sbuf++ = *ustr;
        } else {
          *sbuf++ = ((*ustr & 0x7c0) >> 6) | 0xc0;
          *sbuf++ = (*ustr & 0x3f) | 0x80;
        }
      } else {
        if (*ustr < 0x10000) {
          *sbuf++ = ((*ustr & 0xf000) >> 12) | 0xe0;
          *sbuf++ = ((*ustr & 0xfc0) >> 6) | 0x80;
          *sbuf++ = (*ustr & 0x3f) | 0x80;
        } else if (*ustr < 0x200000) {
          *sbuf++ = ((*ustr & 0x1c0000) >> 18) | 0xf0;
          *sbuf++ = ((*ustr & 0x3f000) >> 12) | 0x80;
          *sbuf++ = ((*ustr & 0xfc0) >> 6) | 0x80;
          *sbuf++ = (*ustr & 0x3f) | 0x80;
        } else {
          abort();
        }
      }
    }
    *sbuf = '\0';
  }
  return str;
}
