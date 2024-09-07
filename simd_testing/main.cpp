#include "csv.hpp"
#include "fast_chain.h"
#include "node.h"
#include <chrono>

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

  FastChain chain;
  for (auto line : contents) {
    chain.add_line(line);
  }

  for (auto line : contents) {
    auto tokens = proccessLine(line);
    for (auto iter = tokens.begin(); iter + 1 != tokens.end(); iter++) {
      auto a_hash = word_hash(*iter);
      auto b_hash = word_hash(*(iter + 1));
      chain.match_tokens(a_hash, b_hash);
    }
  }

  auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cout << "FastChain: " << elapsed_seconds.count() << "s\n";

  start = std::chrono::system_clock::now();

  Chain slow_chain(contents);

  for (auto line : contents) {
    auto tokens = proccessLine(line);
    for (auto iter = tokens.begin(); iter + 1 != tokens.end(); iter++) {
      slow_chain.GetNormalizedWeight(*iter, *(iter + 1));
    }
  }

  end = std::chrono::system_clock::now();

  elapsed_seconds = end - start;

  std::cout << "slow Chain: " << elapsed_seconds.count() << "s\n";

  // return 0;
  // print_bytes(word_hash("ball "));
  // print_bytes(word_hash("to "));
  // print_bytes(combine_hash(word_hash("ball "), word_hash("to ")));
  //
  // return 0;
  // __m256i all_f = _mm256_set1_epi32(0xFFFFFFFF);
  //
  // __m256i hash1 =
  //     _mm256_setr_epi32(word_hash("ball"), word_hash("ball"),
  //     word_hash("ball"),
  //                       word_hash("ball"), word_hash("ball"),
  //                       word_hash("ball"), word_hash("ball"),
  //                       word_hash("ball"));
  // __m256i hash2 =
  //     _mm256_setr_epi32(word_hash("ball"), word_hash("ball"),
  //     word_hash("ball"),
  //                       word_hash("ball"), word_hash("ball"),
  //                       word_hash("ball"), word_hash("ball"),
  //                       word_hash("balls"));
  // __m256 weight = _mm256_setr_ps(0.5, 0.2, 0.6, 0.8, 0.5, 0.2, 0.6, 0.8);
  // __m256i res = _mm256_xor_si256(hash1, hash2);
  // shift_or_simd(res);
  // res = _mm256_xor_si256(res, all_f);
  // __m256 mask = _mm256_castsi256_ps(res);
  //
  // auto res_weight = _mm256_and_ps(mask, weight);
  //
  // for (uint i = 0; i < 8; i++) {
  //   double_t weight_calc = res_weight[i];
  //   printf("%f\n", weight_calc);
  // }
}
