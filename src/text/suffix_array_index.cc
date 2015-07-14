#include "text/suffix_array_index.h"

dsl::SuffixArrayIndex::SuffixArrayIndex(const char *input, size_t size,
                                        SuffixArray* suffix_array) {
  suffix_array_ = suffix_array;
  input_ = input;
  size_ = size;
}

dsl::SuffixArrayIndex::SuffixArrayIndex(const char *input, size_t size)
    : SuffixArrayIndex(input, size, new dsl::SuffixArray(input, size)) {
}

dsl::SuffixArrayIndex::SuffixArrayIndex(const std::string& input,
                                        SuffixArray* suffix_array)
    : SuffixArrayIndex(input.c_str(), input.length(), suffix_array) {

}

dsl::SuffixArrayIndex::SuffixArrayIndex(const std::string& input)
    : SuffixArrayIndex(input, new dsl::SuffixArray(input)) {
}

int32_t dsl::SuffixArrayIndex::compare(const std::string& query, uint64_t pos) {
  for (uint64_t i = pos, q_pos = 0; i < pos + query.length(); i++, q_pos++) {
    if (input_[i % size_] != query[q_pos])
      return input_[i % size_] - query[q_pos];
  }
  return 0;
}

std::pair<int64_t, int64_t> dsl::SuffixArrayIndex::getRange(
    const std::string& query) {
  int64_t st = size_ - 1;
  int64_t sp = 0;
  int64_t s;
  while (sp < st) {
    s = (sp + st) / 2;
    if (compare(query, suffix_array_->at(s)) > 0)
      sp = s + 1;
    else
      st = s;
  }

  int64_t et = size_ - 1;
  int64_t ep = sp - 1;
  int64_t e;

  while (ep < et) {
    e = ceil((double) (ep + et) / 2);
    if (compare(query, suffix_array_->at(e)) == 0)
      ep = e;
    else
      et = e - 1;
  }

  return std::pair<int64_t, int64_t>(sp, ep);
}

void dsl::SuffixArrayIndex::search(std::vector<int64_t>& results,
                                   const std::string& query) {
  std::pair<int64_t, int64_t> range = getRange(query);
  if (range.second < range.first) {
    return;
  }

  for (uint64_t i = range.second; i <= range.first; i++) {
    results.push_back(suffix_array_->at(i));
  }
}

int64_t dsl::SuffixArrayIndex::count(const std::string& query) {
  std::pair<int64_t, int64_t> range = getRange(query);
  return range.second - range.first + 1;
}

bool dsl::SuffixArrayIndex::contains(const std::string& query) {
  std::pair<int64_t, int64_t> range = getRange(query);
  return (range.second >= range.first);
}
