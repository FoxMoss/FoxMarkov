#include "csv.hpp"
#include "node.h"
#include <argparse/argparse.hpp>
#include <cstddef>
#include <cstdlib>
#include <ctime>
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
  arg_parser.add_argument("--discordmate").required();

  try {
    arg_parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    unwrap_exit(-1, err.what());
  }

  csv::CSVReader reader(arg_parser.get("--discordmate"));
  std::vector<std::string> contents = {};
  csv::CSVRow row;
  while (reader.read_row(row)) {
    contents.push_back(row["Content"].get());
  }
  Chain chain(contents);

  std::optional<std::string> suggestion = "\n";
  while ((suggestion = chain.Suggest(suggestion.value())).has_value() &&
         suggestion != "\n") {
    printf("%s", suggestion.value().c_str());
  }
  printf("\n");

  return 0;
}
