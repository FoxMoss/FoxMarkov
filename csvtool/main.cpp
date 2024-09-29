#include "../simd_testing/fast_chain.h"
#include "../src/csv.hpp"
#include <argparse/argparse.hpp>
#include <cstdint>
#include <cstdio>
#include <curl/curl.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include <utility>
#include <vector>
#include <tuple>

CURL *curl;
std::string base = "https://wetdry.world/";

size_t str_write(void *ptr, size_t size, size_t nmemb, std::string *data) {
  size_t data_size = size * nmemb;
  data->append((char *)ptr, data_size);
  return data_size;
}

std::string get_user_webfinger(std::string id) {
  std::string response;
  std::string combined;
  combined = base + "api/v1/accounts/" + id;
  curl_easy_setopt(curl, CURLOPT_URL, combined.c_str());

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, str_write);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_perform(curl);

  nlohmann::json account_data = nlohmann::json::parse(response);
  return account_data["acct"].get<std::string>();
}

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

  uint size = 20;

  std::unordered_map<uint64_t, std::pair<float, uint32_t>> rows;
  std::unordered_map<uint32_t, std::string> map;

  while (reader.read_row(row)) {
     float val = std::atof(row["weight"].get<std::string>().c_str());
    auto word1_hash = word_hash(row["id1"].get<std::string>());
    map[word1_hash] = row["id1"].get<std::string>();
    auto word2_hash = word_hash(row["id2"].get<std::string>());
    map[word2_hash] = row["id2"].get<std::string>();

    uint64_t quick_hash = quick_hash = combine_hash(word1_hash, word2_hash);

    rows[quick_hash] = {val, row["count"].get<int>()};
  }

  std::vector<std::tuple<float, uint64_t, uint32_t>> top_rows;

  for (auto row : rows) {
    if (row.second.second < 40)
        continue;

    if (top_rows.size() < size) {
      top_rows.push_back({row.second.first, row.first, row.second.second});
    }
    for (auto &top_row : top_rows) {
      if (row.second.first < std::get<float>(top_row)) {
	std::get<float>(top_row) = row.second.first;
	std::get<uint64_t>(top_row) = row.first;
	std::get<uint32_t>(top_row) = row.second.second;
        break;
      }
    }
  }

  curl = curl_easy_init();
  for (auto iter = top_rows.begin(); iter != top_rows.end(); iter++) {
    std::string person1 = get_user_webfinger(map[(uint32_t)std::get<uint64_t>(*iter)]);
    std::string person2 =
         get_user_webfinger(map[std::get<uint64_t>(*iter) >> sizeof(uint32_t) * 8]);
    printf("%f, %s, %s\n", std::get<float>(*iter), person1.c_str(), person2.c_str());
    printf("%f, %s, %s, %i\n", std::get<float>(*iter), map[(uint32_t)std::get<uint64_t>(*iter)].c_str(),
           map[std::get<uint64_t>(*iter) >> sizeof(uint32_t) * 8].c_str(), std::get<uint32_t>(*iter));
  }
  curl_easy_cleanup(curl);
}
