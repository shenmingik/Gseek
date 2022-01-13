#include "inverted_list.hpp"

/**
 * @brief 在倒排列表中删除文档id为document_id的列表项
 *
 * @param[in] document_id 待删除的列表项id
 */
void InvertedList::delete_elem(int document_id) {
  for (auto it = invert_list_.begin(); it != invert_list_.end(); it++) {
    if (it->document_id_ == document_id) {
      it = invert_list_.erase(it);
    }
  }
}

/**
 * @brief 合并两个倒排列表
 *
 * @param[in] other 另一个倒排列表
 * @return int 合并后的文档总数
 */
int InvertedList::merge_inverted_list(InvertedList &other) 
{
  std::vector<InvertedListElement> &inverted_list = other.invert_list_;
  int document_count = inverted_list.size();
  for (auto it = invert_list_.begin(); it != invert_list_.end(); it++)
  {
    //这里的逻辑是如果这个倒排索引项不存在就添加，存在的以内存中为主
    if(!is_exist(it->document_id_))
    {
      document_count++;
      push_back(*it);
    }
  }
}