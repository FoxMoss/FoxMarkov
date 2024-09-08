#include "csv.hpp"
#include "fast_chain.h"
#include "node.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <immintrin.h>
#include <vector>

int main() {

  csv::CSVReader reader("incoming.csv");
  std::vector<std::string> contents = {};
  csv::CSVRow row;
  while (reader.read_row(row)) {
    contents.push_back(row["Content"].get());
    contents.push_back(row["Content"].get());
    contents.push_back(row["Content"].get());
    contents.push_back(row["Content"].get());
    contents.push_back(row["Content"].get());
    contents.push_back(row["Content"].get());
  }

  auto start = std::chrono::system_clock::now();
  // first test

  FastChain chain;
  for (auto line : contents) {
    chain.add_line(line);
  }

  uint count = 0;
  float total = 0;
  for (auto line : contents) {
    auto tokens = proccessLine(line);

    for (auto iter = tokens.begin(); iter + 1 != tokens.end(); iter++) {
      auto a_hash = word_hash(*iter);
      auto b_hash = word_hash(*(iter + 1));

      total += chain.match_tokens(a_hash, b_hash);
      count++;
    }
  }

  auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "FastChain: " << elapsed_seconds.count() << "s\n";
  printf("result %f\n\n", total / count);

  start = std::chrono::system_clock::now();
  // second test

  Chain slow_chain(contents);

  count = 0;
  total = 0;
  for (auto line : contents) {
    auto tokens = proccessLine(line);
    for (auto iter = tokens.begin(); iter + 1 != tokens.end(); iter++) {
      total += slow_chain.GetNormalizedWeight(*iter, *(iter + 1));
      count++;
    }
  }

  end = std::chrono::system_clock::now();

  elapsed_seconds = end - start;

  std::cout << "Chain: " << elapsed_seconds.count() << "s\n";
  printf("result %f\n\n", total / count);
}
