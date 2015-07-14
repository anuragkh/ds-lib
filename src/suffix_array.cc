#include "suffix_array.h"

#include "divsufsortxx.h"

dsl::SuffixArray::SuffixArray() {
  num_elements_ = 0;
  bit_width_ = 0;
  size_ = 0;
  data_ = NULL;
}

dsl::SuffixArray::SuffixArray(const char* input_data, size_t input_size) {
  int64_t *lSA = new int64_t[input_size];
  divsufsortxx::constructSA(input_data, lSA, lSA + input_size, 256);

  num_elements_ = input_size;
  bit_width_ = Utils::int_log_2(input_size + 1);

  for (uint64_t i = 0; i < num_elements_; i++) {
    insert(i, lSA[i]);
  }

  delete[] lSA;
}

dsl::SuffixArray::SuffixArray(const std::string& input)
    : SuffixArray(input.c_str(), input.length()) {
}

dsl::SuffixArray::SuffixArray(uint64_t *suffix_array, size_t size)
    : BitmapArray(suffix_array, size, Utils::int_log_2(size)) {
}
