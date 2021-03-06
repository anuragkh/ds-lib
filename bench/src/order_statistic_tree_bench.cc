#include "order_statistic_tree.h"

#include <cstdio>
#include <sys/time.h>

#include <vector>
#include <algorithm>

#define NUM_ELEMENTS 10000000

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

int main(int argc, char** argv) {
  if (argc > 1) {
    fprintf(stderr, "%s does not take any arguments.\n", argv[0]);
  }

  OrderStatisticTree<int> tree;
  {
    std::vector<int> values;

    for (int i = 0; i < NUM_ELEMENTS; i++) {
      values.push_back(i);
    }

    std::random_shuffle(values.begin(), values.end());

    auto t0 = GetTimestamp();
    for (int i = 0; i < values.size(); i++) {
      tree.Insert(values[i]);
    }
    auto t1 = GetTimestamp();
    auto diff = t1 - t0;
    double avg = ((double) diff) / values.size();
    fprintf(stderr,
            "Took %llu time for inserting %zu elements; Average time = %lf\n",
            diff, values.size(), avg);
  }
  {
    long sum = 0;
    auto t0 = GetTimestamp();
    for (int i = 0; i < NUM_ELEMENTS; i++) {
      int r = tree.Rank(i);
      sum += r;
    }
    auto t1 = GetTimestamp();
    auto diff = t1 - t0;
    double avg = ((double) diff) / NUM_ELEMENTS;
    fprintf(
        stderr,
        "Total time = %llu, #Elements = %zu, Average rank time = %lf, sum = %lld\n",
        diff, NUM_ELEMENTS, avg, sum);
  }

  {
    long sum = 0;
    auto t0 = GetTimestamp();
    for (int i = 0; i < NUM_ELEMENTS; i++) {
      int s = tree.Select(i);
      sum += s;
    }
    auto t1 = GetTimestamp();
    auto diff = t1 - t0;
    double avg = ((double) diff) / NUM_ELEMENTS;
    fprintf(
        stderr,
        "Total time = %llu, #Elements = %zu, Average select time = %lf, sum = %lld\n",
        diff, NUM_ELEMENTS, avg, sum);
  }
}
