/*
 * Verification of port/sim/x87 (software x87 extended precision).
 *
 * Every operation is run over deterministic pseudo-random operand
 * distributions (op x distribution = one block).  On x86/x86-64 GCC/Clang,
 * where long double is the x87 80-bit format, each case is also executed on
 * the FPU (inline asm, control word 0x037F / 0x027F / 0x007F) and compared
 * bit-for-bit.  Independently of the host, the emulator results of the first
 * X87V_CASES cases of every block are hashed and checked against
 * x87_vectors.h (recorded from a clean native run), and the first KAT cases
 * of each block are checked individually, so non-x86 builds (ARM, MSVC)
 * verify themselves too.
 *
 *   test_x87                     default run (native compare + vectors)
 *   test_x87 --cases N           cases per block (default X87V_CASES)
 *   test_x87 --seed S            different operands (disables vector checks)
 *   test_x87 --no-native         vector checks only (what non-x86 hosts do)
 *   test_x87 --only NAME         only blocks whose op name contains NAME
 *   test_x87 --gen-vectors FILE  write the vector header (native, 0 failures)
 */
#define _CRT_SECURE_NO_WARNINGS
#include <float.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "port/sim/x87.h"

#define KAT 4

typedef struct {
   const char *op, *dist;
   uint64_t hash;
   uint64_t kat_lo[KAT];
   uint16_t kat_hi[KAT];
} x87v_entry;

#include "x87_vectors.h"

#if (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__)) && \
    LDBL_MANT_DIG == 64
#define HAVE_NATIVE 1
#else
#define HAVE_NATIVE 0
#endif

#define BIAS 16383
#define J_BIT UINT64_C(0x8000000000000000)

/* ------------------------------------------------------------ results */

typedef struct {
   uint64_t lo;
   uint16_t hi;
} res;

static res rx(x87 v) { res r; r.lo = v.m; r.hi = v.se; return r; }
static res ri(uint64_t v) { res r; r.lo = v; r.hi = 0; return r; }

/* ------------------------------------------------------------ operations */

enum {
   O_ADD, O_SUB, O_MUL, O_DIV, O_NEG, O_ABS, O_CMP, O_TOF64, O_TOF32, O_TOI32T, O_TOI64T,
   O_TOI32R, O_TOI64R, O_FROMF64, O_FROMF32, O_FROMI32, O_FROMI64, O_MAC, O_MAC_F64,
   O_SUBDIV_F32, O_POS_F64, O_IDIV, O_IDIV_CMP, O_TRUNC_MAC
};

typedef struct {
   const char *name;
   int kind, pc;
} opdesc;

static const opdesc ops[] = {
   { "add64", O_ADD, 64 }, { "sub64", O_SUB, 64 }, { "mul64", O_MUL, 64 }, { "div64", O_DIV, 64 },
   { "add53", O_ADD, 53 }, { "sub53", O_SUB, 53 }, { "mul53", O_MUL, 53 }, { "div53", O_DIV, 53 },
   { "add24", O_ADD, 24 }, { "sub24", O_SUB, 24 }, { "mul24", O_MUL, 24 }, { "div24", O_DIV, 24 },
   { "neg", O_NEG, 64 }, { "abs", O_ABS, 64 }, { "fucom+relops", O_CMP, 64 },
   { "to_f64", O_TOF64, 64 }, { "to_f32", O_TOF32, 64 },
   { "to_i32_trunc", O_TOI32T, 64 }, { "to_i64_trunc", O_TOI64T, 64 },
   { "to_i32_rn", O_TOI32R, 64 }, { "to_i64_rn", O_TOI64R, 64 },
   { "from_f64", O_FROMF64, 64 }, { "from_f32", O_FROMF32, 64 },
   { "from_i32", O_FROMI32, 64 }, { "from_i64", O_FROMI64, 64 },
   { "chain_mac", O_MAC, 64 }, { "chain_mac_f64", O_MAC_F64, 64 },
   { "chain_subdiv_f32", O_SUBDIV_F32, 64 }, { "chain_pos_f64", O_POS_F64, 64 },
   { "chain_idiv", O_IDIV, 64 }, { "chain_idiv_fucom", O_IDIV_CMP, 64 },
   { "chain_trunc_mac", O_TRUNC_MAC, 64 },
};
#define NOPS ((int)(sizeof ops / sizeof ops[0]))

/* One test case: three x87 operands, raw double/float bits and two ints. */
typedef struct {
   x87 a, b, c;
   uint64_t r; /* from_f64 bits, from_i64 value */
   uint32_t q; /* from_f32 bits */
   int32_t i, j;
} kase;

static unsigned rel_bits(int lt, int le, int gt, int ge, int eq, int ne)
{
   return (unsigned)(lt | le << 1 | gt << 2 | ge << 3 | eq << 4 | ne << 5);
}

static res emu_eval(const opdesc *o, const kase *k)
{
   int pc = o->pc;
   switch (o->kind) {
   case O_ADD: return rx(x87_add_pc(k->a, k->b, pc));
   case O_SUB: return rx(x87_sub_pc(k->a, k->b, pc));
   case O_MUL: return rx(x87_mul_pc(k->a, k->b, pc));
   case O_DIV: return rx(x87_div_pc(k->a, k->b, pc));
   case O_NEG: return rx(x87_neg(k->a));
   case O_ABS: return rx(x87_abs(k->a));
   case O_CMP:
      return ri(x87_fucom_sw(k->a, k->b) |
                rel_bits(x87_lt(k->a, k->b), x87_le(k->a, k->b), x87_gt(k->a, k->b),
                         x87_ge(k->a, k->b), x87_eq(k->a, k->b), x87_ne(k->a, k->b)));
   case O_TOF64: return ri(x87_to_f64_bits(k->a));
   case O_TOF32: return ri(x87_to_f32_bits(k->a));
   case O_TOI32T: return ri((uint32_t)x87_to_i32_trunc(k->a));
   case O_TOI64T: return ri((uint64_t)x87_to_i64_trunc(k->a));
   case O_TOI32R: return ri((uint32_t)x87_to_i32_rn(k->a));
   case O_TOI64R: return ri((uint64_t)x87_to_i64_rn(k->a));
   case O_FROMF64: return rx(x87_from_f64_bits(k->r));
   case O_FROMF32: return rx(x87_from_f32_bits(k->q));
   case O_FROMI32: return rx(x87_from_i32(k->i));
   case O_FROMI64: return rx(x87_from_i64((int64_t)k->r));
   case O_MAC: return rx(x87_add(x87_mul(k->a, k->b), k->c));
   case O_MAC_F64: return ri(x87_to_f64_bits(x87_add(x87_mul(k->a, k->b), k->c)));
   case O_SUBDIV_F32: return ri(x87_to_f32_bits(x87_div(x87_sub(k->a, k->b), k->c)));
   case O_POS_F64:
      return ri(x87_to_f64_bits(x87_add(x87_rnd_f64(k->a), x87_mul(x87_rnd_f64(k->b), k->c))));
   case O_IDIV: return rx(x87_div(x87_from_i32(k->i), x87_from_i32(k->j)));
   case O_IDIV_CMP:
      return ri(x87_fucom_sw(x87_div(x87_from_i32(k->i), x87_from_i32(k->j)), k->a));
   case O_TRUNC_MAC:
      return ri((uint32_t)x87_to_i32_trunc(x87_add(x87_mul(k->a, k->b), k->c)));
   }
   return ri(0);
}

/* ------------------------------------------------------------ native x87 */

#if HAVE_NATIVE
typedef union {
   long double v;
   unsigned char b[sizeof(long double)];
} ldu;

static ldu to_ld(x87 x)
{
   ldu u;
   memset(&u, 0, sizeof u);
   memcpy(u.b, &x.m, 8);
   memcpy(u.b + 8, &x.se, 2);
   return u;
}

static x87 from_ld(const ldu *u)
{
   x87 r;
   memcpy(&r.m, u->b, 8);
   memcpy(&r.se, u->b + 8, 2);
   return r;
}

static void hw_set_cw(uint16_t cw) { __asm__ __volatile__("fldcw %0" : : "m"(cw)); }

static uint16_t cw_for_pc(int pc)
{
   return pc == 24 ? 0x007F : pc == 53 ? 0x027F : 0x037F;
}

/* st0 = a, st1 = b; "op %st(1), %st" computes st0 = a op b. */
#define HW_BIN(fn, insn)                                                              \
   static x87 fn(x87 a, x87 b)                                                        \
   {                                                                                  \
      ldu A = to_ld(a), B = to_ld(b), R;                                              \
      __asm__ __volatile__("fldt %2\n\tfldt %1\n\t" insn " %%st(1), %%st\n\t"         \
                           "fstpt %0\n\tfstp %%st(0)"                                  \
                           : "=m"(R.v)                                                \
                           : "m"(A.v), "m"(B.v));                                     \
      return from_ld(&R);                                                             \
   }
HW_BIN(hw_add, "fadd")
HW_BIN(hw_sub, "fsub")
HW_BIN(hw_mul, "fmul")
HW_BIN(hw_div, "fdiv")

#define HW_UN(fn, insn)                                                               \
   static x87 fn(x87 a)                                                               \
   {                                                                                  \
      ldu A = to_ld(a), R;                                                            \
      __asm__ __volatile__("fldt %1\n\t" insn "\n\tfstpt %0" : "=m"(R.v) : "m"(A.v)); \
      return from_ld(&R);                                                             \
   }
HW_UN(hw_neg, "fchs")
HW_UN(hw_abs, "fabs")

static uint64_t hw_to_f64(x87 a)
{
   ldu A = to_ld(a);
   uint64_t r;
   __asm__ __volatile__("fldt %1\n\tfstpl %0" : "=m"(r) : "m"(A.v));
   return r;
}

static uint32_t hw_to_f32(x87 a)
{
   ldu A = to_ld(a);
   uint32_t r;
   __asm__ __volatile__("fldt %1\n\tfstps %0" : "=m"(r) : "m"(A.v));
   return r;
}

static x87 hw_rnd_f64(x87 a)
{
   uint64_t d = hw_to_f64(a);
   ldu R;
   __asm__ __volatile__("fldl %1\n\tfstpt %0" : "=m"(R.v) : "m"(d));
   return from_ld(&R);
}

static int32_t hw_fist32(x87 a, uint16_t cw)
{
   ldu A = to_ld(a);
   int32_t r;
   uint16_t old;
   __asm__ __volatile__("fnstcw %1\n\tfldcw %3\n\tfldt %2\n\tfistpl %0\n\tfldcw %1"
                        : "=m"(r), "=m"(old)
                        : "m"(A.v), "m"(cw));
   return r;
}

static int64_t hw_fist64(x87 a, uint16_t cw)
{
   ldu A = to_ld(a);
   int64_t r;
   uint16_t old;
   __asm__ __volatile__("fnstcw %1\n\tfldcw %3\n\tfldt %2\n\tfistpll %0\n\tfldcw %1"
                        : "=m"(r), "=m"(old)
                        : "m"(A.v), "m"(cw));
   return r;
}

static x87 hw_from_f64(uint64_t bits)
{
   ldu R;
   __asm__ __volatile__("fldl %1\n\tfstpt %0" : "=m"(R.v) : "m"(bits));
   return from_ld(&R);
}

static x87 hw_from_f32(uint32_t bits)
{
   ldu R;
   __asm__ __volatile__("flds %1\n\tfstpt %0" : "=m"(R.v) : "m"(bits));
   return from_ld(&R);
}

static x87 hw_from_i32(int32_t v)
{
   ldu R;
   __asm__ __volatile__("fildl %1\n\tfstpt %0" : "=m"(R.v) : "m"(v));
   return from_ld(&R);
}

static x87 hw_from_i64(int64_t v)
{
   ldu R;
   __asm__ __volatile__("fildll %1\n\tfstpt %0" : "=m"(R.v) : "m"(v));
   return from_ld(&R);
}

/* fucom with a in st0, b in st1; returns C3/C2/C0 of the status word. */
static uint16_t hw_fucom_sw(x87 a, x87 b)
{
   ldu A = to_ld(a), B = to_ld(b);
   uint16_t sw;
   __asm__ __volatile__("fldt %2\n\tfldt %1\n\tfucompp\n\tfnstsw %0"
                        : "=m"(sw)
                        : "m"(A.v), "m"(B.v));
   return (uint16_t)(sw & 0x4500);
}

/* The compiler's own C relational operators on long double. */
static unsigned hw_rel(x87 a, x87 b)
{
   ldu A = to_ld(a), B = to_ld(b);
   volatile long double x = A.v, y = B.v;
   return rel_bits(x < y, x <= y, x > y, x >= y, x == y, x != y);
}

/* (a*b)+c kept on the register stack, then fstp qword. */
static uint64_t hw_mac_f64(x87 a, x87 b, x87 c)
{
   ldu A = to_ld(a), B = to_ld(b), C = to_ld(c);
   uint64_t r;
   __asm__ __volatile__("fldt %3\n\tfldt %2\n\tfldt %1\n\t"
                        "fmul %%st(1), %%st\n\tfadd %%st(2), %%st\n\t"
                        "fstpl %0\n\tfstp %%st(0)\n\tfstp %%st(0)"
                        : "=m"(r)
                        : "m"(A.v), "m"(B.v), "m"(C.v));
   return r;
}

static res hw_eval(const opdesc *o, const kase *k)
{
   switch (o->kind) {
   case O_ADD: return rx(hw_add(k->a, k->b));
   case O_SUB: return rx(hw_sub(k->a, k->b));
   case O_MUL: return rx(hw_mul(k->a, k->b));
   case O_DIV: return rx(hw_div(k->a, k->b));
   case O_NEG: return rx(hw_neg(k->a));
   case O_ABS: return rx(hw_abs(k->a));
   case O_CMP: return ri(hw_fucom_sw(k->a, k->b) | hw_rel(k->a, k->b));
   case O_TOF64: return ri(hw_to_f64(k->a));
   case O_TOF32: return ri(hw_to_f32(k->a));
   case O_TOI32T: return ri((uint32_t)hw_fist32(k->a, 0x0F7F));
   case O_TOI64T: return ri((uint64_t)hw_fist64(k->a, 0x0F7F));
   case O_TOI32R: return ri((uint32_t)hw_fist32(k->a, 0x037F));
   case O_TOI64R: return ri((uint64_t)hw_fist64(k->a, 0x037F));
   case O_FROMF64: return rx(hw_from_f64(k->r));
   case O_FROMF32: return rx(hw_from_f32(k->q));
   case O_FROMI32: return rx(hw_from_i32(k->i));
   case O_FROMI64: return rx(hw_from_i64((int64_t)k->r));
   case O_MAC: return rx(hw_add(hw_mul(k->a, k->b), k->c));
   case O_MAC_F64: return ri(hw_mac_f64(k->a, k->b, k->c));
   case O_SUBDIV_F32: return ri(hw_to_f32(hw_div(hw_sub(k->a, k->b), k->c)));
   case O_POS_F64:
      return ri(hw_to_f64(hw_add(hw_rnd_f64(k->a), hw_mul(hw_rnd_f64(k->b), k->c))));
   case O_IDIV: return rx(hw_div(hw_from_i32(k->i), hw_from_i32(k->j)));
   case O_IDIV_CMP: return ri(hw_fucom_sw(hw_div(hw_from_i32(k->i), hw_from_i32(k->j)), k->a));
   case O_TRUNC_MAC:
      return ri((uint32_t)hw_fist32(hw_add(hw_mul(k->a, k->b), k->c), 0x0F7F));
   }
   return ri(0);
}
#endif /* HAVE_NATIVE */

/* ------------------------------------------------------------ random */

typedef struct {
   uint64_t s;
} rng;

static uint64_t mix64(uint64_t z)
{
   z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
   z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
   return z ^ (z >> 31);
}

static uint64_t next(rng *r) { return mix64(r->s += UINT64_C(0x9E3779B97F4A7C15)); }
static uint32_t below(rng *r, uint32_t n) { return (uint32_t)(((next(r) >> 32) * n) >> 32); }
static int32_t range(rng *r, int32_t lo, int32_t hi) { return lo + (int32_t)below(r, (uint32_t)(hi - lo + 1)); }

static uint64_t fnv(const char *s)
{
   uint64_t h = UINT64_C(0xCBF29CE484222325);
   while (*s)
      h = (h ^ (unsigned char)*s++) * UINT64_C(0x100000001B3);
   return h;
}

static x87 X(int sign, int32_t exp, uint64_t m) { return x87_make((uint16_t)((sign << 15) | exp), m); }

/* NOTE: the order in which function arguments or operands of arithmetic
 * operators are evaluated is unspecified in C, so every random draw below is
 * its own statement; otherwise compilers generate different operands and
 * the reference vectors do not transfer between hosts. */
static int rsign(rng *r) { return (int)below(r, 2); }
static uint64_t mant(rng *r);
static uint64_t short_mant(rng *r, int k);

/* random sign, exponent in [elo, ehi], significand from mant() (k == 0),
 * short_mant(k) (k > 0) or short_mant(random 1..64) (k < 0) */
static x87 rand_x(rng *r, int32_t elo, int32_t ehi, int k)
{
   int s = rsign(r);
   int32_t e = range(r, elo, ehi);
   uint64_t m;
   if (k < 0)
      k = range(r, 1, 64);
   m = k ? short_mant(r, k) : mant(r);
   return X(s, e, m);
}

/* A significand with bit 63 set; half the time the bits below a random
 * position are forced to 0, 1..1, 10..0, 01..1 or 0..01 (rounding ties and
 * their neighbours at every rounding position). */
static uint64_t mant(rng *r)
{
   uint64_t m = next(r) | J_BIT;
   if (below(r, 2)) {
      int p = (int)below(r, 64); /* bits 0..p-1 */
      uint64_t mask = p ? (((uint64_t)1 << p) - 1) : 0;
      uint64_t pat[5];
      pat[0] = 0;
      pat[1] = mask;
      pat[2] = p ? (uint64_t)1 << (p - 1) : 0;
      pat[3] = p ? mask >> 1 : 0;
      pat[4] = p ? 1 : 0;
      m = (m & ~mask) | pat[below(r, 5)];
   }
   return m;
}

/* A significand with exactly k significant bits (1 <= k <= 64). */
static uint64_t short_mant(rng *r, int k)
{
   uint64_t m;
   if (k <= 1)
      return J_BIT;
   m = J_BIT | ((next(r) >> 1) & ~(((uint64_t)1 << (64 - k)) - 1));
   return m | ((uint64_t)1 << (64 - k));
}

static int32_t rand_int(rng *r)
{
   static const int32_t sp[] = { 0, 1, -1, 2, -2, INT32_MAX, INT32_MIN, INT32_MAX - 1, INT32_MIN + 1 };
   int bits;
   uint32_t v;
   if (below(r, 8) == 0)
      return sp[below(r, sizeof sp / sizeof sp[0])];
   bits = (int)below(r, 32);
   v = (uint32_t)(next(r) >> (64 - bits - 1)) | 0; /* up to bits+1 bits */
   v &= bits >= 31 ? 0x7FFFFFFFu : ((1u << (bits + 1)) - 1);
   return below(r, 2) ? -(int32_t)v : (int32_t)v;
}

static int64_t rand_int64(rng *r)
{
   int bits = (int)below(r, 64);
   uint64_t v = next(r) >> (63 - bits);
   if (below(r, 16) == 0)
      return below(r, 2) ? INT64_MIN : INT64_MAX;
   return below(r, 2) ? (int64_t)(0 - v) : (int64_t)v;
}

/* Random double (as x87): mostly moderate exponents, some full-range,
 * subnormal and zero. */
static x87 rand_dbl(rng *r)
{
   uint64_t sign = (uint64_t)below(r, 2) << 63, f = next(r) >> 12;
   uint32_t e, sel = below(r, 20);
   if (sel < 14)
      e = (uint32_t)range(r, 1023 - 64, 1023 + 64);
   else if (sel < 17)
      e = (uint32_t)range(r, 1, 2046);
   else if (sel < 19)
      e = 0;
   else
      e = below(r, 2) ? (uint32_t)range(r, 1, 60) : (uint32_t)range(r, 2046 - 60, 2046);
   if (below(r, 4) == 0)
      f &= ~((UINT64_C(1) << below(r, 53)) - 1); /* short fraction */
   if (e == 0 && below(r, 4) == 0)
      f = 0;
   return x87_from_f64_bits(sign | ((uint64_t)e << 52) | f);
}

static x87 rand_ext(rng *r)
{
   return rand_x(r, BIAS - 64, BIAS + 64, 0);
}

static x87 rand_special(rng *r)
{
   int s = (int)below(r, 2);
   uint64_t payload = next(r) & (UINT64_C(0x3FFFFFFFFFFFFFFF));
   switch (below(r, 6)) {
   case 0: return X(s, 0, 0);
   case 1: return X(s, 0x7FFF, J_BIT);
   case 2: return X(s, 0x7FFF, J_BIT | UINT64_C(0x4000000000000000) | payload); /* QNaN */
   case 3: return X(s, 0x7FFF, J_BIT | (payload ? payload : 1));                /* SNaN */
   case 4: return X(1, 0x7FFF, UINT64_C(0xC000000000000000));                    /* indefinite */
   default: { /* extended denormal */
      uint64_t m = next(r);
      return X(s, 0, m >> range(r, 1, 63));
   }
   }
}

/* Full exponent range, denormals, underflow/overflow neighbourhoods, specials. */
static x87 rand_wide(rng *r)
{
   switch (below(r, 10)) {
   case 0: return rand_special(r);
   case 1:
   case 2: return rand_x(r, 1, 90, 0);
   case 3:
   case 4: return rand_x(r, 0x7FFE - 90, 0x7FFE, 0);
   case 5: return rand_x(r, BIAS - 1100, BIAS + 1100, 0);
   default: return rand_x(r, 1, 0x7FFE, 0);
   }
}

/* ---- game-like values ---- */

static x87 unit01(rng *r) /* double in [0,1) with 52 random bits */
{
   return x87_sub(x87_from_f64_bits(UINT64_C(0x3FF0000000000000) | (next(r) >> 12)),
                  x87_from_i32(1));
}

static const double game_consts[] = { 0.7,  0.9,  0.3,   0.8,   0.2,   1.4294484665, 0.5, 12.0, 12.2,
                                      -0.2, 0.1,  1.0,   2.0,   3.0,   0.05,         0.95, 640.0,
                                      480.0, 1000.0, 0.25, 0.75, 1.5 };

static x87 game_const(rng *r)
{
   uint32_t n = sizeof game_consts / sizeof game_consts[0];
   uint32_t k = below(r, n + 1);
   x87 v = k == n ? x87_from_f32(65535.0f) : x87_from_f64(game_consts[k]);
   return below(r, 4) ? v : x87_neg(v);
}

static x87 game_velocity(rng *r)
{
   return x87_rnd_f64(x87_sub(x87_mul(unit01(r), x87_from_i32(60)), x87_from_i32(30)));
}

static x87 int_plus_unit(rng *r, int32_t lo, int32_t hi)
{
   x87 i = x87_from_i32(range(r, lo, hi));
   return x87_add(i, unit01(r));
}

static x87 game_double(rng *r)
{
   x87 u, v;
   switch (below(r, 8)) {
   case 0: return x87_rnd_f64(int_plus_unit(r, 0, 1000));
   case 1: return x87_rnd_f64(int_plus_unit(r, -200000, 0));
   case 2: return game_velocity(r);
   case 3: return game_const(r);
   case 4:
      u = game_velocity(r);
      v = game_const(r);
      return x87_rnd_f64(x87_mul(u, v));
   case 5: return x87_from_i32(range(r, -1000, 1000));
   case 6: return x87_rnd_f64(x87_div(x87_from_i32(range(r, -16000, 16000)), x87_from_i32(16)));
   default: return x87_rnd_f32(below(r, 2) ? game_velocity(r) : int_plus_unit(r, 0, 640));
   }
}

static x87 game_value(rng *r)
{
   x87 u, v, w;
   switch (below(r, 5)) {
   case 0:
      u = game_double(r);
      v = game_double(r);
      return x87_mul(u, v);
   case 1:
      u = game_double(r);
      v = game_double(r);
      w = game_const(r);
      return x87_add(u, x87_mul(v, w));
   default: return game_double(r);
   }
}

static int32_t game_int(rng *r, int zero_often)
{
   int32_t p, q, s, t;
   if (zero_often && below(r, 4) == 0)
      return 0;
   switch (below(r, 3)) {
   case 0: return range(r, -640, 640);
   case 1: /* 2-D cross product of screen-sized vectors */
      p = range(r, -640, 640);
      q = range(r, -480, 480);
      s = range(r, -640, 640);
      t = range(r, -480, 480);
      return p * q - s * t;
   default: return range(r, -20, 20);
   }
}

/* ---- rounding-boundary values ---- */

static x87 bump(x87 v, int t) /* move t extended ulps within the binade */
{
   uint64_t m = v.m + (uint64_t)(int64_t)t;
   if ((v.se & 0x7FFF) == 0 || (v.se & 0x7FFF) == 0x7FFF || !(m & J_BIT) ||
       (t > 0 && m < v.m) || (t < 0 && m > v.m))
      return v;
   v.m = m;
   return v;
}

/* Value on (or half-way between points of) the grid of a narrower format:
 * g_shift = significand bits dropped for normals, emin = biased extended
 * exponent of the format's smallest normal. */
static x87 near_grid(x87 v, rng *r, int g_shift, int32_t emin)
{
   int32_t e = v.se & 0x7FFF;
   int s;
   if (e == 0 || e == 0x7FFF)
      return v;
   s = g_shift + (e < emin ? emin - e : 0);
   if (s <= 63 && below(r, 3)) {
      uint64_t half = (uint64_t)1 << (s - 1);
      uint64_t m = v.m & ~(((uint64_t)1 << s) - 1);
      m = below(r, 2) ? m + half : m - half;
      if (m & J_BIT)
         v.m = m;
   }
   return bump(v, range(r, -2, 2));
}

static x87 rand_near(rng *r)
{
   switch (below(r, 5)) {
   case 0: { /* integers, integers + 1/2 */
      static const int64_t sp[] = { 0, 1, -1, INT32_MAX, (int64_t)INT32_MAX + 1, INT32_MIN,
                                    (int64_t)INT32_MIN - 1, INT64_MAX, INT64_MIN, (int64_t)1 << 32 };
      int64_t n = below(r, 3) ? (below(r, 2) ? rand_int(r) : rand_int64(r))
                              : sp[below(r, sizeof sp / sizeof sp[0])];
      x87 v = x87_from_i64(n);
      if (below(r, 3) && (n < ((int64_t)1 << 61) && n > -((int64_t)1 << 61)))
         v = x87_add(v, x87_from_f64(below(r, 2) ? 0.5 : -0.5));
      if (below(r, 8) == 0)
         v = X((int)below(r, 2), BIAS + 63, J_BIT); /* +-2^63 */
      return bump(v, range(r, -2, 2));
   }
   case 1: return near_grid(rand_dbl(r), r, 11, BIAS - 1022);
   case 2: {
      x87 v = below(r, 5) ? rand_x(r, BIAS - 126 - 30, BIAS + 127, 0)
                          : rand_x(r, BIAS - 40, BIAS + 40, 0);
      return near_grid(v, r, 40, BIAS - 126);
   }
   case 3: { /* double / float range edges */
      x87 v;
      if (below(r, 2)) {
         v = below(r, 2) ? rand_x(r, BIAS - 1022 - 60, BIAS - 1020, 0)
                         : rand_x(r, BIAS + 1021, BIAS + 1025, 0);
         return near_grid(v, r, 11, BIAS - 1022);
      }
      v = below(r, 2) ? rand_x(r, BIAS - 126 - 30, BIAS - 124, 0)
                      : rand_x(r, BIAS + 125, BIAS + 129, 0);
      return near_grid(v, r, 40, BIAS - 126);
   }
   default: return near_grid(game_value(r), r, 11, BIAS - 1022);
   }
}

static uint64_t rand_raw_f64(rng *r)
{
   uint64_t b = next(r);
   switch (below(r, 4)) {
   case 0: return b | (UINT64_C(0x7FF) << 52);                  /* inf / NaN */
   case 1: return b & ~(UINT64_C(0x7FF) << 52);                 /* zero / subnormal */
   case 2: return b & (UINT64_C(0x800FFFFFFFFFFFFF) | (below(r, 2) ? 0 : ~(uint64_t)0 >> 12));
   default: return b;
   }
}

static uint32_t rand_raw_f32(rng *r)
{
   uint32_t b = (uint32_t)next(r);
   switch (below(r, 4)) {
   case 0: return b | 0x7F800000u;
   case 1: return b & ~0x7F800000u;
   case 2: return b & (0x807FFFFFu | (below(r, 2) ? 0 : 0x7Fu));
   default: return b;
   }
}

/* ---- distributions ---- */

enum { D_DBL, D_GAME, D_EXT, D_EXPDIFF, D_CANCEL, D_TIE, D_WIDE, D_INT, D_NEAR, NDISTS };
static const char *const dist_names[NDISTS] = { "double", "game",  "ext", "expdiff", "cancel",
                                                "tie",    "wide",  "int", "near" };

static void gen_case(int dist, rng *r, kase *k)
{
   k->r = next(r);
   k->q = (uint32_t)next(r);
   k->i = rand_int(r);
   k->j = rand_int(r);
   switch (dist) {
   case D_DBL:
      k->a = rand_dbl(r);
      k->b = rand_dbl(r);
      k->c = rand_dbl(r);
      k->r = x87_to_f64_bits(k->a);
      k->q = x87_to_f32_bits(k->b);
      break;
   case D_GAME:
      k->a = game_value(r);
      k->b = game_value(r);
      k->c = game_value(r);
      k->r = x87_to_f64_bits(game_double(r));
      k->q = x87_to_f32_bits(game_value(r));
      k->i = game_int(r, 0);
      k->j = game_int(r, 1);
      break;
   case D_EXT:
      k->a = rand_ext(r);
      k->b = rand_ext(r);
      k->c = rand_ext(r);
      break;
   case D_EXPDIFF: {
      x87 a = below(r, 2) ? rand_ext(r) : rand_dbl(r);
      int32_t ea = a.se & 0x7FFF, eb = ea - range(r, 0, 200), ec;
      x87 b = rand_x(r, eb < 1 ? 1 : eb, eb < 1 ? 1 : eb, 0);
      if (below(r, 2)) {
         k->a = a;
         k->b = b;
      } else {
         k->a = b;
         k->b = a;
      }
      ec = ea - range(r, 0, 130);
      k->c = rand_x(r, ec < 1 ? 1 : ec, ec < 1 ? 1 : ec, 0);
      break;
   }
   case D_CANCEL: {
      x87 a = below(r, 2) ? rand_ext(r) : rand_dbl(r), b = a;
      int bits = (int)below(r, 64);
      uint64_t delta = bits ? next(r) >> (64 - bits) : 0;
      if ((a.se & 0x7FFF) == 0)
         a = rand_ext(r), b = a;
      b.m = below(r, 2) ? b.m + delta : b.m - delta;
      if (!(b.m & J_BIT))
         b.m = a.m ^ (delta & 0xFFFF);
      if (below(r, 4) == 0) { /* neighbouring binade */
         int32_t e = (b.se & 0x7FFF) + (below(r, 2) ? 1 : -1);
         b.se = (uint16_t)((b.se & 0x8000) | e);
         b.m = below(r, 2) ? UINT64_C(0xFFFFFFFFFFFFFFFF) - (delta & 0xFF) : J_BIT | (delta & 0xFF);
      }
      if (below(r, 2))
         b.se ^= 0x8000; /* opposite sign: add cancels; same sign: sub cancels */
      k->a = a;
      k->b = b;
      k->c = x87_neg(x87_mul(a, b));
      break;
   }
   case D_TIE: {
      static const int pcs[3] = { 24, 53, 64 };
      int pc = pcs[below(r, 3)];
      int32_t ea = range(r, BIAS - 30, BIAS + 30);
      switch (below(r, 3)) {
      case 0: { /* addition: small operand around the rounding position */
         int d = below(r, 4) ? pc + range(r, -3, 2) : range(r, 0, 3);
         k->a = rand_x(r, ea, ea, -1);
         k->b = rand_x(r, ea - d, ea - d, range(r, 1, 12));
         break;
      }
      case 1: { /* multiplication: product with about pc+1 significant bits */
         int k1 = range(r, 1, 64), k2 = pc + 1 - k1 + range(r, -1, 1);
         if (k2 < 1) k2 = 1;
         if (k2 > 64) k2 = 64;
         k->a = rand_x(r, ea, ea, k1);
         k->b = rand_x(r, BIAS - 30, BIAS + 30, k2);
         break;
      }
      default: { /* division with an exact quotient of about pc+1 bits */
         int kq = below(r, 2) ? pc + 1 : range(r, 1, 63);
         int kb = range(r, 1, kq >= 64 ? 1 : 64 - kq);
         x87 qv = rand_x(r, ea, ea, kq > 64 ? 64 : kq);
         x87 bv = rand_x(r, BIAS - 30, BIAS + 30, kb);
         k->a = x87_mul(qv, bv);
         k->b = bv;
         break;
      }
      }
      k->c = rand_x(r, ea - pc - 2, ea + 2, -1);
      break;
   }
   case D_WIDE:
      k->a = rand_wide(r);
      k->b = rand_wide(r);
      k->c = rand_wide(r);
      switch (below(r, 8)) {
      case 0: /* same magnitude (x - x, NaN pairs with equal significands) */
         k->b = k->a;
         if (below(r, 2))
            k->b.se ^= 0x8000;
         break;
      case 1:
      case 2: { /* product / quotient exponent at the underflow or overflow edge */
         int32_t ea = range(r, 1, 0x7FFE);
         int32_t t = below(r, 2) ? range(r, -70, 3) : range(r, 0x7FFE - 3, 0x7FFE + 2);
         int32_t eb = below(r, 2) ? t + BIAS - ea : ea + BIAS - t;
         if (eb >= 1 && eb <= 0x7FFE) {
            k->a = rand_x(r, ea, ea, below(r, 2) ? 0 : -1);
            k->b = rand_x(r, eb, eb, below(r, 2) ? 0 : -1);
         }
         break;
      }
      default:
         break;
      }
      k->r = rand_raw_f64(r);
      k->q = rand_raw_f32(r);
      break;
   case D_INT:
      k->a = x87_from_i32(rand_int(r));
      k->b = x87_from_i32(below(r, 8) ? rand_int(r) : 0);
      k->c = below(r, 2) ? x87_from_i32(rand_int(r)) : x87_from_i64(rand_int64(r));
      k->r = (uint64_t)rand_int64(r);
      if (below(r, 6) == 0)
         k->j = 0;
      if (below(r, 16) == 0)
         k->i = 0;
      break;
   default: /* D_NEAR */
      k->a = rand_near(r);
      k->b = rand_near(r);
      k->c = rand_near(r);
      k->r = x87_to_f64_bits(rand_near(r));
      k->r ^= below(r, 4) ? 0 : 1;
      k->q = x87_to_f32_bits(rand_near(r));
      k->q ^= below(r, 4) ? 0 : 1;
      break;
   }
}

static void case_for(int op, int dist, uint64_t seed, uint64_t idx, kase *k)
{
   rng r;
   r.s = mix64(fnv(ops[op].name) ^ mix64(fnv(dist_names[dist]) + seed)) ^ mix64(idx * 0x2545F491u + 1);
   gen_case(dist, &r, k);
}

/* ------------------------------------------------------------ driver */

static uint64_t hash_res(uint64_t h, res v)
{
   int i;
   for (i = 0; i < 8; i++)
      h = (h ^ ((v.lo >> (8 * i)) & 0xFF)) * UINT64_C(0x100000001B3);
   h = (h ^ (v.hi & 0xFF)) * UINT64_C(0x100000001B3);
   h = (h ^ (v.hi >> 8)) * UINT64_C(0x100000001B3);
   return h;
}

static const x87v_entry *find_entry(const char *op, const char *dist)
{
   int i;
   for (i = 0; i < x87v_count; i++)
      if (x87v_table[i].op && !strcmp(x87v_table[i].op, op) && !strcmp(x87v_table[i].dist, dist))
         return &x87v_table[i];
   return NULL;
}

static void print_x(const char *label, x87 v)
{
   printf(" %s=%04x:%016" PRIx64, label, v.se, v.m);
}

static void print_case(const kase *k)
{
   print_x("a", k->a);
   print_x("b", k->b);
   print_x("c", k->c);
   printf(" r=%016" PRIx64 " q=%08" PRIx32 " i=%" PRId32 " j=%" PRId32 "\n", k->r, k->q, k->i, k->j);
}

static int res_eq(res a, res b) { return a.lo == b.lo && a.hi == b.hi; }

int main(int argc, char **argv)
{
   long cases = X87V_CASES;
   uint64_t seed = 0;
   int native = HAVE_NATIVE, use_vectors = 1, printed = 0;
   const char *only = NULL, *gen = NULL;
   long long total = 0, total_native = 0, failures = 0, vec_checked = 0;
   int op, dist, i;
   clock_t t0 = clock();
   FILE *out = NULL;
   static long op_fail[64];

   for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "--cases") && i + 1 < argc)
         cases = atol(argv[++i]);
      else if (!strcmp(argv[i], "--seed") && i + 1 < argc)
         seed = strtoull(argv[++i], NULL, 0), use_vectors = seed == 0;
      else if (!strcmp(argv[i], "--no-native"))
         native = 0;
      else if (!strcmp(argv[i], "--only") && i + 1 < argc)
         only = argv[++i];
      else if (!strcmp(argv[i], "--gen-vectors") && i + 1 < argc)
         gen = argv[++i], use_vectors = 0;
      else {
         fprintf(stderr, "usage: %s [--cases N] [--seed S] [--no-native] [--only NAME] "
                         "[--gen-vectors FILE]\n", argv[0]);
         return 2;
      }
   }
   if (gen && (!native || seed || only || cases < X87V_CASES)) {
      fprintf(stderr, "--gen-vectors needs native x87, the default seed, all ops and "
                      ">= %d cases\n", X87V_CASES);
      return 2;
   }
   if (!native && !use_vectors && !gen) {
      fprintf(stderr, "a custom seed without native x87 verifies nothing\n");
      return 2;
   }
   if (!native && x87v_count == 0) {
      fprintf(stderr, "no native x87 and no reference vectors: cannot verify\n");
      return 1;
   }
   if (gen) {
      out = fopen(gen, "wb");
      if (!out) {
         perror(gen);
         return 2;
      }
      fprintf(out, "/* Generated by test_x87 --gen-vectors on native x87 hardware (control word\n"
                   " * 0x037F/0x027F/0x007F).  Per op x distribution block: FNV-1a hash of the\n"
                   " * first X87V_CASES results and the first %d results.  Do not edit. */\n"
                   "#define X87V_CASES %d\n"
                   "static const x87v_entry x87v_table[] = {\n", KAT, X87V_CASES);
   }
#if HAVE_NATIVE
   hw_set_cw(0x037F);
#endif
   printf("x87 emulator verification: %s, %ld cases per block%s\n",
          native ? "native x87 comparison" : "reference vectors only", cases,
          use_vectors ? "" : " (custom seed: vector checks off)");

   for (op = 0; op < NOPS; op++) {
      if (only && !strstr(ops[op].name, only))
         continue;
#if HAVE_NATIVE
      if (native)
         hw_set_cw(cw_for_pc(ops[op].pc));
#endif
      for (dist = 0; dist < NDISTS; dist++) {
         uint64_t h = UINT64_C(0xCBF29CE484222325);
         res kat[KAT];
         const x87v_entry *ent = use_vectors ? find_entry(ops[op].name, dist_names[dist]) : NULL;
         long n;
         for (n = 0; n < cases; n++) {
            kase k;
            res e;
            case_for(op, dist, seed, (uint64_t)n, &k);
            e = emu_eval(&ops[op], &k);
            if (n < X87V_CASES)
               h = hash_res(h, e);
            if (n < KAT) {
               kat[n] = e;
               if (ent) {
                  res want;
                  want.lo = ent->kat_lo[n];
                  want.hi = ent->kat_hi[n];
                  vec_checked++;
                  if (!res_eq(e, want)) {
                     failures++;
                     op_fail[op]++;
                     if (printed++ < 20) {
                        printf("KAT MISMATCH %s/%s #%ld: emu %04x:%016" PRIx64
                               " want %04x:%016" PRIx64 "\n  ", ops[op].name, dist_names[dist], n,
                               e.hi, e.lo, want.hi, want.lo);
                        print_case(&k);
                     }
                  }
               }
            }
#if HAVE_NATIVE
            if (native) {
               res w = hw_eval(&ops[op], &k);
               total_native++;
               if (!res_eq(e, w)) {
                  failures++;
                  op_fail[op]++;
                  if (printed++ < 20) {
                     printf("MISMATCH %s/%s #%ld: emu %04x:%016" PRIx64 " x87 %04x:%016" PRIx64
                            "\n  ", ops[op].name, dist_names[dist], n, e.hi, e.lo, w.hi, w.lo);
                     print_case(&k);
                  }
               }
            }
#endif
         }
         total += cases;
         if (cases >= X87V_CASES) {
            if (ent) {
               vec_checked++;
               if (ent->hash != h) {
                  failures++;
                  op_fail[op]++;
                  if (printed++ < 20)
                     printf("HASH MISMATCH %s/%s: %016" PRIx64 " want %016" PRIx64 "%s\n",
                            ops[op].name, dist_names[dist], h, ent->hash,
                            native ? " (vector file stale? regenerate)" : "");
               }
            } else if (use_vectors && !gen) {
               if (!native) {
                  failures++;
                  op_fail[op]++;
               }
               if (printed++ < 20)
                  printf("no reference vector for %s/%s\n", ops[op].name, dist_names[dist]);
            }
            if (out) {
               fprintf(out, "   { \"%s\", \"%s\", UINT64_C(0x%016" PRIx64 "),\n     {", ops[op].name,
                       dist_names[dist], h);
               for (i = 0; i < KAT; i++)
                  fprintf(out, " UINT64_C(0x%016" PRIx64 ")%s", kat[i].lo, i + 1 < KAT ? "," : "");
               fprintf(out, " },\n     {");
               for (i = 0; i < KAT; i++)
                  fprintf(out, " 0x%04x%s", kat[i].hi, i + 1 < KAT ? "," : "");
               fprintf(out, " } },\n");
            }
         }
      }
   }
#if HAVE_NATIVE
   hw_set_cw(0x037F);
#endif

   printf("\n%-18s %12s %9s\n", "operation", "cases", "failures");
   for (op = 0; op < NOPS; op++) {
      if (only && !strstr(ops[op].name, only))
         continue;
      printf("%-18s %12lld %9ld\n", ops[op].name, (long long)cases * NDISTS, op_fail[op]);
   }
   printf("\ntotal cases %lld (native x87 comparisons %lld), vector checks %lld, failures %lld, "
          "%.1f s\n", total, total_native, vec_checked, failures,
          (double)(clock() - t0) / CLOCKS_PER_SEC);
   if (out) {
      fprintf(out, "};\nstatic const int x87v_count = (int)(sizeof x87v_table / sizeof x87v_table[0]);\n");
      fclose(out);
      if (failures) {
         remove(gen);
         fprintf(stderr, "failures: %s not written\n", gen);
      }
   }
   printf(failures ? "FAIL\n" : "OK\n");
   return failures ? 1 : 0;
}
