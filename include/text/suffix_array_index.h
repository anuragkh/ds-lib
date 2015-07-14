#ifndef DSL_TEXT_SUFFIX_ARRAY_INDEX_H_
#define DSL_TEXT_SUFFIX_ARRAY_INDEX_H_

#include "text/text_index.h"
#include "suffix_array.h"

namespace dsl {
class SuffixArrayIndex : public TextIndex {
 public:
  SuffixArrayIndex(const std::string& input);
  SuffixArrayIndex(const std::string& input, SuffixArray* suffix_array);
  SuffixArrayIndex(const char* input, size_t size);
  SuffixArrayIndex(const char* input, size_t size, SuffixArray* suffix_array);

  void search(std::vector<int64_t>& results, const std::string& query);
  int64_t count(const std::string& query);
  bool contains(const std::string& query);

  size_t serialize(std::ostream& out);
  size_t deserialize(std::istream& in);
 private:
  std::pair<int64_t, int64_t> getRange(const std::string& query);
  int32_t compare(const std::string& query, uint64_t pos);

  SuffixArray *suffix_array_;
  char* input_;
  size_t size_;
};

}

#endif // DSL_TEXT_SUFFIX_ARRAY_INDEX_H_
