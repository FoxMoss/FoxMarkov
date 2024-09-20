#include "csv.hpp"
#include "fast_chain.h"
#include "main.hpp"
#include "node.h"
#include "threadspool.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <sys/types.h>
#include <vector>

void process_scum(std::string file) {
  sqlite3 *db;
  unwrap_exit(sqlite3_open(file.c_str(), &db), "DB failed to open",
              RETURN_ZERO);

  ThreadSpool spool(4);

  sqlite3_stmt *stmt;
  unwrap_exit(
      sqlite3_prepare_v2(db, "select * from already_searched", -1, &stmt, NULL),
      "Search targets failed", RETURN_ZERO);

  std::map<std::string, std::vector<std::string>> users;
  uint count = 0;
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

    if (length < 500) {
      users.erase(id);
      continue;
    }
    printf("loading... %s with %u messages\n", raw_id, length);
    count++;
  }
  printf("all in memory\n");

  std::ofstream output_file("scum_crunched.csv");
  csv::CSVWriter<std::basic_ofstream<char>> *writer =
      new csv::CSVWriter<std::basic_ofstream<char>>(output_file);
  *writer << std::vector<std::string>{"weight", "id1", "id2"};
  std::mutex *writer_lock = new std::mutex();

  uint complete = 0;
  uint total = users.size();
  for (auto pair : users) {
    complete++;
    spool.push_thread([=]() {
      FastChain chain;
      for (auto line : pair.second) {
        chain.add_line(line);
      }

      for (auto test_pair : users) {

        if (test_pair.first == pair.first) {
          continue;
        }

        FastChain compare_chain;
        for (auto line : test_pair.second) {
          compare_chain.add_line(line);
        }

        auto res = compare_chain.compare_chain(chain);

        if (!res.has_value()) {
          continue;
        }

        writer_lock->lock();
        *writer << std::vector<std::string>{std::to_string(res.value()),
                                            pair.first, test_pair.first};
        writer_lock->unlock();
      }
      printf("%f%% done\n", ((float)complete / total) * 100);
    });
  }
  output_file.close();
}
