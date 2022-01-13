#include "global.hpp"
#include "utf32.hpp"

/**
 * @brief 检查字符是否是建立索引的时候需要忽略的字符
 *
 * @param utf32_char 输入的字符
 * @return int 是否是空白字符
 * @retval 0 不是空白字符
 * @retval 1 是空白字符
 */
int gseek_ignored_char(const UTF32Char utf32_char) {
  switch (utf32_char) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case '!':
    case '"':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case ';':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case '\\':
    case ']':
    case '^':
    case '_':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
    case 0x3000: /* 全角空格 */
    case 0x3001: /* 、 */
    case 0x3002: /* 。 */
    case 0xFF08: /* （ */
    case 0xFF09: /* ） */
    case 0xFF01: /* ！ */
    case 0xFF0C: /* ， */
    case 0xFF1A: /* ： */
    case 0xFF1B: /* ； */
    case 0xFF1F: /* ? */
      return 1;
    default:
      return 0;
  }
}
