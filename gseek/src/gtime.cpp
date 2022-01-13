#include "gtime.hpp"
#include <sys/time.h>
#include <time.h>
#include <string>
#include "global.hpp"

#include <iostream>

using namespace std;

/**
 * @brief 将timeval结构表示成浮点数
 *
 * @param[in] tv 传入的timeval结构体
 * @return double 浮点数版本的tv
 */
static double timeval2double(struct timeval& tv) {
  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.000001);
}

/**
 * @brief 将时间结构转换成str返回
 *
 * @param[in] tv
 * @return string
 */
static string timeval2str(struct timeval& tv) {
  char data_buff[TIME_BUFFER] = {0};
  struct tm result;
  localtime_r(&tv.tv_sec, &result);
  sprintf(data_buff, "%04d/%02d/%02d %02d:%02d:%02d.%06d",
          result.tm_year + 1900, result.tm_mon + 1, result.tm_mday,
          result.tm_hour, result.tm_min, result.tm_sec,
          (int)(tv.tv_usec / 100000));
  return data_buff;
}

/**
 * @brief 首次调用打印当前时间，第二次调用打印两次调用时间之差
 * 
 */
void print_time_between_two_times() {
  static double pre_time = 0.0;  //保存上一次调用此函数的时间

  struct timeval tv;
  gettimeofday(&tv, nullptr);

  string time_str = timeval2str(tv);
  double current_time = timeval2double(tv);

  char print_buff[PRINT_BUFF] = {0};
  if (pre_time)
  {
      //打印两次调用此函数之间的差值
      double time_diff = current_time - pre_time;
      sprintf(print_buff, "[time] %s (diff %10.61f)", time_str.c_str(),time_diff);
  }
  else
  {
      //第一次调用pre_time为0，这个时候我们就会打印当前的时间
      sprintf(print_buff, "[time] %s", time_str.c_str());
  }
  cout << print_buff << endl;
  pre_time = current_time;
}
