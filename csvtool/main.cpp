#include "../src/csv.hpp"
#include <argparse/argparse.hpp>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
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

  std::vector<std::pair<float, std::string>> top_rows;

  for (auto col : reader.get_col_names()) {
    printf("%s\n", col.c_str());
  }

  while (reader.read_row(row)) {
    float val = 0;
    if (row["weight"].is_float()) {
      val = row["weight"].get<float>();
    }
    if (top_rows.size() < 10) {
      top_rows.push_back({val, row.to_json()});
    } else {
      for (auto iter = top_rows.begin(); iter != top_rows.end(); iter++) {
        if (iter->first < val) {
          iter->first = val;
          iter->second = row.to_json();
          break;
        }
      }
    }
  }
  for (auto iter = top_rows.begin(); iter != top_rows.end(); iter++) {
    printf("%s\n", iter->second.c_str());
  }
}
