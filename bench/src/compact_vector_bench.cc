#include "compact_vector.h"

#include <cstdio>
#include <sys/time.h>

typedef unsigned long long int TimeStamp;
static TimeStamp GetTimestamp() {
  struct timeval now{};
  gettimeofday(&now, nullptr);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}

#define ARRAY_SIZE (100*1024*1024)

int main(int argc, char **argv) {
  if (argc > 1) {
    fprintf(stderr, "%s does not take any arguments.\n", argv[0]);
  }

  TimeStamp t0, t1;
  bits::CompactVector<uint64_t, 30> v;

  t0 = GetTimestamp();
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    v.Append(i);
  }
  t1 = GetTimestamp();

  fprintf(stderr, "Time to fill CompactVector = %llu\n", (t1 - t0));

  int64_t sum = 0;
  t0 = GetTimestamp();
  for (size_t i = 0; i < ARRAY_SIZE; i++) {
    // assert(v[i] == i);
    sum += v[i];
  }
  t1 = GetTimestamp();

  fprintf(stderr, "Time to read CompactVector = %llu; sum=%lld\n", (t1 - t0), sum);
}
