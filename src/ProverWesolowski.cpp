#include "../include/ProverWesolowski.h"

namespace vdf {

ProverWesolowski::ProverWesolowski() : ctx_ptr(BN_CTX_free_ptr(BN_CTX_secure_new(), ::BN_CTX_free)) {}

typename ProverWesolowski::solution ProverWesolowski::operator()(const VerifierWesolowski &verifier) const {
  return operator()(verifier, false);
}

typename ProverWesolowski::solution ProverWesolowski::operator()(const VerifierWesolowski &verifier, const std::atomic_bool &cancelled) const
{
  // Reset timing
  BN_CTX *ctx = ctx_ptr.get();
  BN_CTX_start(ctx);

  // ------------------------------------------------
  // 1) Extract puzzle info
  // ------------------------------------------------
  const RSWPuzzle puzzle = verifier.get_RSWPuzzle();
  Hash2Prime hash        = verifier.get_Hash();

  const bytevec _T = puzzle.get_T();  // exponent T, in bytes
  const bytevec _N = puzzle.get_N();  // modulus N, in bytes
  const bytevec _x = puzzle.get_x();  // base x, in bytes

  // ------------------------------------------------
  // 2) Allocate BIGNUMs
  // ------------------------------------------------
  BIGNUM* bnT   = BN_CTX_get(ctx);
  BIGNUM* bnN   = BN_CTX_get(ctx);
  BIGNUM* bnX   = BN_CTX_get(ctx);
  BIGNUM* bnY   = BN_CTX_get(ctx);   // final y in normal domain
  BIGNUM* bnPi  = BN_CTX_get(ctx);   // final pi in normal domain
  BIGNUM* bnP   = BN_CTX_get(ctx);   // prime from hash
  BIGNUM* bnR   = BN_CTX_get(ctx);   // used in pi's loop
  BIGNUM* bnTmp = BN_CTX_get(ctx);   // scratch

  // ------------------------------------------------
  // 3) Initialize from byte arrays
  // ------------------------------------------------
  BN_bin2bn(_T.data(), static_cast<int>(_T.size()), bnT);
  BN_bin2bn(_N.data(), static_cast<int>(_N.size()), bnN);
  BN_bin2bn(_x.data(), static_cast<int>(_x.size()), bnX);

  // For simplicity, assume T fits in 64 bits:
  const uint64_t T_val = BN_get_word(bnT);

  // ------------------------------------------------
  // 4) Setup Montgomery context & convert x
  // ------------------------------------------------
  BN_MONT_CTX* mont = BN_MONT_CTX_new();
  BN_MONT_CTX_set(mont, bnN, ctx);

  // x in Montgomery form
  BIGNUM* x_mont = BN_CTX_get(ctx);
  BN_to_montgomery(x_mont, bnX, mont, ctx);

  // We'll compute y in Montgomery form:
  BIGNUM* y_mont = BN_CTX_get(ctx);
  BN_copy(bnY, bnX);                  // y starts as x (normal domain)
  BN_to_montgomery(y_mont, bnY, mont, ctx);

  // For pi, we also keep pi in Montgomery form:
  BIGNUM* pi_mont = BN_CTX_get(ctx);

  // r starts at 1, pi starts at 1
  BN_one(bnR);
  BN_one(bnPi); // normal domain, but we'll set pi_mont later

  // ------------------------------------------------
  // 5) Single for-loop from i=0 to i < 2*T
  //    Phase 1: i < T  => repeated squaring for y
  //    At i == T       => finalize y, compute p
  //    Phase 2: i > T  => repeated steps for pi
  // ------------------------------------------------

  for (uint64_t i = 0; i < 2 * T_val; i++) {

    if(cancelled) {
      BN_CTX_end(ctx);
      BN_MONT_CTX_free(mont);
      return {};
    }

    if (i < T_val) {
      // ----- PHASE 1: Building y -----
      // y_mont = y_mont^2 mod N (Montgomery)
      BN_mod_mul_montgomery(y_mont, y_mont, y_mont, mont, ctx);

      if (i == (T_val - 1)) {

        // Convert y out of Mont
        BN_from_montgomery(bnY, y_mont, mont, ctx);

        // Now we compute p = hash(x||y)

        // Combine x||y in bnTmp (like your original approach)
        BN_copy(bnTmp, bnX);
        BN_lshift(bnTmp, bnTmp, BN_num_bits(bnN));
        BN_add(bnTmp, bnTmp, bnY);

        hash(bnTmp, bnP); // bnP <- H(x||y)

        // Prepare for Phase 2: pi
        // pi = 1 in Montgomery
        BN_one(bnPi);
        BN_to_montgomery(pi_mont, bnPi, mont, ctx);

        // r = 1
        BN_one(bnR);
      }
    } else {
      // ----- PHASE 2: Computing pi -----
      // We must have done the hash (didHash == true) by now
      // i runs from T_val to 2*T_val - 1 for pi steps
      // pi_mont = pi_mont^2 mod N (Montgomery)
      BN_mod_mul_montgomery(pi_mont, pi_mont, pi_mont, mont, ctx);

      // r = 2*r
      BN_lshift1(bnR, bnR);

      // if (r >= p) => r -= p, pi_mont *= x_mont
      if (BN_cmp(bnR, bnP) >= 0) {
        BN_sub(bnR, bnR, bnP);
        BN_mod_mul_montgomery(pi_mont, pi_mont, x_mont, mont, ctx);
      }

      // If i == 2*T_val - 1, we have finished pi
      if (i == (2 * T_val - 1)) {
        // Convert pi back to normal domain
        BN_from_montgomery(bnPi, pi_mont, mont, ctx);
      }
    }
  } // end for i in [0..2T)

  // ------------------------------------------------
  // 6) Export the results
  // ------------------------------------------------
  bytevec _y, _pi;
  bn2bytevec(bnY, _y);
  bn2bytevec(bnPi, _pi);

  BN_CTX_end(ctx);
  BN_MONT_CTX_free(mont);

  return std::make_pair(_pi, _y);
}
}  // namespace vdf
