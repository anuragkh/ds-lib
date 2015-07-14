#include "suffix_tree_bench.h"

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <streambuf>

dsl_bench::SuffixTreeBench::SuffixTreeBench(const std::string& input_file,
                                            bool construct)
    : Benchmark() {
  std::ifstream input_stream(input_file);
  if (construct) {
    const std::string input_text((std::istreambuf_iterator<char>(input_stream)),
                                 std::istreambuf_iterator<char>());
    suffix_tree_ = new dsl::SuffixTree(input_text);

    // Serialize to disk for future use.
    std::ofstream out(input_file + ".st");
    suffix_tree_->serialize(out);
    out.close();
  } else {
    suffix_tree_ = new dsl::SuffixTree();
    suffix_tree_->deserialize(input_stream);
  }
  input_stream.close();
}

void dsl_bench::SuffixTreeBench::benchSearch(
    const std::string& query_file, const std::string& result_path) const {
  std::vector<std::string> queries = readQueryFile(query_file);
  std::ofstream result_stream(result_path);

  for (auto query : queries) {
    std::vector<int64_t> results;
    time_t start = get_timestamp();
    suffix_tree_->search(results, query);
    time_t end = get_timestamp();
    time_t tot = end - start;
    result_stream << results.size() << "\t" << tot << "\n";
    result_stream.flush();
  }

  result_stream.close();
}

void dsl_bench::SuffixTreeBench::benchCount(
    const std::string& query_file, const std::string& result_path) const {
  std::vector<std::string> queries = readQueryFile(query_file);

  std::ofstream result_stream(result_path);

  for (auto query : queries) {
    time_t start = get_timestamp();
    int64_t result = suffix_tree_->count(query);
    time_t end = get_timestamp();
    time_t tot = end - start;
    result_stream << result << "\t" << tot << "\n";
    result_stream.flush();
  }

  result_stream.close();
}

void print_usage(char *exec) {
  fprintf(
      stderr,
      "Usage: %s [-m mode] [-t type] [-q query_file] [-r res_file] [file]\n",
      exec);
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 6) {
    print_usage(argv[0]);
    return -1;
  }

  int c;
  bool construct = true;
  std::string type = "latency-search";
  std::string query_file = "queries.txt";
  std::string res_file = "res.txt";

  while ((c = getopt(argc, argv, "m:t:q:r:")) != -1) {
    switch (c) {
      case 'm': {
        construct = atoi(optarg);
        break;
      }
      case 't': {
        type = std::string(optarg);
        break;
      }
      case 'q': {
        query_file = std::string(optarg);
        break;
      }
      case 'r': {
        res_file = std::string(optarg);
        break;
      }
      default: {
        fprintf(stderr, "Unsupported option %c.\n", (char) c);
        exit(0);
      }
    }
  }

  if (optind == argc) {
    print_usage(argv[0]);
    return -1;
  }

  std::string input_file = std::string(argv[optind]);
  dsl_bench::SuffixTreeBench bench(input_file, construct);

  if (type == "latency-search") {
    bench.benchSearch(query_file, res_file);
  } else if (type == "latency-count") {
    bench.benchCount(query_file, res_file);
  } else {
    fprintf(stderr, "Unsupported type %s.\n", type.c_str());
    exit(0);
  }

  return 0;
}
