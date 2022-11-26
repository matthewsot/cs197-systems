#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "satsolver.h"

Bignum createBignum() {
    Bignum num = {NULL, 0, 0};
    return num;
}

int print_binary(Bignum *num) {
    unsigned saw_first_one = 0;
    for (int i = num->n_words; i --> 0;) {
        for (int j = 64; j --> 0;) {
            if (num->words[i] >> j || saw_first_one) {
                printf("%lu", (num->words[i] >> j) & 1);
                saw_first_one = 1;
            }
        }
    }
    if (!saw_first_one) printf("0");
    printf("\n");
}

int print_hex(Bignum *num) {
    for (int i = num->n_words; i --> 0;) {
        printf("%lX", num->words[i]);
    }
    printf("\n");
}

int isLessThanPower(unsigned power, Bignum *num) {
    if (num->sign) return 1; // negative always less than a power
    unsigned word_idx = power / 64;
    uint64_t bit = ((uint64_t)1 << (uint64_t)(power % 64));
    if (word_idx >= num->n_words) return 1; // num can't be greater because it has less bits
    // num can't be less because contains at a positive bit at least at index corresponding to power
    if (num->words[word_idx] >= bit) return 0; 
    // less than bit at its most significant word, so check more words for a bit to see if greater
    for (uint64_t i = word_idx + 1; i < num->n_words; i++) {
        if (num->words[i] > 0) return 0;  // finds a positive bit that is greater than power
    }
    // no bits greater than power, so num must be less than power
    return 1;
}

int add_pow_2(unsigned power, Bignum *num) {
    // adds a power of two to a bignum
    unsigned word_idx = power / 64;
    if ((word_idx + 1) > num->n_words) {  // possibly going to need a bunch more words
        uint64_t old_n_words = num->n_words;
        num->n_words = 2 * (word_idx + 1);
        num->words = realloc(num->words, num->n_words * sizeof(num->words[0]));
        memset(num->words + old_n_words, num->sign, (num->n_words - old_n_words) * sizeof(num->words[0]));
    } else if (num->n_words && num->words[num->n_words - 1]) { // will need 1 more word to handle overflow
        num->words = realloc(num->words, (++num->n_words) * sizeof(num->words[0]));
        num->words[num->n_words - 1] = -1 * num->sign;
    }
    // https://stackoverflow.com/questions/46701364/how-to-access-the-carry-flag-while-adding-two-64-bit-numbers-using-asm-in-c
    uint64_t bit = ((uint64_t)1 << (uint64_t)(power % 64));
    num->words[word_idx] = bit + num->words[word_idx];
    // GCC does seem to do a fine job of optimizing this into a jb:
    // https://godbolt.org/z/6Tja8xfs6
    unsigned carry = num->words[word_idx] < bit;
    while (carry)
        carry = !(++(num->words[++word_idx]));
    // If we were once negative, but ended up carrying all the way through (i.e. we are now positive)
    if (num->sign && num->words[num->n_words - 1] == 1) {
        num->n_words--;
        num->words = realloc(num->words, num->n_words * sizeof(num->words[0])); 
        num->sign = 0; 
    }
}

int pos_big_sub_small_pow_2(unsigned power, Bignum *num) {
    // subtracts a power of smaller power of 2 from a positive bignum
    unsigned word_idx = power / 64;
    uint64_t bit = ((uint64_t)1 << (uint64_t)(power % 64));
     if (num->words[word_idx] < bit) {
        for (uint64_t i = word_idx + 1; i < num->n_words && num->words[i] > 0; i++) {
            num->words[i] -= 1;
            break;
        }
        num->words[word_idx] = (-1) << power;
     } else {
      num->words[word_idx] -= bit;
     }
}

int pos_big_sub_big_pow_2(unsigned power, Bignum *num) {
   // at most significant word
    unsigned word_idx = power / 64;
    uint64_t bits = (-1 << (uint64_t)(power % 64));
    num->words[word_idx] = num->words[word_idx] | bits;
    while (word_idx++ < num->n_words) num->words[word_idx] = -1;
    num->sign = 1;
}

int flip_sign(Bignum *num) {
    int original_sign = num->sign;
    if (original_sign) { // convert negative to positive
        //printf("Converting negative to positive\n Before doing anything:\n");
        //print_binary(num);
        num->sign = 0;
        //printf("changed sign to pos, now going to subtract 1. result: \n");
        pos_big_sub_small_pow_2(0, num); // 
        //print_binary(num);
        //printf("Negating.  Result: \n");
    }
    for (uint64_t i = 0; i < num->n_words; i++) {
        num->words[i] = ~(num->words[i]);
        //printf("Printing after flipping each word. This is flip %lu\n", i);
      
    }
      //if (original_sign) print_binary(num);
    if (!original_sign) { // positive to negative
        // OPTIMIZE: avoid setting num->sign to 1 twice
        num->sign = 1;
        add_pow_2(0, num);
        num->sign = 1; // in case overflow occurred
    }
};

int negative_big_sub_pow_2(unsigned power, Bignum *num) {
    // (N - p) = -(p - N) = -(-N + p)
    // flip num
    // add p 
    // flip result
    flip_sign(num);
    add_pow_2(power, num);
    flip_sign(num);
}


int pow_2_sub_bignum(unsigned power, Bignum *num) {
    // subtracts the bignum from a power of 2
    flip_sign(num);
    add_pow_2(power, num);
}
/*
void test_flips() {
    Bignum ref = createBignum();
    add_pow_2(65, &ref); // 1
    print_binary(&ref);
    flip_sign(&ref); // -1
    print_binary(&ref);
    flip_sign(&ref); // 1
    print_binary(&ref);
}

void test_arithmetic() {
    for (int i = 1; i < 128; i++) {
     Bignum num = createBignum();
     Bignum ref = createBignum();
     add_pow_2(i, &num);
     add_pow_2(i, &num);
     add_pow_2(i, &num);
     pos_big_sub_small_pow_2(i-1, &num);
     add_pow_2(i + 1, &ref);
     add_pow_2(i - 1, &ref);
     printf("Expected \n");
     print_binary(&ref);
     printf("Calculated \n");
     print_binary(&num);
    }     
}

void test_twos_comp() {
    for (int i = 0; i < 127; i++) {
        Bignum num = createBignum();
        add_pow_2(i, &num);
        flip_sign(&num);
        add_pow_2(1, &num);
        add_pow_2(i, &num);
        print_binary(&num);
    }
}

int main() {
    //test_flips();
    //test_arithmetic();
    //test_twos_comp();
    return 0;
}
*/
/*
What I think works:

    -adding (two's compliment) WORKS
    -switching from positive to negative  WORKS 
    -switching from negative to positive  WORKS

    -subtracting a smaller power from a positive bignum WORKS
    -subtracting a bignum from a power (flip bignum then add) DEPENDS ON OTHERS
    -subtracting a power from a negative bignum (subtract bignum from power, then flip result) DEPENDS ON OTHERS
    -subtracting a larger power from a positive bignum TESTING

*/