#pragma once

#include <openssl/bn.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <vector>

#include "VerifierWesolowski.h"

namespace vdf {
class ProverWesolowski {
 public:
  using solution = typename VerifierWesolowski::solution;

  ProverWesolowski();
  ~ProverWesolowski() = default;

  solution operator()(const VerifierWesolowski& verifier) const;
  solution operator()(const VerifierWesolowski& verifier, const std::atomic_bool& cancelled) const;

  mutable std::vector<typename std::chrono::high_resolution_clock::duration> durations;

 private:
  BN_CTX_free_ptr ctx_ptr;
};
}  // namespace vdf