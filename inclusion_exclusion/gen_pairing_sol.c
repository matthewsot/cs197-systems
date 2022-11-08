typedef struct Literal {
    unsigned long value;
    bool sign;
} Literal;

typedef struct Clause {
    Literal *literals;
    unsigned long numLiterals;    
} Clause;


typedef struct GenChild {
    unsigned long last_clause_num;
    unsigned long num_sol;
    Clause *merged_clause;
} GenChild;

typedef struct 

unsigned long gen_size(unsigned long k, unsigned long total_clauses){
    // basically we need to return total_clauses choose k
    return 6;
}

/*
* parameters:
*  prev_generation: an array of GenChilds that stores the solutions to
                    the last merge (so if we are calling with k = 3 then
                    it stores the sols to k = 2) - technically the memory address
                    of the variable that stores the array is what prev_generation is
*/
int gen_num_sol(GenChild **prev_generation, unsigned long *array_size, unsigned long total_clauses, 
                unsigned long k, Clause* clauses)
{
    // so we know that when k is 2, we have the results of the 1st generation ready and we
    // want to calculate the results of the next generation
    unsigned long prev_gen_size = *array_size;
    GenChild *prev_gen = *prev_generation;
    GenChild *new_gen = malloc(gen_size(k, total_clauses) * sizeof(GenChild))
    unsigned long cur_index = 0;
    unsigned long total_solution = 0;
    for(unsigned long i = 0; i < prev_gen_size; ++i){
        // iterate over each entry to see the new generation k-pairs that can be made
        GenChild cur_child = prev_gen[i];
        unsigned long last_clause = cur_child.last_clause_num;
        // loop over all claues we can merge this one with
        for (unsigned long j = last_clause + 1; j <= total_clauses; ++j){
            unsigned long cur_clause_num = j;
            // we need to merge cur_child_clause with this clause
            // clause array should be such that clause 1 is at index 1
            Clause *result = merge(clauses[cur_clause_num], cur_child.merged_clause);
            unsigned long num_sol = calc_num_sol(result);
            if ((num_sol) == 0) {continue;}
            // add the merged clause to the new generation
            new_gen[cur_index] = {cur_clause, num_sol, result};
            cur_index++;
            total_solution += num_sol;
        }
    }
    // we need to free the previous generation, obviously also the merged clauses 
    // within it

    for (size_t i = 0; i < prev_gen_size; i++) {
        free(prev_gen[i].merged_clause);
    }
    free(prev_gen);
    // and then this new_gen becomes the prev_gen
    *prev_generation = new_gen;
}

/* This function populates the first generation in generations (an array of GenChilds)
 *
 * The first generation will consist of the following:
 * 
 * - numClauses # of GenChilds
 * 
 * - ith GenChild will consist of the following:
 *      - last literal of ith clause
 *      - num of solutions of ith clause
 *      - ith clause
 */ 

unsigned long populate_first_gen(GenChild **generations, Clause *clauses, unsigned long total_literals, unsigned long total_clauses) {
    unsigned long first_gen_soln = 0;

    for (size_t i = 0; i < total_clauses; i++) {
        unsigned long num_soln = count_solutions(clauses[i], total_literals);
        // summing solutions to all clauses
        first_gen_soln += num_soln;
        // Since clauses are sorted, we get last literal of clause
        unsigned long last_clause_num = (clauses[i].literals)[clauses[i].numLiterals];
        // dynamically allocate memory for merged clause
        (*generations)[i].merged_clause = malloc(sizeof(clauses[i]));
        (*generations)[i] = {last_clause_num, num_soln, clauses[i]};

    }

    return first_gen_soln;
}
/* This function creates an array of GenChilds, called generations. The first generation
 * is populated with the clauses themselves. Then, it receives the overestimated and
 * underestimated count of solutions from other functions. This estimation determines
 * whether the problem is sat or unsat.
 *
 */

char *sat_solver(Clause *clauses, unsigned long total_literals, unsigned long total_clauses) {

    unsigned long total_possible_soln = count_solutions();
    unsigned long first_gen_soln = 0;

    // first generation will have (# of clauses) GenChilds
    unsigned long gen_array_size = total_clauses;
    GenChild *generations = malloc(gen_array_size * sizeof(GenChild));
    unsigned long first_gen_soln = populate_first_gen(&generations, clauses, total_literals, total_clauses);

    if (first_gen_soln < total_possible_soln) {

        return "sat";
    }

    unsigned long num_cur_soln = first_gen_soln;
    
    for (size_t i = 2; i < numClauses + 1; i++) {
        // at the odd step
        if (i % 2 == 1) {
            unsigned long num_soln_k_pairs = gen_num_soln(&generations[i], &gen_array_size, total_clauses, i, clauses);
            num_cur_soln += num_soln_k_pairs;
            
            // here we over estimated so if it is less than the CNF is SAT
            if (num_cur_soln < total_possible_soln) {
                return "sat";
            }
        // at the even step
        } else {
            unsigned long num_soln_k_pairs = gen_num_soln(&generations[i], &gen_array_size, total_clauses, i, clauses);
            num_cur_soln -= num_soln_k_pairs;

            // here we underestimated so if it is equal to total_possible, then it is unsat
            if (num_cur_soln == total_possible_soln) {
                return "unsat";
            }

        }

    }



}


