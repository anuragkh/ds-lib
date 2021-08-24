#include <cstdlib>
#include <cassert>
#include <queue>
#include <string>
#include <mutex>

#define nullnode -1
#define nullval -1

// Comparator; should return:
//     -ve if a < b
//       0 if a = b
//     +ve if a > b
struct DefaultCompare {
  int operator()(int i, int j) {
    return i - j;
  }
};

/* Order Statistics Tree */
template<typename T, typename Compare = DefaultCompare, int BalanceFactor = 4>
class OrderStatisticTree {

 public:
  /* Order Statistic Tree Node */
  struct OSTNode {
    int left;
    int right;
    int size;
    T data;

    /* Constructor */
    explicit OSTNode(T value)
        : size(1),
          left(nullnode),
          right(nullnode) {
      data = value;
    }

    OSTNode()
        : data(nullval),
          size(1),
          left(nullnode),
          right(nullnode) {
    }
  };

  /* Order Statistic Tree Node Functions */
  T GetData(int node) {
    return pool_[node].data;
  }

  void SetData(int node, T val) {
    pool_[node].data = val;
  }

  int GetLeft(int node) {
    return pool_[node].left;
  }

  void SetLeft(int node, int left) {
    pool_[node].left = left;
  }

  int GetRight(int node) {
    return pool_[node].right;
  }

  void SetRight(int node, int right) {
    pool_[node].right = right;
  }

  int GetSize(int node) {
    return (node == nullnode) ? 0 : pool_[node].size;
  }

  void SetSize(int node, int size) {
    pool_[node].size = size;
  }

  int IncSize(int node) {
    return ++(pool_[node].size);
  }

  int DecSize(int node) {
    return --(pool_[node].size);
  }

  int GetWeight(int node) {
    return GetSize(node) + 1;
  }

  int NewNode(int val) {
    std::lock_guard<std::mutex> pool_guard(pool_mtx_);
    pool_.push_back(OSTNode(val));
    return pool_.size() - 1;
  }

//  /* Used for traversing the tree */
//  struct VisitingNode {
//    VisitingNode(int n, int l, int ct)
//        : node(n),
//          level(l),
//          child_type(ct) {
//    }
//
//    int node;
//    int level;
//    int child_type;
//  };

  /* Constructor */
  explicit OrderStatisticTree(const Compare& comp = Compare())
      : root_(nullnode),
        comp_(comp) {
  }

  /* Get the root of the tree */
  int GetRoot() {
    return root_;
  }

  /* Function to check if tree is empty */
  bool IsEmpty() {
    return root_ == nullnode;
  }

  /* Functions to insert data */
  void Insert(T x) {
    root_ = Insert(x, root_);
  }

  int Insert(T x, int node) {
    if (node == nullnode) {
      node = NewNode(x);
    } else {
      int cmp = comp_(x, GetData(node));
      if (cmp < 0) {
        IncSize(node);
        SetLeft(node, Insert(x, GetLeft(node)));

        // Left too heavy - rotate left
        if (GetWeight(GetLeft(node))
            > BalanceFactor * GetWeight(GetRight(node))) {
          node = RotateWithLeftChild(node);
        }
      } else if (cmp > 0) {
        IncSize(node);
        SetRight(node, Insert(x, GetRight(node)));

        // Right too heavy - rotate right
        if (GetWeight(GetRight(node))
            > BalanceFactor * GetWeight(GetLeft(node))) {
          node = RotateWithRightChild(node);
        }
      }
    }
    return node;
  }

  /* Rotate tree node with left child  */
  int RotateWithLeftChild(int k2) {
    int k1 = GetLeft(k2);
    int k3 = GetRight(k1);
    SetLeft(k2, k3);
    SetSize(k2, GetSize(k2) - GetSize(k1) + GetSize(k3));
    SetRight(k1, k2);
    SetSize(k1, GetSize(k1) - GetSize(k3) + GetSize(k2));
    return k1;
  }

  /* Rotate tree node with right child */
  int RotateWithRightChild(int k1) {
    int k2 = GetRight(k1);
    int k3 = GetLeft(k2);
    SetRight(k1, k3);
    SetSize(k1, GetSize(k1) - GetSize(k2) + GetSize(k3));
    SetLeft(k2, k1);
    SetSize(k2, GetSize(k2) - GetSize(k3) + GetSize(k1));
    return k2;
  }

  int Rank(const T x) {
    return Rank(root_, x) - 1;
  }

  int Rank(const int node, const T x) {
    if (node == nullnode)
      return -1;
    else {
      int cmp = comp_(x, GetData(node));
      if (cmp < 0)
        return Rank(GetLeft(node), x);
      else if (cmp == 0)
        return GetSize(GetLeft(node)) + 1;
      else
        return GetSize(GetLeft(node)) + 1 + Rank(GetRight(node), x);
    }
  }

  int Select(const int i) {
    return Select(root_, i + 1);
  }

  int Select(const int node, const int i) {
    if (node == nullnode)
      return -1;

    int left_size = GetSize(GetLeft(node));

    if (i <= left_size)
      return Select(GetLeft(node), i);
    else if (i == left_size + 1)
      return GetData(node);
    else
      return Select(GetRight(node), i - left_size - 1);
  }

//  std::string StringifyNode(VisitingNode& node) {
//    char buf[100];
//    sprintf(buf, "{data: %d, size: %d, %d}", GetData(node.node),
//            GetSize(node.node), node.child_type);
//    return std::string(buf);
//  }
//
//  void PrintTree() {
//    std::queue<VisitingNode> visiting;
//    visiting.push(VisitingNode(root_, 0, -1));
//    int level = 0;
//    while (!visiting.empty()) {
//      VisitingNode head = visiting.front();
//      visiting.pop();
//      if (level != head.level) {
//        level = head.level;
//        fprintf(stderr, "\n");
//      }
//
//      fprintf(stderr, "%s\t", StringifyNode(head).c_str());
//
//      if (GetLeft(head.node) != nullnode) {
//        visiting.push(VisitingNode(GetLeft(head.node), head.level + 1, 0));
//      }
//      if (GetRight(head.node) != nullnode) {
//        visiting.push(VisitingNode(GetRight(head.node), head.level + 1, 1));
//      }
//    }
//  }

 private:
  std::vector<OSTNode> pool_;
  int root_;
  Compare comp_;

  std::mutex pool_mtx_;
};
