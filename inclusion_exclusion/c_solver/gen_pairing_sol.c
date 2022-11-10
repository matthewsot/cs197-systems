#include "satsolver.h"

unsigned long gen_size(unsigned long k, unsigned long total_clauses){
    // basically we need to return total_clauses choose 
    // but just return a large constat for now
    return 100;
}

unsigned long count_solutions(Clause *clause, unsigned long total_literals){
    unsigned long numLiterals = clause->numLiterals;
    printf("counting solutionsss\n");
    printf("numLiterals is %ld \n", numLiterals);
    // when we do big num then different way obviously
    unsigned long num_sol = pow(2, (total_literals - numLiterals));
    printf("so num sol is %ld \n", num_sol);
    return num_sol;
}


void free_clauses(GenChild **prev_generation, unsigned long prev_gen_size){
    GenChild *prev_gen = *prev_generation;
    for(size_t i = 0; i < prev_gen_size; ++i){
        GenChild cur_child = prev_gen[i];
        Clause *clause_to_free = cur_child.merged_clause;
        free(clause_to_free->literals);
        free(clause_to_free);
    }
}

/*
* parameters:
*  prev_generation: an array of GenChilds that stores the solutions to
                    the last merge (so if we are calling with k = 3 then
                    it stores the sols to k = 2) - technically the memory address
                    of the variable that stores the array is what prev_generation is
*/
unsigned long gen_num_sol(GenChild **prev_generation, unsigned long *array_size, unsigned long total_clauses, 
                unsigned long total_literals, unsigned long k, Clause* clauses, int gen_num)
{
    // so we know that when k is 2, we have the results of the 1st generation ready and we
    // want to calculate the results of the next generation
    printf("generating num sol for generation number: %d \n", gen_num);
    unsigned long prev_gen_size = *array_size;
    GenChild *prev_gen = *prev_generation;
    unsigned long new_gen_size = gen_size(k, total_clauses); // this is the max size possible
    GenChild *new_gen = malloc(new_gen_size * sizeof(GenChild));
    unsigned long cur_index = 0;
    unsigned long total_solution = 0;
    for(unsigned long i = 0; i < prev_gen_size; ++i){
        // iterate over each entry to see the new generation k-pairs that can be made
        GenChild cur_child = prev_gen[i];
        unsigned long last_clause = cur_child.last_clause_num;
        // loop over all claues we can merge this one with
        for (unsigned long j = last_clause + 1; j < total_clauses; ++j){
            int cur_clause_num = j;
            // we need to merge cur_child_clause with this clause
            // clause array should be such that clause 0 is at index 0
            printf("now merging clause number %d with merged clause where last clause is %ld \n",
                    cur_clause_num, last_clause);
            Clause *result = merge(&clauses[cur_clause_num], cur_child.merged_clause);
            if (result == NULL){
                printf("numsol is zero so NOT making the entry in generation \n"); 
                continue;
            }
            unsigned long num_sol = count_solutions(result, total_literals);
            printf("numsol is nonzero so making the entry in generation \n");
            // add the merged clause to the new generation
            new_gen[cur_index] = (GenChild){cur_clause_num, num_sol, result};
            cur_index++; // this will tell the true number of childs 
            total_solution += num_sol;
        }
    }
    // we need to free the previous generation, obviously also the merged clauses 
    // within it, however, we do not free the merged clauses in gen 1
    if (gen_num != 2){
        free_clauses(prev_generation, prev_gen_size);
    }
    free(prev_gen);
    *prev_generation = new_gen;
    *array_size = cur_index; 
    return total_solution;
}

/* This function populates the first generation in generations (an array of GenChilds)
 *
 * The first generation will consist of the following:
 * 
 * - numClauses # of GenChilds
 * 
 * - ith GenChild will consist of the following:
 *      - i as the last clause num
 *      - num of solutions of ith clause
 *      - ith clause
 */ 

unsigned long populate_first_gen(GenChild **generations, Clause *clauses, unsigned long total_literals, 
                                 unsigned long total_clauses) {
    unsigned long first_gen_soln = 0;
    GenChild *first_gen = *generations;

    for (size_t i = 0; i < total_clauses; i++) {
        unsigned long num_soln = count_solutions(&clauses[i], total_literals);
        // summing solutions to all clauses
        first_gen_soln += num_soln;
        // for first gen, the last_clause_num is just the clause number
        unsigned long last_clause_num = i;
        GenChild new_child = {last_clause_num, num_soln, &clauses[i]};
        first_gen[i] = new_child;

    }

    return first_gen_soln;
}

