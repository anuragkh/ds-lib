#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "suffix_tree.h"
#include "compressed_suffix_tree.h"

void print_usage(char *exec) {
  fprintf(
  stderr,
          "Usage: %s [-d data-structure] [file]\n", exec);
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4) {
    print_usage(argv[0]);
    return -1;
  }

  int c;
  int data_structure = 0;

  while ((c = getopt(argc, argv, "d:")) != -1) {
    switch (c) {
      case 'd': {
        data_structure = atoi(optarg);
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

  std::ifstream input_stream(input_file);
  const std::string input_text((std::istreambuf_iterator<char>(input_stream)),
                               std::istreambuf_iterator<char>());
  input_stream.close();
  if (data_structure == 0) {
    fprintf(stderr, "Constructing suffix tree...\n");
    dsl::SuffixTree suffix_tree(input_text);
    std::ofstream out(input_file + ".st");
    suffix_tree.serialize(out);
    out.close();
  } else if (data_structure == 1) {
    fprintf(stderr, "Constructing compressed suffix tree...\n");
    dsl::CompressedSuffixTree compressed_suffix_tree(input_text, input_file);
  } else {
    fprintf(stderr, "Data structure %d not supported yet.\n", data_structure);
    exit(0);
  }

  return 0;
}