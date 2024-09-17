#include "main.hpp"
#include "csv.hpp"
#include "fast_chain.h"
#include "node.h"
#include <algorithm>
#include <argparse/argparse.hpp>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

#define PROGRAM_NAME "foxkov"
#define TRY_UNWRAP(stmt) stmt
#define ADD_HELP(arg_parser)                                                   \
  arg_parser.add_argument("--help", "-h")                                      \
      .action([&](const auto & /*unused*/) {                                   \
        std::cout << arg_parser.help().str();                                  \
        exit(1);                                                               \
      })                                                                       \
      .default_value(false)                                                    \
      .help("Shows help message and exits")                                    \
      .implicit_value(true)                                                    \
      .nargs(0);

void process_scum(std::string file);

void unwrap_exit(int exit_value, const char *exit_str,
                 RETURN_TYPE return_type) {
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
                                      argparse::default_arguments::none);
  arg_parser.add_argument("--csv").help(
      "The CSV from which the chain will be generated.");
  arg_parser.add_argument("--column")
      .default_value("Content")
      .help("The column of the CSV of which will used for chain generation, "
            "all other columns will be ignored.");

  argparse::ArgumentParser generate_parser("generate", "",
                                           argparse::default_arguments::none);
  ADD_HELP(generate_parser);
  generate_parser.add_argument("--perfect")
      .nargs(0)
      .default_value(false)
      .help("Traverses only the most weighted path, often leading to an "
            "infinite loop.");
  generate_parser.add_description("Traverse the chain");

  argparse::ArgumentParser scum_parser("scum", "",
                                       argparse::default_arguments::none);
  ADD_HELP(scum_parser);
  scum_parser.add_argument("db-file");
  scum_parser.add_description("Crunch the numbers for a scumdb database.");

  argparse::ArgumentParser compare_parser("compare", "",
                                          argparse::default_arguments::none);
  ADD_HELP(compare_parser);
  compare_parser.add_description(
      "Identify similarity between data and the chain");
  compare_parser.add_argument("csv").remaining().help(
      "The CSV(s) that will be compared to the chain.");
  compare_parser.add_argument("--column")
      .default_value("Content")
      .help("The column of the CSV of which will used for chain comparision, "
            "all other columns will be ignored.");
  ;
  compare_parser.add_argument("--overview-log")
      .default_value("")
      .help("Creates a CSV of the targets and their average weight.");
  compare_parser.add_argument("--detailed-log")
      .default_value("")
      .help("Creates a CSV of all targets with their individual line weight "
            "averages.");

  argparse::ArgumentParser cleanup_parser("cleanup", "",
                                          argparse::default_arguments::none);
  ADD_HELP(cleanup_parser);
  cleanup_parser.add_description("Seperate individual targets into a new CSV");
  cleanup_parser.add_argument("--matching-column")
      .default_value("Author")
      .help("The column from which a match will be derived.");
  cleanup_parser.add_argument("--matching-data")
      .help("The data the column needs to match for it to be included.");
  cleanup_parser.add_argument("--byproduct").default_value("");
  cleanup_parser.add_argument("--output").default_value("cleanup.csv");

  arg_parser.add_subparser(generate_parser);
  arg_parser.add_subparser(compare_parser);
  arg_parser.add_subparser(cleanup_parser);
  arg_parser.add_subparser(scum_parser);

  arg_parser.add_description(
      "Markov chains are models that encode probability. This project creates "
      "Markov chains out of text to generate probabilities of individuals and "
      "groups, and provides tools to analyze them.");
  ADD_HELP(arg_parser);

  if (argc == 1) {
    std::cout << arg_parser.help().str();
    exit(1);
  }
  try {
    arg_parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    unwrap_exit(-1, err.what());
  }

  try {
    if (arg_parser.is_subcommand_used("scum")) {
      process_scum(scum_parser.get("db-file"));
      return 0;
    }

    if (arg_parser.is_subcommand_used("generate")) {
      csv::CSVReader reader(arg_parser.get("--csv"));

      std::vector<std::string> contents = {};
      csv::CSVRow row;
      while (reader.read_row(row)) {
        TRY_UNWRAP(contents.push_back(row[arg_parser.get("--column")].get());)
      }
      Chain chain(contents);

      std::optional<std::string> suggestion = "\n";
      while ((suggestion = chain.Suggest(suggestion.value(),
                                         generate_parser.is_used("--perfect")))
                 .has_value() &&
             suggestion != "\n") {
        printf("%s", suggestion.value().c_str());
        printf("\n");
      }
    }

    if (arg_parser.is_subcommand_used("compare")) {

      csv::CSVReader reader(arg_parser.get("--csv"));
      csv::CSVRow row;
      FastChain original_chain;

      while (reader.read_row(row)) {
        original_chain.add_line(row[arg_parser.get("--column")].get());
      }

      std::vector<std::vector<std::string>> overview;
      std::map<std::string, std::vector<std::string>> detailed;
      for (auto file : compare_parser.get<std::vector<std::string>>("csv")) {
        csv::CSVReader sten_reader(file);
        FastChain compare_chain;

        while (sten_reader.read_row(row)) {
          compare_chain.add_line(row[compare_parser.get("--column")].get());
        }

        auto result = compare_chain.compare_chain(original_chain);
        if (!result.has_value())
          continue;

        printf("Contents of %s matches weights %f/%f\n", file.c_str(),
               result.value(), 1.0);
        if (compare_parser.get("--overview-log") != "") {
          overview.push_back({file, std::to_string(result.value())});
        }
      }

      if (compare_parser.get("--detailed-log") != "") {
        std::ofstream output_file(compare_parser.get("--detailed-log"));
        auto writer = csv::make_csv_writer(output_file);
        std::vector<std::string> keys = {"Test"};
        size_t max_length = 0;
        for (auto pair : detailed) {
          keys.push_back(pair.first);
          max_length = std::max(pair.second.size(), max_length);
        }
        writer << keys;
        for (size_t index = 0; index < max_length; index++) {
          std::vector<std::string> row = {std::to_string(index)};
          for (auto pair : detailed) {
            if (pair.second.size() >= index + 1) {
              row.push_back(pair.second[index]);
            }
          }
          writer << row;
        }
        printf("Wrote detailed results to %s\n",
               compare_parser.get("--detailed-log").c_str());
        output_file.close();
      }

      if (compare_parser.get("--overview-log") != "") {
        std::ofstream output_file(compare_parser.get("--overview-log"));
        auto writer = csv::make_csv_writer(output_file);
        writer << std::vector<std::string>{"Filename", "Weight"};
        for (auto result : overview) {
          if (result[0] == arg_parser.get("--csv")) {
            result[0] = "Control";
          }
          writer << result;
        }
        printf("Wrote overview of results to %s\n",
               compare_parser.get("--overview-log").c_str());
        output_file.close();
      }
    } // end compare

    if (arg_parser.is_subcommand_used("cleanup")) {

      std::map<std::string, uint> user_map;
      std::ofstream output_file(cleanup_parser.get("--output"));
      csv::CSVReader reader(arg_parser.get("--csv"));
      auto writer = csv::make_csv_writer(output_file);
      writer << reader.get_col_names();
      csv::CSVRow row;
      TRY_UNWRAP(
          // the csv libary is shit with error handling so im just wrapping this
          // in a try catch
          for (csv::CSVRow row
               : reader) {
            if (cleanup_parser.get("--byproduct") != "") {
              if (user_map.find(
                      row[cleanup_parser.get("--matching-column")].get()) ==
                  user_map.end()) {
                user_map[row[cleanup_parser.get("--matching-column")].get()] =
                    0;
              }
              user_map[row[cleanup_parser.get("--matching-column")].get()]++;
            }
            // check if row should be included
            if (row[cleanup_parser.get("--matching-column")].get() ==
                cleanup_parser.get("--matching-data")) {

              std::vector<std::string> row_data = {};

              for (csv::CSVField field : row) {
                row_data.push_back(field.get<>());
              }

              writer << row_data;
            }
          })
      if (cleanup_parser.get("--byproduct") != "") {
        std::ofstream byproduct(cleanup_parser.get("--byproduct"));
        auto byproduct_writer = csv::make_csv_writer(byproduct);
        byproduct_writer << std::vector<std::string>{"Username", "Instances"};
        for (auto pair : user_map) {
          byproduct_writer << std::vector<std::string>{
              pair.first, std::to_string(pair.second)};
        }
        printf("Wrote byproduct data to %s\n",
               cleanup_parser.get("--byproduct").c_str());
        byproduct.close();
      }

      printf("Wrote sectioned data to %s\n",
             cleanup_parser.get("--output").c_str());
      output_file.close();
    }
  } catch (const std::exception &err) {
    unwrap_exit(-1, err.what());
  }

  return 0;
}
