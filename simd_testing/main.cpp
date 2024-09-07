#include "csv.hpp"
#include "fast_chain.h"
#include "node.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <immintrin.h>
#include <vector>

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
  // first test

  FastChain chain;
  for (auto line : contents) {
    chain.add_line(line);
  }

  uint count = 0;
  float total = 0;
  for (auto line : contents) {
    auto tokens = proccessLine(line);

    auto iter = tokens.begin();
    while (1) {
      uint index = 0;
      union results {
        float in_values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        float out_values[8];
      } results;
      float divisors[8] = {1, 1, 1, 1, 1, 1, 1, 1};

      for (; iter + 1 != tokens.end(); iter++) {
        auto a_hash = word_hash(*iter);
        auto b_hash = word_hash(*(iter + 1));

        results.in_values[index] =
            (float)chain.hash_to_children[a_hash][b_hash];
        divisors[index] = (float)chain.hash_to_child_total[a_hash];
        count++;
        if (index >= 8)
          break;
      }

      __m256 values_simd = _mm256_setr_ps(
          results.in_values[0], results.in_values[1], results.in_values[2],
          results.in_values[3], results.in_values[4], results.in_values[5],
          results.in_values[6], results.in_values[7]);
      __m256 divisors_simd =
          _mm256_setr_ps(divisors[0], divisors[1], divisors[2], divisors[3],
                         divisors[4], divisors[5], divisors[6], divisors[7]);
      auto results_simd = _mm256_div_ps(values_simd, divisors_simd);

      _mm256_storeu_ps(results.out_values, results_simd);
      for (uint i = 0; i < 8; i++) {
        total += results.out_values[i];
      }
      if (index != 7)
        break;
    }
  }

  auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "FastChain: " << elapsed_seconds.count() << "s\n";
  printf("result %f\n\n", total / count);

  start = std::chrono::system_clock::now();
  // second test

  Chain slow_chain(contents);

  count = 0;
  total = 0;
  for (auto line : contents) {
    auto tokens = proccessLine(line);
    for (auto iter = tokens.begin(); iter + 1 != tokens.end(); iter++) {
      total += slow_chain.GetNormalizedWeight(*iter, *(iter + 1));
      count++;
    }
  }

  end = std::chrono::system_clock::now();

  elapsed_seconds = end - start;

  std::cout << "Chain: " << elapsed_seconds.count() << "s\n";
  printf("result %f\n\n", total / count);

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
