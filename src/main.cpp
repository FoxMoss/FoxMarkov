#include "csv.hpp"
#include "node.h"
#include <argparse/argparse.hpp>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define PROGRAM_NAME "term-gpu"
#define TRY_UNWRAP(stmt)                                                       \
  try {                                                                        \
    stmt                                                                       \
  } catch (const std::exception e) {                                           \
    unwrap_exit(-1, e.what());                                                 \
  }

enum RETURN_TYPE { RETURN_POSITIVE, RETURN_ZERO };

void unwrap_exit(int exit_value, const char *exit_str,
                 RETURN_TYPE return_type = RETURN_POSITIVE) {
  bool should_exit = false;
  switch (return_type) {
  case RETURN_POSITIVE:
    should_exit = exit_value < 0;
    break;
  case RETURN_ZERO:
    should_exit = exit_value != 0;
    break;
  }
  if (should_exit) {
    printf("%s: %s\n", PROGRAM_NAME, exit_str);
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  argparse::ArgumentParser arg_parser(PROGRAM_NAME, "",
                                      argparse::default_arguments::help);
  arg_parser.add_argument("--csv").required();
  arg_parser.add_argument("--column").default_value("Content");

  argparse::ArgumentParser generate_parser("generate");
  generate_parser.add_argument("--perfect").nargs(0).default_value(false);
  argparse::ArgumentParser stenography_parser("stenography");
  stenography_parser.add_argument("--csv").required();
  stenography_parser.add_argument("--column").default_value("Content");

  arg_parser.add_subparser(generate_parser);
  arg_parser.add_subparser(stenography_parser);

  try {
    arg_parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    unwrap_exit(-1, err.what());
  }

  csv::CSVReader reader(arg_parser.get("--csv"));
  std::vector<std::string> contents = {};
  csv::CSVRow row;
  while (reader.read_row(row)) {
    TRY_UNWRAP(contents.push_back(row[arg_parser.get("--column")].get());)
  }
  Chain chain(contents);

  if (arg_parser.is_subcommand_used("generate")) {
    std::optional<std::string> suggestion = "\n";
    while ((suggestion = chain.Suggest(suggestion.value(),
                                       generate_parser.is_used("--perfect")))
               .has_value() &&
           suggestion != "\n") {
      printf("%s", suggestion.value().c_str());
    }
    printf("\n");
  } else if (arg_parser.is_subcommand_used("stenography")) {
    csv::CSVReader sten_reader(stenography_parser.get("--csv"));

    float total_weight = 0;
    uint weight_length = 0;
    while (sten_reader.read_row(row)) {
      std::vector<std::string> proccesed;
      TRY_UNWRAP(proccesed = proccessLine(
                     row[stenography_parser.get("--column")].get());)

      float row_weight = 0;
      uint row_weight_length = 0;

      for (auto iter = proccesed.begin(); iter != proccesed.end() - 1; iter++) {
        row_weight +=
            chain.GetNormalizedWeight(*iter.base(), *(iter + 1).base());
        row_weight_length++;
      }

      if (row_weight_length == 0) {
        continue;
      }

      total_weight += row_weight / row_weight_length;
      weight_length++;
    }
    printf("Other source matches weights %f/%f\n", total_weight / weight_length,
           1.0);
  }

  return 0;
}
