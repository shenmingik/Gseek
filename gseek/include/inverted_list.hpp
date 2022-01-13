#pragma once
#include <algorithm>
#include <vector>

class InvertedListElement {
 public:
  int document_id_;             //这个倒排列表项的id
  int wordNum_of_occurrences_;  //关键词在document_id中出现的次数
};

class InvertedList {
 public:
  void delete_elem(int document_id);

  int merge_inverted_list(InvertedList &other);

 public:
  /**
   * @brief 尾部插入元素
   *
   * @param[in] element 待插入的元素
   */
  void push_back(InvertedListElement &element) {
    if (is_exist(element.document_id_)) {
      find_elem(element.document_id_).wordNum_of_occurrences_++;
    } else {
      invert_list_.emplace_back(element);
    }
  }

  /**
   * @brief 倒排列表倒序排序
   *
   */
  void sort_desc() {
    std::sort(invert_list_.begin(), invert_list_.end(),
              [](InvertedListElement &elem1, InvertedListElement &elem2) {
                return elem1.wordNum_of_occurrences_ >
                       elem2.wordNum_of_occurrences_;
              });
  }

  /**
   * @brief 判断documentid是否是在这个倒排列表之中
   *
   * @param[in] document_id 文档id
   * @return true 存在
   * @return false 不存在
   */
  bool is_exist(int document_id) {
    std::for_each(invert_list_.begin(), invert_list_.end(),
                  [=](InvertedListElement &elem) {
                    if (elem.document_id_ == document_id) {
                      return true;
                    }
                  });
    return false;
  }

  /**
   * @brief 在倒排列表中找到元素的文档id为document_id的，并返回其引用
   *
   * @param[in] document_id 待查找的文档
   * @return InvertedListElement& 这个倒排列表项的引用
   */
  InvertedListElement &find_elem(int document_id) {
    std::for_each(invert_list_.begin(), invert_list_.end(),
                  [=](InvertedListElement &elem) {
                    if (elem.document_id_ == document_id) {
                      return elem;
                    }
                  });
  }

  /**
   * @brief Get the inverted list object
   *
   * @return const std::vector<InvertedListElement>& 倒排索引
   */
  const std::vector<InvertedListElement> &get_inverted_list() {
    return invert_list_;
  }

  /**
   * @brief 判断倒排列表是否为空
   *
   * @return true 空
   * @return false 非空
   */
  bool is_empty() { return invert_list_.empty(); }

 private:
  int word_total_appers_;  //这个词元在所有文档中出现的次数之和

  std::vector<InvertedListElement> invert_list_;  //出现过这个词元的所有文档信息
};