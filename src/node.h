#include <algorithm>
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

class Chain {
public:
  Chain(std::vector<std::string> string_list) {
    std::vector<std::string> source = {};
    // proccess line list to be chunked into tokens
    for (auto line : string_list) {
      source.push_back("\n");
      std::string token = "";
      for (auto ch : line) {
        token.push_back(ch);
        if (ch == ' ') {
          source.push_back(token);
          token = "";
        }
      }
      if (!token.empty()) {
        token.push_back(' ');
        source.push_back(token);
      }
    }

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

  std::optional<std::string> Suggest(std::string token) {
    std::map<std::string, Node *>::iterator node;
    if ((node = nodes.find(token)) == nodes.end())
      return {};

    srand(time(NULL));
    uint totalWeight = 0;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      totalWeight += iter->second;
    }
    uint totalRand = rand() % totalWeight;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      if (totalRand < iter->second) {
        return iter->first->token;
      }
      totalRand -= iter->second;
    }
    return {};
  }

  float GetNormalizedWeight(std::string start_token, std::string child_token) {
    std::map<std::string, Node *>::iterator node;
    if ((node = nodes.find(start_token)) == nodes.end())
      return 0;
    std::map<std::string, Node *>::iterator searched_node;
    if ((searched_node = nodes.find(start_token)) == nodes.end())
      return 0;

    float totalWeight = 0;
    for (auto iter = node->second->children.begin();
         iter != node->second->children.end(); iter++) {
      totalWeight += iter->second;
    }

    auto connection = node->second->children.find(searched_node->second);
    if (connection == node->second->children.end())
      return 0;
    return connection->second / totalWeight;
  }

private:
  std::map<std::string, Node *> nodes;
};
