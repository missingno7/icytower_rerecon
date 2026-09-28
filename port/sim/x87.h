/*
 * Exact software emulation of x87 80-bit extended-precision arithmetic.
 *
 * Icy Tower 1.5.1 was built with GCC 4.4 for i386 using the x87 FPU with
 * control word 0x037F (the MinGW CRT runs fninit): 64-bit significand
 * precision (PC=11), round-to-nearest-even, every exception masked.  The
 * gameplay code keeps intermediates in 80-bit registers and rounds to
 * double/float only at stores, and replays store only inputs, so the port
 * expresses each gameplay floating-point operation through this module to get
 * bit-identical results on SSE2/ARM/any other CPU.  See docs/port/X87.md.
 *
 * Every function is pure, uses only 64-bit integer arithmetic, and never
 * touches the host FPU (no long double, no x87 instructions; float/double are
 * only reinterpreted bit-for-bit through memcpy).
 *
 * Arithmetic rounds to nearest-even at the chosen precision control with the
 * full 15-bit exponent range; masked-exception results are produced exactly
 * as the hardware does: overflow -> signed infinity, underflow -> rounded
 * extended denormal, x/0 -> signed infinity, invalid operations (0/0, inf/inf,
 * 0*inf, inf-inf, unsupported encodings) -> the real indefinite QNaN
 * (sign 1, exponent 0x7FFF, significand 0xC000000000000000), NaN operands
 * propagate quieted using the x87 rule (a QNaN beats an SNaN, otherwise the
 * larger significand wins).
 *
 * Mixed-operand instructions need no separate entry points because loading
 * is exact: `fadd dword [m]` is x87_add(st, x87_from_f32(m)), `fidivr [m]`
 * is x87_div(x87_from_i32(m), st), `fsubr` swaps the operands of x87_sub.
 */
#ifndef PORT_SIM_X87_H
#define PORT_SIM_X87_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The exact 80-bit register image: se = sign (bit 15) | biased exponent
 * (bits 14..0, bias 16383); m = 64-bit significand with the explicit integer
 * bit at bit 63.  Values produced by this module are always canonical
 * (normals have bit 63 set; exponent 0 means zero or denormal). */
typedef struct x87 {
   uint64_t m;
   uint16_t se;
} x87;

/* Precision control (the PC field of the control word). */
enum { X87_PC24 = 24, X87_PC53 = 53, X87_PC64 = 64 };

/* x87_cmp results. */
enum { X87_LT = -1, X87_EQ = 0, X87_GT = 1, X87_UNORDERED = 2 };

/* x87_fucom condition bits (C3<<2 | C2<<1 | C0). */
enum { X87_CC_C0 = 1, X87_CC_C2 = 2, X87_CC_C3 = 4 };

/* ---- construction ---------------------------------------------------- */
x87 x87_make(uint16_t se, uint64_t m);        /* raw register image */
x87 x87_from_i32(int32_t v);                  /* fild dword  (exact) */
x87 x87_from_i64(int64_t v);                  /* fild qword  (exact) */
x87 x87_from_f64(double v);                   /* fld qword   (exact) */
x87 x87_from_f32(float v);                    /* fld dword   (exact) */
x87 x87_from_f64_bits(uint64_t bits);         /* fld qword from IEEE bits */
x87 x87_from_f32_bits(uint32_t bits);         /* fld dword from IEEE bits */

/* ---- arithmetic, PC=64 (the game's control word 0x037F) --------------- */
x87 x87_add(x87 a, x87 b);                    /* a + b  (fadd)          */
x87 x87_sub(x87 a, x87 b);                    /* a - b  (fsub; fsubr = x87_sub(b, a)) */
x87 x87_mul(x87 a, x87 b);                    /* a * b  (fmul)          */
x87 x87_div(x87 a, x87 b);                    /* a / b  (fdiv; fdivr = x87_div(b, a)) */

/* ---- arithmetic at an explicit precision control (24, 53 or 64) ------- */
x87 x87_add_pc(x87 a, x87 b, int pc);
x87 x87_sub_pc(x87 a, x87 b, int pc);
x87 x87_mul_pc(x87 a, x87 b, int pc);
x87 x87_div_pc(x87 a, x87 b, int pc);

x87 x87_neg(x87 a);                           /* fchs (sign flip, also of NaN) */
x87 x87_abs(x87 a);                           /* fabs (sign clear, also of NaN) */

/* ---- stores (round-to-nearest-even, independent of PC) ---------------- */
double   x87_to_f64(x87 a);                   /* fst qword */
float    x87_to_f32(x87 a);                   /* fst dword */
uint64_t x87_to_f64_bits(x87 a);
uint32_t x87_to_f32_bits(x87 a);
x87      x87_rnd_f64(x87 a);                  /* fstp qword [m]; fld qword [m] */
x87      x87_rnd_f32(x87 a);                  /* fstp dword [m]; fld dword [m] */

/* fistp with RC=chop, as emitted for C (int)/(long long) casts, and with
 * RC=nearest-even (plain fistp under 0x037F).  NaN, infinities and values
 * whose rounded result does not fit return the integer indefinite
 * (INT32_MIN / INT64_MIN). */
int32_t x87_to_i32_trunc(x87 a);
int64_t x87_to_i64_trunc(x87 a);
int32_t x87_to_i32_rn(x87 a);
int64_t x87_to_i64_rn(x87 a);

/* ---- comparison ------------------------------------------------------- */
/* X87_LT, X87_EQ, X87_GT or X87_UNORDERED (either operand NaN or an
 * unsupported encoding).  +0 and -0 compare equal. */
int  x87_cmp(x87 a, x87 b);
/* Condition bits of `fucom st1` with a in st0 and b in st1:
 * a > b -> 0, a < b -> C0 (1), a == b -> C3 (4), unordered -> C3|C2|C0 (7). */
int  x87_fucom(x87 st0, x87 st1);
/* The same result as it appears in the FPU status word (bits 8, 10, 14), so
 * `fnstsw %ax; test $0x45,%ah` is ((x87_fucom_sw(a, b) >> 8) & 0x45). */
uint16_t x87_fucom_sw(x87 st0, x87 st1);
/* C relational operators: false when unordered (x87_ne is then true). */
bool x87_lt(x87 a, x87 b);
bool x87_le(x87 a, x87 b);
bool x87_gt(x87 a, x87 b);
bool x87_ge(x87 a, x87 b);
bool x87_eq(x87 a, x87 b);
bool x87_ne(x87 a, x87 b);

bool x87_is_nan(x87 a);
bool x87_identical(x87 a, x87 b);             /* bitwise equality */

#ifdef __cplusplus
}
#endif

#endif /* PORT_SIM_X87_H */
