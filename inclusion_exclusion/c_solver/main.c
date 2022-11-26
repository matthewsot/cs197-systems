#include "satsolver.h"


Clause *basicTest(){
    // creates and returns an array of clauses.
    unsigned long clauseVals1[] = {1, 2, 3, 4};
    bool clauseSigns1[] = {true, true, true, true};
    unsigned long clauseVals2[] = {1, 2, 3, 4, 5};
    bool clauseSigns2[] = {true, true, true, true, true};
    unsigned long clauseVals3[] = {1, 3, 4};
    bool clauseSigns3[] = {true, true, true};
    unsigned long clauseVals4[] = {1, 5};
    bool clauseSigns4[] = {true, false};
    
    Clause clause1 = createClause(clauseVals1, clauseSigns1, 4);
    
    Clause clause2 = createClause(clauseVals2, clauseSigns2, 5);

    Clause clause3 = createClause(clauseVals3, clauseSigns3, 3);
    
    Clause clause4 = createClause(clauseVals4, clauseSigns4, 2);
    
    Clause *clauses = malloc(4 * sizeof(Clause));
    clauses[0] = clause1;
    clauses[1] = clause2;
    clauses[2] = clause3;
    clauses[3] = clause4;
    
    return clauses;
}

void print_generation(GenChild *generation, int gen_size, int gen_number){
    printf("printing generation number %d , the size is: %d \n", gen_number, gen_size);
    for(int i = 0; i < gen_size; i++){
        GenChild cur_child = generation[i];
        printf("child number: %d , last_clause_num: %ld , num_sol: %ld \n", 
        i, cur_child.last_clause_num, cur_child.num_sol);
        printf("Now printing the merged clause: \n");
        printClause(cur_child.merged_clause);
    }
}

void sat_solver(DimacsInfo input){
    Clause *clauses_array = input.clauses;
    unsigned long total_literals = input.numLiterals;
	unsigned long total_clauses = input.numClauses;
	
    //unsigned long total_possible_soln_pow = total_literals; 

    // first generation will have (# of clauses) GenChilds
    unsigned long gen_array_size = total_clauses;
    GenChild *generation = malloc(gen_array_size * sizeof(GenChild));
    Bignum I_k = createBignum();
    populate_first_gen(&generation, clauses_array, total_literals, total_clauses, &I_k);
    print_generation(generation, gen_array_size, 1);
    /*if (isLessThanPower(total_literals, &I_k)) {
        printf("sat\n");
        return;
    }
    */
    //unsigned long I_k = first_gen_soln;
    int gen_num = 2;
    
    for (size_t k = 2; k <= total_clauses; ++k){
         gen_num_sol(&generation, &gen_array_size, total_clauses, 
                      total_literals, k, clauses_array, gen_num, &I_k);
            print_generation(generation, gen_array_size, gen_num);
        //int isLess = isLessThanPower(total_literals, &I_k);
        // at the odd step
        /*
        if (k % 2 == 1) {
            // here we over estimated so if it is less than the CNF is SAT
            if (isLess){
                printf("sat\n");
                return;
            }
        // at the even step
        } else {
            // here we underestimated so if it is equal to total_possible, then it is unsat
            if (!isLess) {
                printf("unsat\n");
                return;
            }
        }
        */
        gen_num++;
    }
    // obviously we never come here, this was when I was testing gen by gen
    printf("Number of solutions:  ");
    pow_2_sub_bignum(total_literals, &I_k);
    print_binary(&I_k);
    print_hex(&I_k);
    return;
}

int main(void) {
    // manually create a DimacsInfo for testing purposes
    Clause *clauses_array = basicTest(); // will be freed at the very end only!!
    unsigned long numLiterals = 5;
	unsigned long numClauses = 4;
	DimacsInfo data = parseDimacs(); //{clauses_array, numLiterals, numClauses};
	// print the Dimacs
	printf("printing DimacsInfo: \n");
	for (size_t i = 0; i < data.numClauses; i++) {
	    printClause(data.clauses + i);
	    
	}
	
	sat_solver(data);
	// now free the clause array
	for (int i = 0; i < numClauses; ++i){
	    free(clauses_array[i].literals);
	}
	free(clauses_array);

    return 0;
}