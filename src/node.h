#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <map>
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

class Node {
public:
  Node(std::string cToken) { token = cToken; }
  ~Node() {}

  std::map<Node *, uint> children = {};
  std::string token;

private:
};

std::vector<std::string> proccessLine(std::string,
                                      std::vector<std::string> * = NULL);

std::vector<std::string> proccessLineList(std::vector<std::string> line_list);

class Chain {
public:
  Chain(std::vector<std::string> string_list) {
    // proccess line list to be chunked into tokens
    std::vector<std::string> source = proccessLineList(string_list);

    // generate nodes in graph
    for (auto iter = source.begin(); iter != source.end(); iter++) {
      auto token = *iter.base();
      if (nodes.find(token) == nodes.end()) {
        nodes[token] = new Node(token);
      }
    }
    // connect nodes
    for (auto iter = source.begin(); iter != source.end(); iter++) {
      auto token = *iter.base();
      if (iter + 1 != source.end()) {
        auto nodesedToken = nodes[*(iter + 1).base()];

        if (nodes[token]->children.find(nodesedToken) ==
            nodes[token]->children.end()) {
          nodes[token]->children[nodesedToken] = 0;
        }

        nodes[token]->children[nodesedToken]++;
      }
    }
  };
  ~Chain() {
    for (auto pair : nodes) {
      delete pair.second;
    }
  };

  std::optional<std::string> Suggest(std::string token, bool perfect = true) {
    std::map<std::string, Node *>::iterator node;
    if ((node = nodes.find(token)) == nodes.end())
      return {};

    srand(time(NULL));
    uint total_weight = 0;
    uint top_weight = 0;
    Node *top_weight_node;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      total_weight += iter->second;
      if (perfect && iter->second > top_weight) {
        top_weight = iter->second;
        top_weight_node = iter->first;
      }
    }
    if (perfect) {
      return top_weight_node->token;
    }
    uint total_rand = rand() % total_weight;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      if (total_rand < iter->second) {
        return iter->first->token;
      }
      total_rand -= iter->second;
    }
    return {};
  }

  float GetNormalizedWeight(std::string start_token, std::string child_token) {
    std::map<std::string, Node *>::iterator node;
    if ((node = nodes.find(start_token)) == nodes.end())
      return 0;
    std::map<std::string, Node *>::iterator searched_node;
    if ((searched_node = nodes.find(child_token)) == nodes.end())
      return 0;

    float total_weight = 0;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      total_weight += iter->second;
    }

    auto connection = node->second->children.find(searched_node->second);
    if (connection == node->second->children.end())
      return 0;
    return connection->second / total_weight;
  }

private:
  std::map<std::string, Node *> nodes;
};
