#include "order_statistic_tree.h"

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

class OrderStatisticTreeTest : public testing::Test {
 public:
  const int kNumValues = 1024 * 1024;

  void CheckTree(OrderStatisticTree<> tree) {
    CheckSubtree(tree.GetRoot());
  }

  void CheckSubtree(OSTNode* root) {
    if (root == NULL)
      return;

    OSTNode* left = root->left;
    OSTNode* right = root->right;

    int left_size = 0;
    int right_size = 0;
    if (left) {
      ASSERT_TRUE(left->data < root->data);
      left_size = left->size;
    }

    if (right) {
      ASSERT_TRUE(root->data < right->data);
      right_size = right->size;
    }

    ASSERT_EQ(left_size + right_size + 1, root->size);

    CheckSubtree(root->left);
    CheckSubtree(root->right);
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
  OrderStatisticTree<> tree;
  fprintf(stderr, "Inserting...\n");
  for (auto v : values) {
    tree.Insert(v);
  }

  CheckTree(tree);

//  // Check searches
//  fprintf(stderr, "Checking searches...\n");
//  for (auto v : values) {
//    bool found = tree.Search(v);
//    ASSERT_TRUE(found);
//  }
//
//  for (auto v : values) {
//    bool found = tree.Search(v + 1);
//    ASSERT_FALSE(found);
//  }

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
