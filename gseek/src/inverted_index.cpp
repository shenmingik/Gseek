#include "inverted_index.hpp"

/**
 * @brief 往word_id代表的倒排列表中插入元素
 *
 * @param[in] word_id 词元id
 * @param[in] document_id 文档id
 */
void InvertedIndex::update_inverted_list(WordId word_id, int document_id) {
  InvertedListElement inverted_list_element;
  inverted_list_element.document_id_ = document_id;
  inverted_list_element.wordNum_of_occurrences_ = 1;

  inverted_index_[word_id].push_back(inverted_list_element);
}