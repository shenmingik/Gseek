#pragma once

#include <map>
#include <memory>
#include "inverted_list.hpp"

using WordId = int;

class InvertedIndex {
 public:
  void update_inverted_list(WordId word_id, int document_id);

  std::map<WordId, InvertedList>& get_inverted_index() {
    return inverted_index_;
  }

 private:
  std::map<WordId, InvertedList> inverted_index_;
};