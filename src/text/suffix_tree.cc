#include "text/suffix_tree.h"

#include <iostream>
#include <assert.h>
#include <stack>
#include <algorithm>
#include <iterator>

// Edge structure.
dsl::suffix_tree::Edge::Edge(int64_t start, int64_t end, int64_t origin,
                             int64_t destination) {
  start_idx = start;
  end_idx = end;
  src_node = origin;
  dst_node = destination;
  if (end_idx != CURRENT_END)
    assert(end_idx >= start_idx);  //sanity check
}

dsl::suffix_tree::Edge::Edge()
    : start_idx(-1),
      end_idx(-1),
      src_node(-1),
      dst_node(-1) {
}

int64_t dsl::suffix_tree::Edge::length() const {
  return end_idx - start_idx + 1;
}

// Node structure.
dsl::suffix_tree::Node::Node()
    : value(-1),
      suffix_link(-1) {
}

// Active Point structure.
// Uses three ints to represent a particular point on the growing tree.
dsl::suffix_tree::ActivePoint::ActivePoint() {
  active_node = 0;
  active_length = 0;
  active_edge = 0;
}

dsl::SuffixTree::SuffixTree() {
}

/**
 * Constructor for the suffix tree uses ukkonen's algorithm to create the tree
 * in linear time with respect to the sequence length.
 */
dsl::SuffixTree::SuffixTree(const std::string& s)
    : text_(std::string(s + std::string("$"))),
      active_point(suffix_tree::ActivePoint()) {
  nodes_.push_back(suffix_tree::Node());  // The root node.
  int64_t remainder = 0;

  // Main loop of algorithm.
  for (int64_t i = 0; i < text_.length(); i++) {
    remainder += 1;
    int64_t last_inserted = -1;
    int64_t new_inserted = -1;
    bool first = true;

    // Try to add suffixes while there's still suffixes to add at the current
    // position i in the string.
    while (remainder > 0) {
      // Check if the suffix is already contained in the tree implicitly by
      // checking if the new character appears immediately after the active
      // point.
      if (suffixAlreadyExists(i)) {
        // If it does, move the active point up 1 and end current step. If
        // there was already an internal node marked this step as needing a
        // suffix link, link it to the active node.
        if (active_point.active_node != 0 && !first) {
          nodes_[new_inserted].suffix_link = active_point.active_node;
        }
        if (active_point.active_edge == 0)
          active_point.active_edge = i;
        active_point.active_length += 1;
        canonize();
        break;
      }

      // If we get to here, we didn't find the suffix so it's time to update
      // the tree. We add a new leaf to the tree
      // If the leaf is added to an internal node, the internal node gets a
      // suffix link to the last internal node that got a leaf added (rule 2).
      // Insert a leaf node directly if active point is at a node.
      if (active_point.active_edge == 0) {
        addLeafNode(i, remainder);
        if (active_point.active_node != 0) {
          last_inserted = new_inserted;
          new_inserted = active_point.active_node;
          if (!first) {  //rule 2
            nodes_[last_inserted].suffix_link = active_point.active_node;
          }
        }
      }

      // If the active point is on an edge, split the edge and add a leaf to
      // the new internal node.
      else {
        // Sanity check.
        assert(active_point.active_edge != 0 && active_point.active_length > 0);
        last_inserted = new_inserted;
        new_inserted = splitEdge(
            activeEdge(), activeEdge().start_idx + active_point.active_length,
            i, i - remainder + 1);
        if (!first) {  //rule 2
          nodes_[last_inserted].suffix_link = new_inserted;
        }
      }

      // Now that we've added a new node, move the active point to the next
      // shorter suffix; if we're at root, this is done by decrementing active
      // length (rule 1); if we we're at a non-root node, this is done by
      // following a suffix link (rule 3).
      if (active_point.active_node == 0 && active_point.active_length > 0) {
        active_point.active_length -= 1;
        if (active_point.active_length == 0)
          active_point.active_edge = 0;
        else
          active_point.active_edge += 1;
        canonize();
      } else if (active_point.active_node != 0) {
        active_point.active_node =
            activeNode().suffix_link == -1 ? 0 : activeNode().suffix_link;
        canonize();
      }
      first = false;
      remainder -= 1;
    }
  }
}

// Print a representation of the suffix tree for debugging.
void dsl::SuffixTree::show() {
  std::cout << "Original text: [" << text_ << "]\n";
  std::cout << "Text size = %zu\n" << text_.length() << std::endl;
  std::cout << "Num nodes = %zu\n" << nodes_.size() << std::endl;
  for (int64_t i = 0; i < nodes_.size(); i++) {
    if (!nodes_[i].edges.empty()) {
      std::cout << "node " << i;
      if (nodes_[i].suffix_link != -1)
        std::cout << " - - - " << nodes_[i].suffix_link;
      std::cout << std::endl;
      for (auto it = nodes_[i].edges.begin(); it != nodes_[i].edges.end();
          ++it) {
        int64_t start = it->second.start_idx;
        int64_t end = it->second.end_idx;
        if (end == CURRENT_END)
          end = text_.size() - 1;
        std::cout << "[" << it->first << "] : ["
            << text_.substr(start, end - start + 1) << "]";

        // Edge does not point to leaf node.
        if (it->second.end_idx != CURRENT_END)
          std::cout << " -> " << it->second.dst_node;
        else
          std::cout << "(" << nodes_[it->second.dst_node].value << ")";
        std::cout << std::endl;
      }
    }
  }
}

uint64_t dsl::SuffixTree::numNodes() {
  return nodes_.size();
}

bool dsl::SuffixTree::suffixAlreadyExists(int64_t i) {
  if (active_point.active_edge == 0)
    return nodes_[active_point.active_node].edges.count(text_[i]) == 1;
  else
    return text_[i] == activePointCharacter();
}

// Add a new leaf node to the active node.
int64_t dsl::SuffixTree::addLeafNode(int64_t position, int64_t remainder) {
  nodes_.push_back(suffix_tree::Node());
  nodes_[active_point.active_node].edges.insert(
      std::make_pair(
          text_[position],
          dsl::suffix_tree::Edge(position, CURRENT_END,
                                 active_point.active_node, nodes_.size() - 1)));
  // Add index where the suffix represented by this leaf started.
  nodes_[nodes_.size() - 1].value = position - remainder + 1;
  return nodes_.size() - 1;
}

int64_t dsl::SuffixTree::splitEdge(dsl::suffix_tree::Edge& e,
                                   int64_t position_to_split,
                                   int64_t current_position,
                                   int64_t suffix_start) {

  // Make a new internal node.
  nodes_.push_back(suffix_tree::Node());
  suffix_tree::Edge old_edge = e;

  // Edit the edge to be split to go from active node to new internal node.
  e.dst_node = nodes_.size() - 1;
  e.end_idx = position_to_split - 1;

  // Add an edge from the internal node to the orphan leaf node.
  nodes_[nodes_.size() - 1].edges.insert(
      std::make_pair(
          text_[position_to_split],
          dsl::suffix_tree::Edge(position_to_split, old_edge.end_idx,
                                 nodes_.size() - 1, old_edge.dst_node)));

  // Add a new leaf node to the internal node representing the repeated
  // character.
  nodes_.push_back(suffix_tree::Node());
  nodes_[nodes_.size() - 2].edges.insert(
      std::make_pair(
          text_[current_position],
          dsl::suffix_tree::Edge(current_position, CURRENT_END,
                                 nodes_.size() - 2, nodes_.size() - 1)));
  nodes_[nodes_.size() - 1].value = suffix_start;

  // Return the address of the internal node.
  return e.dst_node;
}

dsl::suffix_tree::Edge& dsl::SuffixTree::activeEdge() {
  return nodes_[active_point.active_node].edges.at(
      text_[active_point.active_edge]);
}

dsl::suffix_tree::Node& dsl::SuffixTree::activeNode() {
  return nodes_[active_point.active_node];
}

// Character immediately AFTER the active point
char dsl::SuffixTree::activePointCharacter() {
  if (active_point.active_edge == 0)
    return 0;
  return text_[nodes_[active_point.active_node].edges.at(
      text_[active_point.active_edge]).start_idx + active_point.active_length];
}

// Fixes the active point when active_length grows beyond the bounds of the
// active_edge.
void dsl::SuffixTree::canonize() {
  if (active_point.active_edge == 0)
    return;
  if (activeEdge().end_idx == CURRENT_END)
    return;
  while (activeEdge().start_idx + active_point.active_length
      > activeEdge().end_idx) {
    int64_t increment = activeEdge().end_idx - activeEdge().start_idx + 1;
    active_point.active_node = activeEdge().dst_node;
    active_point.active_length -= increment;
    if (active_point.active_length > 0)
      active_point.active_edge += increment;
    else {
      active_point.active_edge = 0;
      return;
    }
    if (active_point.active_edge != 0)
      if (activeEdge().end_idx == CURRENT_END)
        return;
    assert(active_point.active_edge >= 0);
  }
}

// Returns the root node of the sub-tree which represents all the suffixes of
// the query or -1 if query not found.
int64_t dsl::SuffixTree::findSubtreeRoot(const std::string& query) const {
  suffix_tree::ActivePoint active_point = suffix_tree::ActivePoint();
  for (int64_t i = 0; i < query.size(); i++) {
    if (active_point.active_edge == 0) {  // We are on a node
      if (nodes_[active_point.active_node].edges.count(query[i]) == 0)
        return -1;
      else
        active_point.active_edge = i;
      active_point.active_length += 1;
    } else {  // We are on an edge
      if (!(query[i]
          == text_[nodes_[active_point.active_node].edges.at(
              query[active_point.active_edge]).start_idx
              + active_point.active_length])) {
        return -1;
      } else {
        active_point.active_length += 1;
      }
    }
    // If we reached the end of an edge, hop to the node
    if (active_point.active_length
        >= nodes_[active_point.active_node].edges.at(
            query[active_point.active_edge]).length()) {
      active_point.active_node = nodes_[active_point.active_node].edges.at(
          query[active_point.active_edge]).dst_node;
      active_point.active_length = active_point.active_edge = 0;
    }
  }
  if (active_point.active_edge == 0)
    return active_point.active_node;
  else {
    return nodes_[active_point.active_node].edges.at(
        query[active_point.active_edge]).dst_node;
  }
}

std::vector<int64_t> dsl::SuffixTree::search(const std::string& query) const {
  int64_t subtree_root = findSubtreeRoot(query);
  std::stack<int64_t> stack;
  std::vector<int64_t> values;

  if (subtree_root == -1) {
    return values;
  }

  // Traverse the sub tree to find the leaves. Their labels give the positions
  // of the query in the full string.
  stack.push(subtree_root);
  while (!stack.empty()) {
    int64_t current_node = stack.top();
    stack.pop();
    // If node has no children it is a leaf. Save its start position.
    if (nodes_[current_node].edges.empty()) {
      // Sanity check, if its a leaf it has a positive value.
      assert(nodes_[current_node].value > 0);
      values.push_back(nodes_[current_node].value);
    }
    // Otherwise, put its children on the stack.
    for (auto it = nodes_[current_node].edges.begin();
        it != nodes_[current_node].edges.end(); ++it) {
      stack.push(it->second.dst_node);
    }
  }
  return values;
}

int64_t dsl::SuffixTree::count(const std::string& query) const {
  int64_t subtree_root = findSubtreeRoot(query);
  std::stack<int64_t> stack;
  int64_t count = 0;

  if (subtree_root == -1) {
    return 0;
  }

  // Traverse the sub tree to find the leaves. Their labels give the positions
  // of the query in the full string.
  stack.push(subtree_root);
  while (!stack.empty()) {
    int64_t current_node = stack.top();
    stack.pop();
    // If node has no children it is a leaf. Save its start position.
    if (nodes_[current_node].edges.empty()) {
      // Sanity check, if its a leaf it has a positive value.
      assert(nodes_[current_node].value > 0);
      count += 1;
    }
    // Otherwise, put its children on the stack.
    for (auto it = nodes_[current_node].edges.begin();
        it != nodes_[current_node].edges.end(); ++it) {
      stack.push(it->second.dst_node);
    }
  }
  return count;
}

bool dsl::SuffixTree::contains(const std::string& query) const {
  return (findSubtreeRoot(query) >= 0);
}

size_t dsl::SuffixTree::serialize(std::ostream& out) {
  // Serialize text
  size_t out_size = 0;

  // Serialize text
  uint64_t text_size = text_.length();
  out.write(reinterpret_cast<const char *>(&text_size), sizeof(uint64_t));
  out_size += sizeof(uint64_t);
  out.write(reinterpret_cast<const char *>(text_.c_str()), text_size);
  out_size += text_size;

  // Serialize tree
  size_t nodes_size = nodes_.size();
  out.write(reinterpret_cast<const char *>(&nodes_size), sizeof(uint64_t));
  out_size += sizeof(uint64_t);
  for (auto node : nodes_) {
    out_size += serializeNode(node, out);
  }

  return out_size;
}

size_t dsl::SuffixTree::serializeNode(suffix_tree::Node& node,
                                      std::ostream& out) {
  size_t out_size = 0;

  out.write(reinterpret_cast<const char *>(&(node.value)), sizeof(int64_t));
  out_size += sizeof(int64_t);

  out.write(reinterpret_cast<const char *>(&(node.suffix_link)),
            sizeof(int64_t));
  out_size += sizeof(int64_t);

  size_t edges_size = node.edges.size();
  out.write(reinterpret_cast<const char *>(&(edges_size)), sizeof(uint64_t));
  out_size += sizeof(uint64_t);
  for (auto edge_pair : node.edges) {
    out.write(reinterpret_cast<const char *>(&(edge_pair.first)), sizeof(char));
    out_size += sizeof(char);
    out_size += serializeEdge(edge_pair.second, out);
  }

  return out_size;
}

size_t dsl::SuffixTree::serializeEdge(suffix_tree::Edge& edge,
                                      std::ostream& out) {
  size_t out_size = 0;

  out.write(reinterpret_cast<const char *>(&(edge.start_idx)), sizeof(int64_t));
  out_size += sizeof(int64_t);

  out.write(reinterpret_cast<const char *>(&(edge.end_idx)), sizeof(int64_t));
  out_size += sizeof(int64_t);

  out.write(reinterpret_cast<const char *>(&(edge.src_node)), sizeof(int64_t));
  out_size += sizeof(int64_t);

  out.write(reinterpret_cast<const char *>(&(edge.dst_node)), sizeof(int64_t));
  out_size += sizeof(int64_t);

  return out_size;
}

size_t dsl::SuffixTree::deserialize(std::istream& in) {
  size_t in_size = 0;

  // Deserialize text
  size_t text_size;
  in.read(reinterpret_cast<char *>(&text_size), sizeof(uint64_t));
  in_size += sizeof(uint64_t);
  text_.resize(text_size);
  in.read(reinterpret_cast<char *>(&text_[0]), text_size);
  in_size += text_size;

  // Deserialize tree
  size_t nodes_size;
  in.read(reinterpret_cast<char *>(&nodes_size), sizeof(uint64_t));
  in_size += sizeof(uint64_t);
  nodes_.reserve(nodes_size);
  for (size_t i = 0; i < nodes_size; i++) {
    suffix_tree::Node node;
    in_size += deserializeNode(node, in);
    nodes_.push_back(node);
  }

  return in_size;
}

size_t dsl::SuffixTree::deserializeNode(suffix_tree::Node& node,
                                        std::istream& in) {
  size_t in_size = 0;

  in.read(reinterpret_cast<char *>(&node.value), sizeof(int64_t));
  in_size += sizeof(int64_t);

  in.read(reinterpret_cast<char *>(&node.suffix_link), sizeof(int64_t));
  in_size += sizeof(int64_t);

  size_t edges_size;
  in.read(reinterpret_cast<char *>(&edges_size), sizeof(uint64_t));
  in_size += sizeof(uint64_t);

  for (size_t i = 0; i < edges_size; i++) {
    char c;
    in.read(reinterpret_cast<char *>(&c), sizeof(char));
    in_size += sizeof(char);
    suffix_tree::Edge edge;
    in_size += deserializeEdge(edge, in);
    node.edges[c] = edge;
  }

  return in_size;
}

size_t dsl::SuffixTree::deserializeEdge(suffix_tree::Edge& edge,
                                        std::istream& in) {
  size_t in_size = 0;

  in.read(reinterpret_cast<char *>(&(edge.start_idx)), sizeof(int64_t));
  in_size += sizeof(int64_t);

  in.read(reinterpret_cast<char *>(&(edge.end_idx)), sizeof(int64_t));
  in_size += sizeof(int64_t);

  in.read(reinterpret_cast<char *>(&(edge.src_node)), sizeof(int64_t));
  in_size += sizeof(int64_t);

  in.read(reinterpret_cast<char *>(&(edge.dst_node)), sizeof(int64_t));
  in_size += sizeof(int64_t);

  return in_size;
}
