#pragma once

#include "../src/node.h"
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
#define OFFSET_BASIS 2166136261
#define PRIME 16777619;
static uint32_t word_hash(std::string word) {
  uint32_t hash = OFFSET_BASIS;
  for (int index = word.size(); index >= 0; index--) {
    char byte = word[index];
    hash = hash * PRIME;
    hash = hash ^ byte;
  }
  return hash;
}

static uint64_t combine_hash(uint32_t a, uint32_t b) {

  uint64_t ret = (uint64_t)a << sizeof(uint32_t) * 8;
  ret |= b;
  return ret;
}

template <typename T> void print_bytes(T num) {
  const size_t size = sizeof(T) * 8;
  std::bitset<size> num_bitmap(num);
  std::cout << num_bitmap << "\n";
}

template <typename T> T shift_or(T num) {
  T ret = 0;
  size_t size = sizeof(T);
  for (size_t i = 0; i < size * 8; ++i) {
    ret |= num >> i;
    ret |= num << i;
  }
  return ret;
}

// static void shift_or_simd(__m256i &num) {
//   size_t size = sizeof(uint64_t);
//   for (size_t i = 0; i < size * 8; ++i) {
//     num = _mm256_or_si256(_mm256_srli_epi32(num, i), num);
//     num = _mm256_or_si256(_mm256_slli_epi32(num, i), num);
//   }
// }

class FastChain {
public:
  std::unordered_map<uint32_t, std::string> hash_to_string;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint>>
      hash_to_children;
  std::unordered_map<uint32_t, uint> hash_to_child_total;
  FastChain() {}
  void add_line(std::string line) {
    std::vector<std::string> source = proccessLine(line);

    for (auto iter = source.begin(); iter + 1 != source.end(); iter++) {
      auto token = word_hash(*iter.base());
      hash_to_string[token] = *iter.base();
      auto next_token = word_hash(*(iter + 1).base());
      hash_to_children[token][next_token]++;
      hash_to_child_total[token]++;
    }
  }
  std::string generate() {
    std::string ret;
    srand(time(NULL));

    bool first = true;
    uint32_t token = word_hash("\n");
    while (1) {
      if (!first && token == word_hash("\n")) {
        break;
      }
      first = false;
      uint total_weight = 0;
      for (auto child : hash_to_children[token]) {
        total_weight += child.second;
      }

      if (total_weight == 0) {
        break;
      }

      uint total_rand = rand() % total_weight;
      for (auto child : hash_to_children[token]) {
        if (total_rand < child.second) {
          token = child.first;
          ret += hash_to_string[token];
          break;
        }

        total_rand -= child.second;
      }
    }

    return ret;
  }
  float match_tokens(uint32_t a, uint32_t b) {
    if (hash_to_child_total[a] == 0)
      return 0;
    return (float)hash_to_children[a][b] / hash_to_child_total[a];
  }
};
