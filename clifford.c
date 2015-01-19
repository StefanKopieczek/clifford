#include "clifford.h"
#include "cliff_list.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define LOG_256                         5.54517744448
#define LOG_10                          2.30258509299
#define LONG_MULT_THRESHOLD_DIGITS      40

static BIG_INT *cliff_sub_cmn(BIG_INT *a, BIG_INT *b, short to_overwrite);
static CLIFF_LIST *get_ints_to_tidy();
static void mark_for_tidyup(BIG_INT *n);
static void init_big_int(BIG_INT *n);
static BIG_INT *new_big_int();
static void set_length(int length, BIG_INT *n);
static void tidy_ints();
static int int_max(int a, int b);

void cliff_init()
{
    // Nothing to do, but let's keep this function part of the API in case we
    // end up needing it later!
}

void cliff_teardown()
{
    tidy_ints();
    free_cliff_list(get_ints_to_tidy());
}

BIG_INT *cliff(long l)
{
    BIG_INT *n = new_big_int();

    if (l < 0)
    {
        n->sgn = -1;
        l = -l;
    }


    set_length((int)ceil(log((double)l) / LOG_256), n);

    char *p = n->val;
    while (l > 0)
    {
        *p = (char)l;
        l >>= 8;
        p++;
    }

    return n;
}

BIG_INT *cliff_retain(BIG_INT *n)
{
    cliff_list_remove(get_ints_to_tidy(), n);
    tidy_ints();
    return n;
}

void cliff_release(BIG_INT *n)
{
   free(n->val);
   free(n);
}

void cliff_discard(BIG_INT *n)
{
    // 'n' will be in the tidy list by default.
    // To discard it, we need only kick off the GC, which we would have had to
    // do anyway to remove any calculation by-products.
    tidy_ints();
}

BIG_INT *cliff_clone(BIG_INT *n)
{
    BIG_INT *copy = new_big_int();
    copy->length = n->length;
    copy->sgn = n->sgn;
    copy->val = malloc(n->length);
    memcpy(copy->val, n->val, n->length);
    return copy;
}

BIG_INT *cliff_add(BIG_INT *a, BIG_INT *b)
{
    return cliff_add1(cliff_clone(a), b);
}

BIG_INT *cliff_add1(BIG_INT *a, BIG_INT *b)
{
    if (a->sgn != b->sgn)
    {
        // a and b have different signs, so this is better understood as a
        // subtraction.
        BIG_INT *result;
        if (b->sgn == -1)
        {
            // a + b == a - |b|
            b->sgn = 1;
            result = cliff_sub1(a, b);
            b->sgn = -1;
        }
        else
        {
            // a + b == b - |a|
            a->sgn = 1;
            result = cliff_sub2(b, a);
        }
        return result;
    }

    set_length(int_max(a->length, b->length), a);
    int idx;
    int carry = 0;
    for (idx = 0; idx < a->length; idx++)
    {
        unsigned char bval;
        if (idx < b->length)
        {
            bval = b->val[idx];
        }
        else
        {
            bval = 0;
        }

        int digit_sum = carry + a->val[idx] + bval;
        if (digit_sum > 255)
        {
            carry = 1;
            digit_sum -= 256;
        }
        else
        {
            carry = 0;
        }

        a->val[idx] = digit_sum;
    }

    if (carry == 1)
    {
        set_length(a->length + 1, a);
        a->val[idx + 1] = 1;
    }

    return a;
}

BIG_INT *cliff_add_int1(BIG_INT *a, int b)
{
    return cliff_add1(a, cliff(b));
}

BIG_INT *cliff_sub(BIG_INT *a, BIG_INT *b)
{
    return cliff_sub1(cliff_clone(a), b);
}

BIG_INT *cliff_sub1(BIG_INT *a, BIG_INT *b)
{
    if (b->sgn == -1)
    {
        b->sgn = 1;
        BIG_INT *result = cliff_add1(a, b);
        b->sgn = -1;
        return result;
    }

    if (a->sgn == -1)
    {
        a->sgn = 1;
        BIG_INT *result = cliff_add1(a, b);
        result->sgn = -1;
        return result;
    }

    if (cliff_lt(a, b))
    {
        BIG_INT *result = cliff_sub2(b, a);
        result->sgn *= -1;
        return result;
    }

    // We now know a >= b >= 0, so we can apply cliff_sub_cmn.
    return cliff_sub_cmn(a, b, 0);
}

// Subtract b from a. Requires a >= b >= 0.
// If to_overwrite==0, then the value of a will be overwritten.
// If to_overwrite==1, then the value of b will be overwritten.
BIG_INT *cliff_sub_cmn(BIG_INT *a, BIG_INT *b, short to_overwrite)
{
    assert(to_overwrite == 0 || to_overwrite == 1);
    BIG_INT *output = (to_overwrite == 1) ? b : a;

    short carry = 0;
    int idx = 0;
    if (to_overwrite == 1)
    {
        // As a >= b >= 0, max(a->length, b->length) == a->length.
        set_length(a->length, b);
    }

    for (idx = 0; idx < b->length; idx++)
    {
        // Use ints for this step to avoid overflow.
        int subtract = b->val[idx] + carry;
        if (a->val[idx] >= subtract)
        {
            output->val[idx] = a->val[idx] - subtract;
            carry = 0;
        }
        else
        {
            // This is guaranteed not to overflow: (a - subtract) is negative,
            // so the result below must be < 256.
            output->val[idx] = (256 - subtract) + a->val[idx];
            carry = 1;
        }

    }

    if (carry == 1)
    {
        output->val[idx] = a->val[idx] - 1;
    }

    return output;
}

BIG_INT *cliff_sub2(BIG_INT *a, BIG_INT *b)
{
    if (b->sgn == -1)
    {
        b->sgn == 1;
        return cliff_add1(b, a);
    }

    if (a->sgn == -1)
    {
        a->sgn = 1;
        BIG_INT *result = cliff_add1(b, a);
        a->sgn = -1;
        result->sgn *= -1;
        return result;
    }

    if (cliff_lt(a, b))
    {
        BIG_INT *result = cliff_sub1(b, a);
        result->sgn *= -1;
        return result;
    }

    // We now know a >= b >= 0, so we can apply cliff_sub_cmn.
    return cliff_sub_cmn(a, b, 1);
}

BIG_INT *cliff_sub_int(BIG_INT *a, int b)
{
    return cliff_sub2(a, cliff(b));
}

BIG_INT *cliff_int_sub(int a, BIG_INT *b)
{
    return cliff_sub1(cliff(a), b);
}

BIG_INT *cliff_sub_int1(BIG_INT *a, int b)
{
    return cliff_sub1(a, cliff(b));
}

BIG_INT *cliff_int_sub2(int a, BIG_INT *b)
{
    return cliff_sub2(cliff(a), b);
}

BIG_INT *cliff_mult(BIG_INT *a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_mult1(BIG_INT *a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_mult_int(BIG_INT *a, int b)
{
    // TODO
}

BIG_INT *cliff_mult_int1(BIG_INT *a, int b)
{
    // TODO
}

BIG_INT *cliff_div(BIG_INT *a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_div1(BIG_INT *a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_div2(BIG_INT *a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_div_int(BIG_INT *a, int b)
{
    // TODO
}

BIG_INT *cliff_div_int1(BIG_INT *a, int b)
{
    // TODO
}

BIG_INT *cliff_int_div(int a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_int_div2(int a, BIG_INT *b)
{
    // TODO
}

BIG_INT *cliff_pow(BIG_INT *base, BIG_INT *exp)
{
    // TODO
}

BIG_INT *cliff_pow1(BIG_INT *base, BIG_INT *exp)
{
    // TODO
}

BIG_INT *cliff_pow2(BIG_INT *base, BIG_INT *exp)
{
    // TODO
}

BIG_INT *cliff_pow_int(BIG_INT *base, int exp)
{
    // TODO
}

BIG_INT *cliff_pow_int1(BIG_INT *base, int exp)
{
    // TODO
}

BIG_INT *cliff_int_pow(int base, BIG_INT *exp)
{
    // TODO
}

BIG_INT *cliff_int_pow2(int base, BIG_INT *exp)
{
    // TODO
}

BIG_INT *cliff_abs(BIG_INT *n)
{
    return cliff_abs1(cliff_clone(n));
}

BIG_INT *cliff_abs1(BIG_INT *n)
{
    n->sgn = 1;
    return n;
}

short cliff_sgn(BIG_INT *n)
{
    return n->sgn;
}


static short cliff_lt_cmn(BIG_INT *a, BIG_INT *b)
{
    short lt = -1;
    short eq = 0;
    short gt = 1;

    if (a->sgn < b->sgn)
    {
        return lt;
    }
    else if (b->sgn < a->sgn)
    {
        return gt;
    }
    else
    {
        short is_positive = (a->sgn == 1);
        if (a->length < b->length)
        {
            return is_positive * lt;
        }
        else if (b->length < a->length)
        {
            return is_positive * gt;
        }
        else
        {
            int idx;
            for (idx = a->length - 1; idx >= 0; idx--)
            {
                if (a->val[idx] < b->val[idx])
                {
                    return is_positive * lt;
                }
                else if (b->val[idx] < a->val[idx])
                {
                    return is_positive * gt;
                }
            }

            // They are equal.
            return eq;
        }
    }
}

short cliff_lt(BIG_INT *a, BIG_INT *b)
{
    return cliff_lt_cmn(a, b) < 0;
}

short cliff_lte(BIG_INT *a, BIG_INT *b)
{
    return cliff_lt_cmn(a, b) <= 0;
}

short cliff_gt(BIG_INT *a, BIG_INT *b)
{
    return cliff_lt_cmn(a, b) > 0;
}

short cliff_gte(BIG_INT *a, BIG_INT *b)
{
    return cliff_lt_cmn(a, b) >= 0;
}

char *cliff_to_string(BIG_INT *n)
{
    // Exercise to the reader; the length of the output string is bounded by
    // the ceiling of (klog10(k) + 3k + 3) where k is the size of n in base256.
    int k = n->length;
    int length = (int)(ceil(k * log(k) / LOG_10 + 3 * k + 3));
    short has_sign = (n->sgn == -1);
    char *result = calloc(length + 1 + has_sign, sizeof(char));
    char *output_digits = result + has_sign;
    if (has_sign)
    {
        result[0] = '-';
    }

    // Take a copy of the contents of n, as we will repeatedly divide it by 10
    // to perform the base conversion, but don't want to alter the original.
    unsigned char *current = malloc(n->length);
    memcpy(current, n->val, n->length);

    int base10_idx = 0;
    int digits_remaining = 1;
    while (digits_remaining)
    {
        digits_remaining = 0;
        int remainder = 0;
        int base256_idx;
        for (base256_idx = n->length - 1; base256_idx >= 0; base256_idx--)
        {
            int divisand = 256 * remainder + current[base256_idx];
            current[base256_idx] = divisand / 10;
            remainder = divisand % 10;

            if (current[base256_idx] != 0)
            {
                digits_remaining = 1;
            }
        }

        output_digits[base10_idx] = (char)(remainder + 0x30);
        base10_idx++;
    }

    free(current);

    // We've calculated the base10 output string, but it's reversed.
    // Put the digits in the correct order for output.
    int output_length = strlen(output_digits);
    for (base10_idx = 0; base10_idx < output_length / 2; base10_idx++)
    {
        int swap_idx = output_length - base10_idx - 1;
        char temp = output_digits[base10_idx];
        output_digits[base10_idx] = output_digits[swap_idx];
        output_digits[swap_idx] = temp;
    }

    // Remember to free the result when you're done with it!
    return result;
}

static CLIFF_LIST *get_ints_to_tidy()
{
    static CLIFF_LIST *ints_to_tidy;
    if (ints_to_tidy == 0)
    {
        ints_to_tidy = new_cliff_list();
    }

    return ints_to_tidy;
}

static void mark_for_tidyup(BIG_INT *n)
{
    cliff_list_add(get_ints_to_tidy(), n);
}

static void init_big_int(BIG_INT *n)
{
    mark_for_tidyup(n);
    n->length = 1;
    n->val = calloc(1, sizeof(char));
    n->sgn = 1;
}

static BIG_INT *new_big_int()
{
    BIG_INT *n = malloc(sizeof(BIG_INT));
    init_big_int(n);
    return n;
}

static void set_length(int length, BIG_INT *n)
{
    int old_length = n->length;
    n->length = length;
    n->val = realloc(n->val, length);

    if (length > old_length)
    {
        // We've grown our BIG_INT, so we need to zero out the new digits.
        memset(n->val + old_length, 0, length - old_length);
    }
}

static void tidy_ints()
{
    CLIFF_LIST *ints_to_tidy = get_ints_to_tidy();
    CLIFF_LIST_NODE *current = ints_to_tidy->start;
    CLIFF_LIST_NODE **prev_ptr = &(ints_to_tidy->start);
    while (current != 0)
    {
        cliff_release((BIG_INT *)(current->data));
        *prev_ptr = current->next;
        free(current);
        current = *prev_ptr;
    }
}

static int int_max(int a, int b)
{
    if (a >= b)
    {
        return a;
    }
    else
    {
        return b;
    }
}
