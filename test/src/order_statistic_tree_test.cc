#include "order_statistic_tree.h"

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

class OrderStatisticTreeTest : public testing::Test {
 public:
  const int kNumValues = 1024 * 1024;
  typedef OrderStatisticTree<int>::OSTNode OSTNode;

  void CheckTree(OrderStatisticTree<int>& tree) {
    CheckSubtree(tree, tree.GetRoot());
  }

  void CheckSubtree(OrderStatisticTree<int>& tree, int root) {
    if (root == nullnode)
      return;

    int left = tree.GetLeft(root);
    int right = tree.GetRight(root);

    int left_size = 0;
    int right_size = 0;
    if (left != nullnode) {
      ASSERT_TRUE(tree.GetData(left) < tree.GetData(root));
      left_size = tree.GetSize(left);
    }

    if (right != nullnode) {
      ASSERT_TRUE(tree.GetData(root) < tree.GetData(right));
      right_size = tree.GetSize(right);
    }

    ASSERT_EQ(left_size + right_size + 1, tree.GetSize(root));

    CheckSubtree(tree, left);
    CheckSubtree(tree, right);
  }
};

TEST_F(OrderStatisticTreeTest, BasicTest) {

  std::vector<int> values;
  for (int i = 0; i < kNumValues; i++) {
    values.push_back(i * 100);
  }

  fprintf(stderr, "Random shuffling...\n");
  std::random_shuffle(values.begin(), values.end());

  // Check insertions
  OrderStatisticTree<int> tree;
  fprintf(stderr, "Inserting...\n");
  for (auto v : values) {
    tree.Insert(v);
  }

  CheckTree(tree);

  // Check ranks
  fprintf(stderr, "Checking ranks...\n");
  std::sort(values.begin(), values.end());
  for (int i = 0; i < kNumValues; i++) {
    auto v = values.at(i);
    int r = tree.Rank(v);
    ASSERT_EQ(i, r);
  }

  // Check selects
  fprintf(stderr, "Checking selects...\n");
  for (int i = 0; i < kNumValues; i++) {
    auto v = values.at(i);
    int s = tree.Select(i);
    ASSERT_EQ(v, s);
  }
}
