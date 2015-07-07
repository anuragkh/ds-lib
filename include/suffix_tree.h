#ifndef DSL_SUFFIX_TREE_H_
#define DSL_SUFFIX_TREE_H_

#include <vector>
#include <map>
#include <string>

namespace dsl {
const static int64_t CURRENT_END = -1;

namespace suffix_tree {

struct Edge {
  Edge();
  Edge(int64_t start, int64_t end, int64_t origin, int64_t destination);
  int64_t start_index, end_index;
  int64_t origin_node, destination_node;
  int64_t length();
};

struct Node {
  Node();
  int64_t value;
  int64_t suffix_link;  // Points to the longest proper suffix of this node
  std::map<char, Edge> edges;
};

struct ActivePoint {
  ActivePoint();
  int64_t active_node, active_edge, active_length;
};

}

class SuffixTree {
 public:
  SuffixTree(const std::string& input);

  SuffixTree();

  /**
   * Display the Suffix Tree on stdout.
   */
  void show();

  /**
   * Find all occurrences of a substring in the original string.
   * @param query The substring to be searched.
   * @return List of all locations of the substring in the original string.
   */
  std::vector<int64_t> search(const std::string& query);

  /**
   * Count all occurrences of a substring in the original string.
   * @param query The substring to be searched.
   * @return Count of all occurrences of the substring in the original string.
   */
  int64_t count(const std::string& query);

  /**
   * Serialize SuffixTree to output stream.
   * @param out Output stream to serialize to.
   * @return Size of serialized data.
   */
  size_t serialize(std::ostream& out);

  /**
   * Deserialize SuffixTree from input stream.
   * @param in Input stream to deserialize from.
   * @return Size of deserialized data.
   */
  size_t deserialize(std::istream& in);

 private:
  std::vector<suffix_tree::Node> nodes_;
  std::string text_;
  suffix_tree::ActivePoint active_point;

  // Methods for tree construction
  int64_t addLeafNode(int64_t position, int64_t remainder);
  int64_t splitEdge(suffix_tree::Edge& e, int64_t position_to_split,
                    int64_t current_position, int64_t suffix_start);
  bool suffixAlreadyExists(int64_t i);
  char activePointCharacter();

  // Functions to use the active point
  suffix_tree::Edge& activeEdge();
  suffix_tree::Node& activeNode();
  void canonize();

  // Helper functions for search/count
  int64_t findSubtreeRoot(const std::string& query);

  // Helper functions for serialization/deserialization
  size_t serializeNode(suffix_tree::Node& node, std::ostream& out);
  size_t serializeEdge(suffix_tree::Edge& edge, std::ostream& out);
  size_t deserializeNode(suffix_tree::Node& node, std::istream& in);
  size_t deserializeEdge(suffix_tree::Edge& edge, std::istream& in);
};

}
#endif // DSL_SUFFIX_TREE_H_
