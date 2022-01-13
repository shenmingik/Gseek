#pragma once
#include <unordered_map>
#include "singleton.hpp"

typedef enum compress_method {
  NONE,  //不压缩
  GOLOMB
} CompressMethod;

using CompressMethodStr = std::string;

class Compress : public Singleton<Compress> {
 public:
  Compress() {
    compress_method_["none"] = NONE;
    compress_method_["golomb"] = GOLOMB;
  }

    /**
     * @brief 根据字符化压缩方法得到这个压缩方法的枚举表示
     * 
     * @param compress_method CompressMethod定义的压缩方法
     * @return CompressMethod 
     */
  CompressMethod get_method(CompressMethodStr compress_method_str) {
    return compress_method_[compress_method_str];
  }

  /**
   * @brief 根据compress_method得到其对应的字符串化名称
   *
   * @param compress_method 压缩方法的枚举表示
   * @return CompressMethodStr 压缩方法的字符串表示
   */
  CompressMethodStr get_method_str(CompressMethod compress_method) {
      for (auto it = compress_method_.begin(); it != compress_method_.end();
           it++)
      {
          if (it->second == compress_method)
          {
              return it->first;
          }
      }
  }

 private:
  std::unordered_map<CompressMethodStr, CompressMethod> compress_method_;
};