#ifndef _H_CLIFFORD
#define _H_CLIFFORD

typedef struct big_int
{
    /* Numerical data, stored in base 256, least-significant part first. */
    unsigned char *val;

    /* Number of base 256 chunks the number is composed of. */
    int length;

    /* Sign flag, either 1 or -1. */
    short sgn;
} BIG_INT;

void cliff_init();
void cliff_teardown();

BIG_INT *cliff_retain(BIG_INT *n);
void cliff_discard(BIG_INT *n);
void cliff_release(BIG_INT *n);
char *cliff_to_string(BIG_INT *n);

BIG_INT *cliff(long l);
BIG_INT *cliff_from_string(char *c);
BIG_INT *cliff_clone(BIG_INT *n);

BIG_INT *cliff_add(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_add1(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_add_int(BIG_INT *a, int i);
BIG_INT *cliff_add_int1(BIG_INT *a, int i);

BIG_INT *cliff_sub(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_sub1(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_sub2(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_sub_int(BIG_INT *a, int b);
BIG_INT *cliff_int_sub(int a, BIG_INT *b);
BIG_INT *cliff_sub_int1(BIG_INT *a, int b);
BIG_INT *cliff_int_sub2(int a, BIG_INT *b);

BIG_INT *cliff_mult(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_mult1(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_mult_int(BIG_INT *a, int b);
BIG_INT *cliff_mult_int1(BIG_INT *a, int b);

BIG_INT *cliff_div(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_div1(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_div2(BIG_INT *a, BIG_INT *b);
BIG_INT *cliff_div_int(BIG_INT *a, int b);
BIG_INT *cliff_div_int1(BIG_INT *a, int b);
BIG_INT *cliff_int_div(int a, BIG_INT *b);
BIG_INT *cliff_int_div2(int a, BIG_INT *b);

BIG_INT *cliff_pow(BIG_INT *base, BIG_INT *exp);
BIG_INT *cliff_pow1(BIG_INT *base, BIG_INT *exp);
BIG_INT *cliff_pow2(BIG_INT *base, BIG_INT *exp);
BIG_INT *cliff_pow_int(BIG_INT *base, int exp);
BIG_INT *cliff_pow_int1(BIG_INT *base, int exp);
BIG_INT *cliff_int_pow(int base, BIG_INT *exp);
BIG_INT *cliff_int_pow2(int base, BIG_INT *exp);


BIG_INT *cliff_abs(BIG_INT *n);
BIG_INT *cliff_abs1(BIG_INT *n);

short cliff_sgn(BIG_INT *n);

short cliff_lt(BIG_INT *a, BIG_INT *b);
short cliff_lte(BIG_INT *a, BIG_INT *b);
short cliff_gt(BIG_INT *a, BIG_INT *b);
short cliff_gte(BIG_INT *a, BIG_INT *b);

#endif
