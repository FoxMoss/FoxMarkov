#include "csv.hpp"
#include "main.hpp"
#include "node.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sqlite3.h>
#include <string>
#include <sys/types.h>
#include <vector>

void process_scum(std::string file) {
  sqlite3 *db;
  unwrap_exit(sqlite3_open(file.c_str(), &db), "DB failed to open",
              RETURN_ZERO);

  sqlite3_stmt *stmt;
  unwrap_exit(
      sqlite3_prepare_v2(db, "select * from already_searched", -1, &stmt, NULL),
      "Search targets failed", RETURN_ZERO);

  std::map<std::string, std::vector<std::string>> users;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *raw_id = (const char *)sqlite3_column_text(stmt, 0);
    std::string id(raw_id);
    users[id] = {};

    sqlite3_stmt *message_stmt;

    unwrap_exit(sqlite3_prepare_v2(db,
                                   "select * from messages where user_id = ?",
                                   -1, &message_stmt, NULL),
                "Search targets failed", RETURN_ZERO);

    sqlite3_bind_text(message_stmt, 1, raw_id, -1, SQLITE_STATIC);

    uint length = 0;
    while (sqlite3_step(message_stmt) == SQLITE_ROW && length < 1000) {
      length++;
      const char *message = (const char *)sqlite3_column_text(message_stmt, 0);
      users[id].push_back(message);
    }

    if (length < 800) {
      users.erase(id);
      continue;
    }
    printf("loading... %s with %u messages\n", raw_id, length);
  }
  printf("all in memory\n");

  std::ofstream output_file("scum_crunched.csv");
  auto writer = csv::make_csv_writer(output_file);
  writer << std::vector<std::string>{"weight", "id1", "id2"};

  uint complete = 0;
  uint total = users.size();
  for (auto pair : users) {
    complete++;
    Chain chain(pair.second);
    for (auto test_pair : users) {
      if (test_pair.first == pair.first) {
        continue;
      }

      float total_weight = 0;
      uint weight_length = 0;

      for (auto message : test_pair.second) {
        auto proccesed = proccessLine(message);

        float row_weight = 0;
        uint row_weight_length = 0;

        for (auto iter = proccesed.begin(); iter != proccesed.end() - 1;
             iter++) {
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
      writer << std::vector<std::string>{
          std::to_string(total_weight / weight_length), pair.first,
          test_pair.first};
    }
    printf("%f%% done\n", ((float)complete / total) * 100);
  }
  output_file.close();
}
