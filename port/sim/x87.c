/*
 * Exact software x87 extended-precision arithmetic.  See x87.h and
 * docs/port/X87.md.  Integer-only; no host floating point is evaluated.
 */
#include <string.h>
#include "port/sim/x87.h"

#define BIAS      16383
#define EXP_INF   0x7FFF
#define J_BIT     UINT64_C(0x8000000000000000)
#define QUIET_BIT UINT64_C(0x4000000000000000)

/* ------------------------------------------------------------ helpers */

static x87 mk(unsigned se, uint64_t m)
{
   x87 r;
   r.m = m;
   r.se = (uint16_t)se;
   return r;
}

static x87 zero(int sign) { return mk((unsigned)sign << 15, 0); }
static x87 inf(int sign) { return mk(((unsigned)sign << 15) | EXP_INF, J_BIT); }
static x87 indefinite(void) { return mk(0xFFFF, UINT64_C(0xC000000000000000)); }

static int clz64(uint64_t v) /* v != 0 */
{
#if defined(__GNUC__) || defined(__clang__)
   return __builtin_clzll(v);
#else
   int n = 0;
   if (!(v >> 32)) { n += 32; v <<= 32; }
   if (!(v >> 48)) { n += 16; v <<= 16; }
   if (!(v >> 56)) { n += 8; v <<= 8; }
   if (!(v >> 60)) { n += 4; v <<= 4; }
   if (!(v >> 62)) { n += 2; v <<= 2; }
   if (!(v >> 63)) { n += 1; }
   return n;
#endif
}

/* 64x64 -> 128 multiplication.  The portable 32-bit-limb path is the
 * default and the verified one; X87_USE_INT128 opts into __int128. */
static void mul64(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
#if defined(X87_USE_INT128) && defined(__SIZEOF_INT128__)
   unsigned __int128 p = (unsigned __int128)a * b;
   *hi = (uint64_t)(p >> 64);
   *lo = (uint64_t)p;
#else
   uint64_t a0 = (uint32_t)a, a1 = a >> 32, b0 = (uint32_t)b, b1 = b >> 32;
   uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
   uint64_t mid = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
   *lo = (mid << 32) | (uint32_t)p00;
   *hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
#endif
}

/* hi:lo >>= n, OR-ing every shifted-out bit into the lowest bit (sticky). */
static void shr128_jam(uint64_t *hi, uint64_t *lo, int32_t n)
{
   uint64_t h = *hi, l = *lo;
   if (n <= 0)
      return;
   if (n < 64) {
      l = (h << (64 - n)) | (l >> n) | ((l << (64 - n)) != 0);
      h >>= n;
   } else if (n == 64) {
      l = h | (l != 0);
      h = 0;
   } else if (n < 128) {
      l = (h >> (n - 64)) | (((h << (128 - n)) | l) != 0);
      h = 0;
   } else {
      l = (h | l) != 0;
      h = 0;
   }
   *hi = h;
   *lo = l;
}

/* ------------------------------------------------------------ unpacking */

enum { C_ZERO, C_FIN, C_INF, C_NAN, C_BAD };

typedef struct {
   int cls;
   int sign;
   int32_t exp;  /* biased; for C_FIN the value is sig * 2^(exp - BIAS - 63) */
   uint64_t sig; /* C_FIN: normalised, bit 63 set */
} unp;

static unp unpack(x87 a)
{
   unp u;
   int32_t e = a.se & 0x7FFF;
   u.sign = a.se >> 15;
   u.exp = e;
   u.sig = a.m;
   if (e == EXP_INF) {
      if (!(a.m & J_BIT))
         u.cls = C_BAD; /* pseudo-infinity / pseudo-NaN */
      else
         u.cls = (a.m << 1) ? C_NAN : C_INF;
   } else if (e == 0) {
      if (!a.m) {
         u.cls = C_ZERO;
      } else { /* denormal or pseudo-denormal: effective exponent 1 */
         int s = clz64(a.m);
         u.cls = C_FIN;
         u.sig = a.m << s;
         u.exp = 1 - s;
      }
   } else {
      u.cls = (a.m & J_BIT) ? C_FIN : C_BAD; /* unnormals are invalid */
   }
   return u;
}

/* x87 NaN propagation; at least one operand is a NaN. */
static x87 propagate_nan(x87 a, const unp *ua, x87 b, const unp *ub)
{
   int an = ua->cls == C_NAN, bn = ub->cls == C_NAN;
   x87 r;
   if (an && bn) {
      int aq = (a.m & QUIET_BIT) != 0, bq = (b.m & QUIET_BIT) != 0;
      if (aq != bq)
         r = aq ? a : b;              /* the QNaN operand */
      else if (a.m != b.m)
         r = a.m > b.m ? a : b;       /* larger significand */
      else
         r = a.se < b.se ? a : b;     /* equal significands: positive one */
   } else {
      r = an ? a : b;
   }
   r.m |= QUIET_BIT;
   return r;
}

/* ------------------------------------------------------------ rounding */

/* Round (sign, exp, hi:lo) to `pc` significand bits, nearest-even, and pack.
 * hi has bit 63 set unless hi:lo is zero; the value is
 * hi:lo * 2^(exp - BIAS - 127).  exp may lie outside 1..0x7FFE: tiny results
 * are denormalised before rounding (masked underflow), huge ones overflow to
 * infinity (masked overflow, round-to-nearest). */
static x87 round_pack(int sign, int32_t exp, uint64_t hi, uint64_t lo, int pc)
{
   if (!hi && !lo)
      return zero(sign);
   if (exp >= EXP_INF)
      return inf(sign);
   if (exp <= 0) {
      shr128_jam(&hi, &lo, 1 - exp);
      exp = 0;
   }
   if (pc != X87_PC53 && pc != X87_PC24)
      pc = X87_PC64;
   if (pc == X87_PC64) {
      if (lo > J_BIT || (lo == J_BIT && (hi & 1))) {
         hi++;
         if (!hi) {
            hi = J_BIT;
            exp++;
         }
      }
   } else {
      int d = 64 - pc;
      uint64_t unit = (uint64_t)1 << d, mask = unit - 1, half = unit >> 1;
      uint64_t rb = hi & mask;
      hi &= ~mask;
      if (rb > half || (rb == half && (lo || (hi & unit)))) {
         hi += unit;
         if (!hi) {
            hi = J_BIT;
            exp++;
         }
      }
   }
   if (exp == 0) {
      if (!hi)
         return zero(sign);
      if (hi & J_BIT)
         exp = 1; /* rounded up into the normal range */
   }
   if (exp >= EXP_INF)
      return inf(sign);
   return mk(((unsigned)sign << 15) | (unsigned)exp, hi);
}

/* ------------------------------------------------------------ add / sub */

static x87 add_mag(const unp *a, const unp *b, int sign, int pc)
{
   const unp *x = a, *y = b;
   uint64_t hi, lo, yh, yl;
   int32_t exp;
   if (x->exp < y->exp) {
      x = b;
      y = a;
   }
   yh = y->sig;
   yl = 0;
   shr128_jam(&yh, &yl, x->exp - y->exp);
   exp = x->exp;
   lo = yl;
   hi = x->sig + yh;
   if (hi < yh) { /* carry out of bit 63 */
      lo = (hi << 63) | (lo >> 1) | (lo & 1);
      hi = (hi >> 1) | J_BIT;
      exp++;
   }
   return round_pack(sign, exp, hi, lo, pc);
}

/* |a| - |b| with a carrying sign `sign`. */
static x87 sub_mag(const unp *a, const unp *b, int sign, int pc)
{
   const unp *x = a, *y = b;
   uint64_t hi, lo, yh, yl;
   int32_t exp;
   int s;
   if (a->exp < b->exp || (a->exp == b->exp && a->sig < b->sig)) {
      x = b;
      y = a;
      sign ^= 1;
   } else if (a->exp == b->exp && a->sig == b->sig) {
      return zero(0); /* exact cancellation is +0 under round-to-nearest */
   }
   yh = y->sig;
   yl = 0;
   shr128_jam(&yh, &yl, x->exp - y->exp);
   exp = x->exp;
   lo = 0 - yl;
   hi = x->sig - yh - (yl != 0);
   if (!hi) { /* only when the operands were within one binade: exact */
      hi = lo;
      lo = 0;
      exp -= 64;
   }
   s = clz64(hi);
   if (s) {
      hi = (hi << s) | (lo >> (64 - s));
      lo <<= s;
      exp -= s;
   }
   return round_pack(sign, exp, hi, lo, pc);
}

static x87 addsub(x87 a, x87 b, int negate_b, int pc)
{
   unp ua = unpack(a), ub = unpack(b);
   if (ua.cls == C_NAN || ub.cls == C_NAN)
      return propagate_nan(a, &ua, b, &ub);
   if (ua.cls == C_BAD || ub.cls == C_BAD)
      return indefinite();
   ub.sign ^= negate_b;
   if (ua.cls == C_INF) {
      if (ub.cls == C_INF && ua.sign != ub.sign)
         return indefinite();
      return inf(ua.sign);
   }
   if (ub.cls == C_INF)
      return inf(ub.sign);
   if (ua.cls == C_ZERO && ub.cls == C_ZERO)
      return zero(ua.sign & ub.sign);
   if (ua.cls == C_ZERO)
      return round_pack(ub.sign, ub.exp, ub.sig, 0, pc);
   if (ub.cls == C_ZERO)
      return round_pack(ua.sign, ua.exp, ua.sig, 0, pc);
   if (ua.sign == ub.sign)
      return add_mag(&ua, &ub, ua.sign, pc);
   return sub_mag(&ua, &ub, ua.sign, pc);
}

x87 x87_add_pc(x87 a, x87 b, int pc) { return addsub(a, b, 0, pc); }
x87 x87_sub_pc(x87 a, x87 b, int pc) { return addsub(a, b, 1, pc); }

/* ------------------------------------------------------------ mul / div */

x87 x87_mul_pc(x87 a, x87 b, int pc)
{
   unp ua = unpack(a), ub = unpack(b);
   uint64_t hi, lo;
   int32_t exp;
   int sign;
   if (ua.cls == C_NAN || ub.cls == C_NAN)
      return propagate_nan(a, &ua, b, &ub);
   if (ua.cls == C_BAD || ub.cls == C_BAD)
      return indefinite();
   sign = ua.sign ^ ub.sign;
   if (ua.cls == C_INF || ub.cls == C_INF) {
      if (ua.cls == C_ZERO || ub.cls == C_ZERO)
         return indefinite();
      return inf(sign);
   }
   if (ua.cls == C_ZERO || ub.cls == C_ZERO)
      return zero(sign);
   mul64(ua.sig, ub.sig, &hi, &lo);
   exp = ua.exp + ub.exp - BIAS + 1;
   if (!(hi & J_BIT)) {
      hi = (hi << 1) | (lo >> 63);
      lo <<= 1;
      exp--;
   }
   return round_pack(sign, exp, hi, lo, pc);
}

x87 x87_div_pc(x87 a, x87 b, int pc)
{
   unp ua = unpack(a), ub = unpack(b);
   uint64_t sa, sb, r, q, lo;
   int32_t exp;
   int sign, i;
   if (ua.cls == C_NAN || ub.cls == C_NAN)
      return propagate_nan(a, &ua, b, &ub);
   if (ua.cls == C_BAD || ub.cls == C_BAD)
      return indefinite();
   sign = ua.sign ^ ub.sign;
   if (ua.cls == C_INF)
      return ub.cls == C_INF ? indefinite() : inf(sign);
   if (ub.cls == C_INF)
      return zero(sign);
   if (ub.cls == C_ZERO)
      return ua.cls == C_ZERO ? indefinite() : inf(sign); /* masked zero-divide */
   if (ua.cls == C_ZERO)
      return zero(sign);

   /* Restoring long division producing 64 quotient bits, one guard bit and
    * a sticky bit from the exact remainder.  Invariant: r < sb. */
   sa = ua.sig;
   sb = ub.sig;
   if (sa >= sb) {
      exp = ua.exp - ub.exp + BIAS;
      r = sa - sb;
   } else {
      exp = ua.exp - ub.exp + BIAS - 1;
      r = (sa << 1) - sb; /* 2*sa - sb is in [0, sb): exact modulo 2^64 */
   }
   q = 1; /* leading quotient bit */
   lo = 0;
   for (i = 0; i < 64; i++) { /* 63 more quotient bits, then the guard bit */
      uint64_t carry = r >> 63, bit = 0;
      r <<= 1;
      if (carry || r >= sb) {
         r -= sb; /* exact: the true 65-bit value minus sb is below sb */
         bit = 1;
      }
      if (i < 63)
         q = (q << 1) | bit;
      else
         lo = bit << 63;
   }
   lo |= (r != 0); /* sticky: inexact remainder */
   return round_pack(sign, exp, q, lo, pc);
}

x87 x87_add(x87 a, x87 b) { return addsub(a, b, 0, X87_PC64); }
x87 x87_sub(x87 a, x87 b) { return addsub(a, b, 1, X87_PC64); }
x87 x87_mul(x87 a, x87 b) { return x87_mul_pc(a, b, X87_PC64); }
x87 x87_div(x87 a, x87 b) { return x87_div_pc(a, b, X87_PC64); }

x87 x87_neg(x87 a) { a.se ^= 0x8000; return a; }
x87 x87_abs(x87 a) { a.se &= 0x7FFF; return a; }

/* ------------------------------------------------------------ loads */

x87 x87_make(uint16_t se, uint64_t m) { return mk(se, m); }

static x87 from_mag(int sign, uint64_t mag)
{
   int s;
   if (!mag)
      return zero(0); /* fild never produces -0 */
   s = clz64(mag);
   return mk(((unsigned)sign << 15) | (unsigned)(BIAS + 63 - s), mag << s);
}

x87 x87_from_i32(int32_t v)
{
   return v < 0 ? from_mag(1, 0 - (uint64_t)(int64_t)v) : from_mag(0, (uint64_t)v);
}

x87 x87_from_i64(int64_t v)
{
   return v < 0 ? from_mag(1, 0 - (uint64_t)v) : from_mag(0, (uint64_t)v);
}

/* IEEE binary load: fb fraction bits, bias, all-ones exponent field emax. */
static x87 from_ieee(int sign, uint32_t e, uint64_t f, int fb, int bias, uint32_t emax)
{
   unsigned s15 = (unsigned)sign << 15;
   if (e == emax) {
      if (!f)
         return inf(sign);
      return mk(s15 | EXP_INF, J_BIT | QUIET_BIT | (f << (63 - fb))); /* SNaN quieted */
   }
   if (e == 0) {
      int s;
      uint64_t m;
      if (!f)
         return zero(sign);
      m = f << (63 - fb);
      s = clz64(m);
      return mk(s15 | (unsigned)(BIAS - bias + 1 - s), m << s);
   }
   return mk(s15 | (unsigned)((int32_t)e - bias + BIAS), J_BIT | (f << (63 - fb)));
}

x87 x87_from_f64_bits(uint64_t b)
{
   return from_ieee((int)(b >> 63), (uint32_t)(b >> 52) & 0x7FF,
                    b & ((UINT64_C(1) << 52) - 1), 52, 1023, 0x7FF);
}

x87 x87_from_f32_bits(uint32_t b)
{
   return from_ieee((int)(b >> 31), (b >> 23) & 0xFF, b & ((1u << 23) - 1), 23, 127, 0xFF);
}

x87 x87_from_f64(double v)
{
   uint64_t b;
   memcpy(&b, &v, sizeof b);
   return x87_from_f64_bits(b);
}

x87 x87_from_f32(float v)
{
   uint32_t b;
   memcpy(&b, &v, sizeof b);
   return x87_from_f32_bits(b);
}

/* ------------------------------------------------------------ stores */

/* Round a finite nonzero unpacked value to an IEEE binary format and return
 * the bits without sign.  fb = fraction bits, bias, emax = all-ones field. */
static uint64_t pack_ieee(const unp *u, int fb, int32_t bias, int32_t emax)
{
   int32_t be = u->exp - BIAS + bias; /* target biased exponent */
   int32_t total = 63 - fb;           /* significand bits to drop */
   uint64_t kept, bits;
   int inc;
   if (be >= emax)
      return (uint64_t)emax << fb; /* overflow -> infinity */
   if (be <= 0) {                  /* denormal: drop 1 - be more bits */
      total += 1 - be;
      be = 1;
   }
   if (total > 64) {
      kept = 0;
      inc = 0; /* below half the smallest denormal */
   } else if (total == 64) {
      kept = 0;
      inc = u->sig > J_BIT;
   } else {
      uint64_t unit = (uint64_t)1 << total;
      uint64_t rem = u->sig & (unit - 1), half = unit >> 1;
      kept = u->sig >> total;
      inc = rem > half || (rem == half && (kept & 1));
   }
   /* kept carries the hidden bit, which adds the 1 missing from be - 1; a
    * denormal rounding up to 2^fb becomes the smallest normal naturally. */
   bits = ((uint64_t)(be - 1) << fb) + kept + (uint64_t)inc;
   if ((bits >> fb) >= (uint64_t)emax)
      return (uint64_t)emax << fb;
   return bits;
}

static uint64_t to_ieee(x87 a, int fb, int32_t bias, int32_t emax, int width)
{
   unp u = unpack(a);
   uint64_t sign = (uint64_t)u.sign << (width - 1);
   uint64_t expall = (uint64_t)emax << fb;
   switch (u.cls) {
   case C_ZERO:
      return sign;
   case C_INF:
      return sign | expall;
   case C_NAN: /* quieted, significand truncated */
      return sign | expall | ((uint64_t)1 << (fb - 1)) |
             ((a.m & ~J_BIT) >> (63 - fb));
   case C_BAD: /* invalid operand: default NaN */
      return ((uint64_t)1 << (width - 1)) | expall | ((uint64_t)1 << (fb - 1));
   default:
      return sign | pack_ieee(&u, fb, bias, emax);
   }
}

uint64_t x87_to_f64_bits(x87 a) { return to_ieee(a, 52, 1023, 0x7FF, 64); }
uint32_t x87_to_f32_bits(x87 a) { return (uint32_t)to_ieee(a, 23, 127, 0xFF, 32); }

double x87_to_f64(x87 a)
{
   uint64_t b = x87_to_f64_bits(a);
   double v;
   memcpy(&v, &b, sizeof v);
   return v;
}

float x87_to_f32(x87 a)
{
   uint32_t b = x87_to_f32_bits(a);
   float v;
   memcpy(&v, &b, sizeof v);
   return v;
}

x87 x87_rnd_f64(x87 a) { return x87_from_f64_bits(x87_to_f64_bits(a)); }
x87 x87_rnd_f32(x87 a) { return x87_from_f32_bits(x87_to_f32_bits(a)); }

/* fistp: bits = 32 or 64; chop = RC 11 (truncate), otherwise nearest-even. */
static int64_t to_int(x87 a, int bits, int chop)
{
   const int64_t indef = INT64_MIN;
   unp u = unpack(a);
   uint64_t mag, limit;
   int32_t sh;
   if (u.cls == C_ZERO)
      return 0;
   if (u.cls != C_FIN)
      return indef;
   sh = BIAS + 63 - u.exp; /* value = sig >> sh */
   if (sh < 0)
      return indef;
   if (sh == 0) {
      mag = u.sig;
   } else if (sh > 64) {
      mag = 0; /* |value| < 1/2 */
   } else {
      uint64_t unit = sh == 64 ? 0 : (uint64_t)1 << sh;
      uint64_t rem = sh == 64 ? u.sig : u.sig & (unit - 1);
      uint64_t half = (uint64_t)1 << (sh - 1);
      mag = sh == 64 ? 0 : u.sig >> sh;
      if (!chop && (rem > half || (rem == half && (mag & 1))))
         mag++;
   }
   limit = (uint64_t)1 << (bits - 1);
   if (u.sign) {
      if (mag > limit)
         return indef;
      return mag == limit ? (bits == 64 ? INT64_MIN : (int64_t)INT32_MIN)
                          : -(int64_t)mag;
   }
   if (mag >= limit)
      return indef;
   return (int64_t)mag;
}

int32_t x87_to_i32_trunc(x87 a)
{
   int64_t v = to_int(a, 32, 1);
   return v == INT64_MIN ? INT32_MIN : (int32_t)v;
}

int32_t x87_to_i32_rn(x87 a)
{
   int64_t v = to_int(a, 32, 0);
   return v == INT64_MIN ? INT32_MIN : (int32_t)v;
}

int64_t x87_to_i64_trunc(x87 a) { return to_int(a, 64, 1); }
int64_t x87_to_i64_rn(x87 a) { return to_int(a, 64, 0); }

/* ------------------------------------------------------------ compare */

int x87_cmp(x87 a, x87 b)
{
   unp ua = unpack(a), ub = unpack(b);
   int mag;
   if (ua.cls == C_NAN || ub.cls == C_NAN || ua.cls == C_BAD || ub.cls == C_BAD)
      return X87_UNORDERED;
   if (ua.cls == C_ZERO && ub.cls == C_ZERO)
      return X87_EQ;
   if (ua.cls == C_ZERO)
      return ub.sign ? X87_GT : X87_LT;
   if (ub.cls == C_ZERO)
      return ua.sign ? X87_LT : X87_GT;
   if (ua.sign != ub.sign)
      return ua.sign ? X87_LT : X87_GT;
   /* same sign, both finite nonzero or infinite: compare magnitudes */
   if (ua.cls == C_INF || ub.cls == C_INF)
      mag = (ua.cls == C_INF) - (ub.cls == C_INF);
   else if (ua.exp != ub.exp)
      mag = ua.exp < ub.exp ? -1 : 1;
   else
      mag = ua.sig < ub.sig ? -1 : ua.sig > ub.sig;
   return ua.sign ? -mag : mag;
}

int x87_fucom(x87 st0, x87 st1)
{
   switch (x87_cmp(st0, st1)) {
   case X87_GT: return 0;
   case X87_LT: return X87_CC_C0;
   case X87_EQ: return X87_CC_C3;
   default:     return X87_CC_C3 | X87_CC_C2 | X87_CC_C0;
   }
}

uint16_t x87_fucom_sw(x87 st0, x87 st1)
{
   int cc = x87_fucom(st0, st1);
   return (uint16_t)(((cc & X87_CC_C0) ? 0x0100 : 0) | ((cc & X87_CC_C2) ? 0x0400 : 0) |
                     ((cc & X87_CC_C3) ? 0x4000 : 0));
}

bool x87_lt(x87 a, x87 b) { return x87_cmp(a, b) == X87_LT; }
bool x87_gt(x87 a, x87 b) { return x87_cmp(a, b) == X87_GT; }
bool x87_eq(x87 a, x87 b) { return x87_cmp(a, b) == X87_EQ; }
bool x87_ne(x87 a, x87 b) { return x87_cmp(a, b) != X87_EQ; }
bool x87_le(x87 a, x87 b) { int c = x87_cmp(a, b); return c == X87_LT || c == X87_EQ; }
bool x87_ge(x87 a, x87 b) { int c = x87_cmp(a, b); return c == X87_GT || c == X87_EQ; }

bool x87_is_nan(x87 a) { return unpack(a).cls == C_NAN; }
bool x87_identical(x87 a, x87 b) { return a.m == b.m && a.se == b.se; }
