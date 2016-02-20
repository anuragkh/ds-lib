#include <cstdlib>
#include <cassert>
#include <queue>
#include <string>

/* Order Statistic Tree Node */
struct OSTNode {
  struct OSTNode *left;
  struct OSTNode *right;
  int size, data;

  /* Constructor */
  OSTNode(int value)
      : left(NULL),
        right(NULL) {
    data = value;
    size = 1;
  }

};

/* Order Statistics Tree */
template<int BALANCE_FACTOR = 4>
class OrderStatisticTree {

 public:
  struct VisitingNode {
    VisitingNode(OSTNode* n, int l, int ct)
        : node(n),
          level(l),
          child_type(ct) {
    }

    OSTNode* node;
    int level;
    int child_type;
  };

  /* Constructor */
  OrderStatisticTree() {
    root_ = NULL;
  }

  /* Function to check if tree is empty */
  bool IsEmpty() {
    return root_ == NULL;
  }

  /* Functions to insert data */
  void Insert(int x) {
    root_ = Insert(x, root_);
  }

  OSTNode *Insert(int x, OSTNode *node) {
    if (node == NULL) {
      node = new OSTNode(x);
    } else if (x < node->data) {
      node->size++;
      node->left = Insert(x, node->left);
      // Left too heavy
      if (Weight(node->left) > BALANCE_FACTOR * Weight(node->right)) {
        node = RotateWithLeftChild(node);
      }
    } else if (x > node->data) {
      node->size++;
      node->right = Insert(x, node->right);
      // Right too heavy
      if (Weight(node->right) > BALANCE_FACTOR * Weight(node->left)) {
        node = RotateWithRightChild(node);
      }
    }
    return node;
  }

//  /* Functions to delete data */
//  bool Remove(int x) {
//    if (IsEmpty() || Search(x) == false)
//      return false;
//
//    root_ = Remove(x, root_);
//    return true;
//  }
//
//  OSTNode *Remove(int x, OSTNode *node) {
//    if (node != NULL) {
//      if (x < node->data) {
//        node->left = Remove(x, node->left);
//        node->size--;
//      } else if (x > node->data) {
//        node->right = Remove(x, node->right);
//        node->size--;
//      } else {
//        if (Weight(node->left) > BALANCE_FACTOR * Weight(node->right)) {
//          node = RotateWithLeftChild(node);
//        } else if (Weight(node->right) > BALANCE_FACTOR * Weight(node->left)) {
//          node = RotateWithRightChild(node);
//        }
//        if (node != NULL)
//          node = Remove(x, node);
//        else
//          node->left = NULL;
//      }
//    }
//    return node;
//  }

  /* Rotate tree node with left child  */
  OSTNode *RotateWithLeftChild(OSTNode *k2) {
    OSTNode *k1 = k2->left;
    k2->left = k1->right;
    k2->size = (k2->size - k1->size + Size(k2->left));
    k1->right = k2;
    k1->size = (k1->size - Size(k2->left) + k2->size);
    return k1;
  }

  /* Rotate tree node with right child */
  OSTNode *RotateWithRightChild(OSTNode *k1) {
    OSTNode *k2 = k1->right;
    k1->right = k2->left;
    k1->size = Size(k1->left) + Size(k1->right) + 1;
    k2->left = k1;
    k2->size = Size(k2->left) + Size(k2->right) + 1;
    return k2;
  }

  int Size(OSTNode* node) {
    return node == NULL ? 0 : node->size;
  }

  int Weight(OSTNode* node) {
    return Size(node) + 1;
  }

  /* Functions to count number of nodes */
  int CountNodes() {
    return CountNodes(root_);
  }

  int CountNodes(OSTNode *r) {
    if (r == NULL)
      return 0;
    else {
      int l = 1;
      l += CountNodes(r->left);
      l += CountNodes(r->right);
      return l;
    }
  }

  OSTNode* GetRoot() {
    return root_;
  }

  /* Functions to Search for an element */
  bool Search(int val) {
    return Search(root_, val);
  }

  bool Search(OSTNode *r, int val) {
    bool found = false;
    while ((r != NULL) && !found) {
      int rval = r->data;
      if (val < rval)
        r = r->left;
      else if (val > rval)
        r = r->right;
      else {
        found = true;
        break;
      }
      found = Search(r, val);
    }
    return found;
  }

  int Rank(int x) {
    return Rank(root_, x) - 1;
  }

  int Rank(OSTNode *r, int x) {
    if (r == NULL)
      return -1;
    else if (x < r->data)
      return Rank(r->left, x);
    else if (x == r->data)
      return Size(r->left) + 1;
    else
      return Size(r->left) + 1 + Rank(r->right, x);
  }

  int Select(int i) {
    return Select(root_, i + 1);
  }

  int Select(OSTNode *r, int i) {
    if (r == NULL)
      return -1;

    int left_size = Size(r->left);

    if (i <= left_size)
      return Select(r->left, i);
    else if (i == left_size + 1)
      return r->data;
    else
      return Select(r->right, i - left_size - 1);
  }

  static std::string StringifyNode(VisitingNode& node) {
    char buf[100];
    sprintf(buf, "{val: %d, size: %d, %d}", node.node->data, node.node->size,
            node.child_type);
    return std::string(buf);
  }

  void PrintTree() {
    std::queue<VisitingNode> visiting;
    visiting.push(VisitingNode(root_, 0, -1));
    int level = 0;
    while (!visiting.empty()) {
      auto head = visiting.front();
      visiting.pop();
      if (level != head.level) {
        level = head.level;
        fprintf(stderr, "\n");
      }

      fprintf(stderr, "%s\t", StringifyNode(head).c_str());

      if (head.node->left) {
        visiting.push(VisitingNode(head.node->left, head.level + 1, 0));
      }
      if (head.node->right) {
        visiting.push(VisitingNode(head.node->right, head.level + 1, 1));
      }
    }

  }

 private:

  OSTNode *root_;
}
;
