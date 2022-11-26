#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifndef SATSOLVER_H
#define SATSOLVER_H



typedef struct Bignum {
    uint64_t *words;
    uint64_t n_words; 
    int sign;
} Bignum;

typedef struct Literal {
    unsigned long value;
    bool sign;
} Literal;

typedef struct Clause {
    Literal *literals;  // array of literals
    unsigned long numLiterals;    
} Clause;

typedef struct DimacsInfo {
	Clause* clauses;
	unsigned long numLiterals;
	unsigned long numClauses;
} DimacsInfo; 

typedef struct GenChild {
    unsigned long last_clause_num;
    unsigned long num_sol; // STORE POWER INSTEAD
    Clause *merged_clause;
} GenChild;

// functions in cleanParse
DimacsInfo parseDimacs(void);
void printClause(Clause *clause);

// functions in cleanMerge
Clause createClause(unsigned long *vals,  bool *signs, unsigned long numLits);
Clause* merge(Clause *clause1, Clause *clause2);

// functions in gen_pairing_sol
unsigned long gen_size(unsigned long k, unsigned long total_clauses);
unsigned long count_power(Clause *clause, unsigned long total_literals);
void free_clauses(GenChild **prev_generation, unsigned long prev_gen_size);
void gen_num_sol(GenChild **prev_generation, unsigned long *array_size, unsigned long total_clauses, 
                unsigned long total_literals, unsigned long k, Clause* clauses, int gen_num, Bignum *solution_count);
void populate_first_gen(GenChild **generations, Clause *clauses, unsigned long total_literals, 
                                 unsigned long total_clauses, Bignum *solution_count);


Bignum createBignum();

int print_binary(Bignum *num);

int print_hex(Bignum *num);

int isLessThanPower(unsigned power, Bignum *num);

int add_pow_2(unsigned power, Bignum *num);

int pos_big_sub_small_pow_2(unsigned power, Bignum *num);

int pos_big_sub_big_pow_2(unsigned power, Bignum *num);

int flip_sign(Bignum *num);

int negative_big_sub_pow_2(unsigned power, Bignum *num);

int pow_2_sub_bignum(unsigned power, Bignum *num);


#endif 