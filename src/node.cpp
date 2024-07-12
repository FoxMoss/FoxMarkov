#include "node.h"
#include <cstddef>

std::vector<std::string> proccessLineList(std::vector<std::string> line_list) {
  std::vector<std::string> ret = {};
  for (auto line : line_list) {
    proccessLine(line, &ret);
  }
  return ret;
}

std::vector<std::string> proccessLine(std::string line,
                                      std::vector<std::string> *source_pass) {
  std::vector<std::string> source_src = {};
  std::vector<std::string> *source = &source_src;
  if (source_pass != NULL)
    source = source_pass;

  source->push_back("\n");
  std::string token = "";
  for (auto ch : line) {
    token.push_back(ch);
    if (ch == ' ' || ch == '@' || ch == '"' || ch == '.') { // split tokens
      source->push_back(token);
      token = "";
    }
  }
  if (!token.empty()) {
    token.push_back(' ');
    source->push_back(token);
  }

  if (source_pass != NULL) {
    return {};
  }
  return source_src;
}
