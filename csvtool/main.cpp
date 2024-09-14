#include "../simd_testing/fast_chain.h"
#include "../src/csv.hpp"
#include <argparse/argparse.hpp>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include <utility>
#include <vector>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser parser("csvsort");
  parser.add_argument("csv").required();
  parser.add_argument("row").required();

  try {
    parser.parse_args(argc, argv);
  } catch (std::exception e) {
    printf("csvsort: %s\n", e.what());
  }

  std::ifstream reader_stream(parser.get<std::string>("csv"));
  csv::CSVReader reader(reader_stream);
  csv::CSVRow row;

  uint size = 40;

  std::unordered_map<uint64_t, float> rows;
  std::unordered_map<uint32_t, std::string> map;

  while (reader.read_row(row)) {
    float val = 0;
    if (row["weight"].is_float()) {
      val = row["weight"].get<float>();
    }
    auto word1_hash = word_hash(row["id1"].get<std::string>());
    map[word1_hash] = row["id1"].get<std::string>();
    auto word2_hash = word_hash(row["id2"].get<std::string>());
    map[word2_hash] = row["id2"].get<std::string>();

    uint64_t quick_hash = 0;
    if (word1_hash > word2_hash) {
      quick_hash = combine_hash(word2_hash, word1_hash);
    } else {
      quick_hash = combine_hash(word1_hash, word2_hash);
    }

    rows[quick_hash] += val;
  }

  std::vector<std::pair<float, uint64_t>> top_rows;

  for (auto row : rows) {
    if (top_rows.size() < size) {
      top_rows.push_back({row.second, row.first});
    }
    for (auto &top_row : top_rows) {
      if (row.second > top_row.first) {
        top_row.second = row.first;
        top_row.first = row.second;
        break;
      }
    }
  }

  for (auto iter = top_rows.begin(); iter != top_rows.end(); iter++) {
    printf("%f, %s, %s\n", iter->first, map[(uint32_t)iter->second].c_str(),
           map[iter->second >> sizeof(uint32_t) * 8].c_str());
  }
}
