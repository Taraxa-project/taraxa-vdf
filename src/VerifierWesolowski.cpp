#include "../include/VerifierWesolowski.h"
namespace vdf
{
VerifierWesolowski::VerifierWesolowski(
    const unsigned long _lambda,
    const unsigned long _t,
    const bytevec &_x,
    const unsigned long _lambdaRSW) : hash(_lambda),
                                      puzzle(_lambda, _t, _x, _lambdaRSW),
                                      lambda(_lambda),
                                      ctx_ptr(BN_CTX_free_ptr(BN_CTX_secure_new(), ::BN_CTX_free)) {}

VerifierWesolowski::VerifierWesolowski(
    const unsigned long _lambda,
    const unsigned long _t,
    const bytevec &_x,
    const bytevec &N) : hash(_lambda),
                        puzzle(_lambda, _t, _x, N),
                        lambda(_lambda),
                        ctx_ptr(BN_CTX_free_ptr(BN_CTX_secure_new(), ::BN_CTX_free)) {}

Hash2Prime VerifierWesolowski::get_Hash() const
{
  return Hash2Prime(lambda);
}

RSWPuzzle VerifierWesolowski::get_RSWPuzzle() const
{
  return puzzle;
}

bool VerifierWesolowski::operator()(const solution &sol) const
{
  BN_CTX *ctx = ctx_ptr.get();
  BN_CTX_start(ctx);

  // constants
  const unsigned long t = puzzle.get_log2T();
  const bytevec _T = puzzle.get_T();
  const bytevec _N = puzzle.get_N();
  const bytevec _x = puzzle.get_x();
  const bytevec _pi = sol.first;
  const bytevec _y = sol.second;

  // helper variables
  BIGNUM *pi = BN_CTX_get(ctx);
  BIGNUM *N = BN_CTX_get(ctx);
  BIGNUM *T = BN_CTX_get(ctx);
  BIGNUM *h = BN_CTX_get(ctx);
  BIGNUM *p = BN_CTX_get(ctx);
  BIGNUM *r = BN_CTX_get(ctx);
  BIGNUM *x = BN_CTX_get(ctx);
  BIGNUM *y = BN_CTX_get(ctx);
  BIGNUM *xy = BN_CTX_get(ctx);
  BIGNUM *BN_value_two = BN_CTX_get(ctx);

  // set initial values
  BN_bin2bn(_x.data(), (int)_x.size(), x);
  BN_bin2bn(_T.data(), (int)_T.size(), T);
  BN_bin2bn(_N.data(), (int)_N.size(), N);
  BN_bin2bn(_y.data(), (int)_y.size(), y);
  BN_bin2bn(_pi.data(), (int)_pi.size(), pi);
  BN_set_word(BN_value_two, 2);

  // hash x||y
  BN_copy(xy, x);
  BN_lshift(xy, xy, BN_num_bits(N));
  BN_add(xy, xy, y);
  hash(xy, p);

  // compute r
  BN_mod_exp(r, BN_value_two, T, p, ctx);

  // compute h and compare to y
  BN_mod_exp(x, x, r, N, ctx);
  BN_mod_exp(pi, pi, p, N, ctx);
  BN_mod_mul(h, x, pi, N, ctx);

  bool result = (!BN_is_zero(y)) && (!BN_is_zero(pi)) && BN_cmp(y, h) == 0;
  BN_CTX_end(ctx);
  return result;
}
} // namespace vdf
