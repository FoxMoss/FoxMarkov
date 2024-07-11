#include "node.h"
std::vector<std::string> proccessLine(std::string line) {
  std::vector<std::string> source = {};
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
  return source;
}
