#include "csv.hpp"
#include "fast_chain.h"
#include "node.h"
#include <argparse/argparse.hpp>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <immintrin.h>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser arg_parser("simd", "",
                                      argparse::default_arguments::none);

  arg_parser.add_argument("csv1").required();
  arg_parser.add_argument("csv2").required();

  if (argc == 1) {
    std::cout << arg_parser.help().str();
    exit(1);
  }
  try {
    arg_parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    exit(1);
  }

  csv::CSVRow row;

  csv::CSVReader reader1(arg_parser.get<std::string>("csv1"));
  FastChain chain1;

  while (reader1.read_row(row)) {
    chain1.add_line(row["Content"].get());
  }

  csv::CSVReader reader2(arg_parser.get<std::string>("csv2"));
  FastChain chain2;

  while (reader2.read_row(row)) {
    chain2.add_line(row["Content"].get());
  }

  auto res = chain2.compare_chain(chain1);
  if (res.has_value())
    printf("%s - %s: %f\n", arg_parser.get<std::string>("csv1").c_str(),
           arg_parser.get<std::string>("csv2").c_str(), res.value());
  return 0;
}
