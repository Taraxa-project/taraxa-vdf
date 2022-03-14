#pragma once

#include <openssl/bn.h>
#include <cmath>
#include "VerifierWesolowski.h"
#include <chrono>
#include <vector>
#include <atomic>

namespace vdf
{
class ProverWesolowski
{
public:
  using solution = typename VerifierWesolowski::solution;

  ProverWesolowski();
  ~ProverWesolowski() = default;

  solution operator()(const VerifierWesolowski &verifier) const;

  void cancel();

  mutable std::vector<typename std::chrono::high_resolution_clock::duration> durations;

private:
  BN_CTX_free_ptr ctx_ptr;
  std::atomic<bool> canceled = false;
};
} // namespace vdf