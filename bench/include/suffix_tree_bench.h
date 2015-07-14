#ifndef DSL_BENCH_SUFFIX_TREE_BENCH_H_
#define DSL_BENCH_SUFFIX_TREE_BENCH_H_

#include <string>
#include <vector>

#include "text/suffix_tree.h"
#include "benchmark.h"

namespace dsl_bench {

class SuffixTreeBench : public Benchmark {
 public:
  /**
   * Constructor for SuffixTree benchmark.
   */
  SuffixTreeBench(const std::string& input_file, bool construct);

  /**
   * Benchmark search operation on SuffixTree.
   */
  void benchSearch(const std::string& query_file,
                   const std::string& result_path) const;

  /**
   * Benchmark count operation on SuffixTree.
   */
  void benchCount(const std::string& query_file,
                  const std::string& result_path) const;

 private:
  dsl::SuffixTree *suffix_tree_;
};

}

#endif /* DSL_BENCH_SUFFIX_TREE_BENCH_H_ */
