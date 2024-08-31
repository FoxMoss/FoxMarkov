#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <eve/eve.hpp>
#include <vector>

#include "../src/node.h"
#include "eve/module/core/regular/if_else.hpp"
#include "eve/module/core/regular/is_greater.hpp"

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
#define OFFSET_BASIS 0xcbf29ce484222325
#define PRIME 0x100000001b3;
uint64_t word_hash(std::string word) {
  uint64_t hash = OFFSET_BASIS;
  for (char byte : word) {
    hash = hash * PRIME;
    hash = hash ^ byte;
  }
  return hash;
}

int main() {
  std::vector<std::string> chain_src = {"\n", "i ",  "think ", "therfore ",
                                        "i ", "am ", "\n"};
  Chain chain(chain_src);
  std::vector<uint64_t> hash_1;
  std::vector<uint64_t> hash_2;
  std::vector<double> weight;
  std::vector<double> results;
  std::vector<uint64_t> result_keys;
  for (auto node : chain.nodes) {
    for (auto child : node.second->children) {
      hash_1.push_back(word_hash(node.first));
      hash_2.push_back(word_hash(child.first->token));
      weight.push_back((double)child.second / chain.nodes.size());
      results.push_back(0);
      result_keys.push_back(0);
    }
  }

  eve::wide<uint64_t> simd_hash_1(hash_1);
  eve::wide<uint64_t> simd_hash_2(hash_2);
  eve::wide<double> simd_weight(weight);
  eve::wide<double> simd_results(results);
  eve::wide<uint64_t> simd_result_keys(result_keys);

  std::vector<std::string> compare_src = {"\n", "i ", "am ", "\n"};

  for (size_t index = 0; compare_src[index + 1] != "\n"; index++) {
    eve::wide<uint64_t> matches = simd_hash_1 ^ word_hash(compare_src[index]);
    matches |= simd_hash_2 ^ word_hash(compare_src[index + 1]);

    // this is a fucked hack
    eve::wide<uint64_t> matches_bool =
        eve::if_else(matches == 0, 0xFFFFFFFFFFFFFFFF, (uint64_t)0) &
        (uint64_t)0b0000000000000000000000000000000000000000000000000000000000000001;
    eve::wide<double> matches_bool_double =
        eve::convert(matches_bool, eve::as<double>{});
    std::cout << matches_bool << "\n";

    simd_result_keys += matches_bool;

    eve::wide<double> pair_results = simd_results * matches_bool_double;

    simd_results += pair_results;
  }

  double total_weight = 0;
  uint total_size = 0;
  for (uint index = 0; index < simd_results.size(); index++) {
    total_weight += simd_results.get(index);
    total_size += simd_result_keys.get(index);
  }

  printf("simd calc'ed %f\n", total_weight);

  return 0;
}
