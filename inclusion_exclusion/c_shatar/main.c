#include "satsolver.h"
#include <time.h>

int NUM_TRIALS = 10000;


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


void sat_solver(DimacsInfo input){
    Clause *clauses_array = input.clauses;
    unsigned long total_literals = input.numLiterals;
    unsigned long total_clauses = input.numClauses;
    
    unsigned long total_possible_soln = 1 << total_literals; // gmp

    // first generation will have (# of clauses) GenChilds
    unsigned long gen_array_size = total_clauses;
    GenChild *generation = malloc(gen_array_size * sizeof(GenChild));
    unsigned long first_gen_soln = populate_first_gen(&generation, clauses_array, total_literals, total_clauses); //gmp
    //print_generation(generation, gen_array_size, 1);
    
    //if (first_gen_soln < total_possible_soln) {
        //printf("sat\n");
        //return;
    //}
    
    unsigned long I_k = first_gen_soln;
    int gen_num = 2;
    
    for (size_t k = 2; k <= total_clauses; ++k){
        // at the odd step
        if (k % 2 == 1) {
            unsigned long num_soln_k_pairs = gen_num_sol(&generation, &gen_array_size, total_clauses, 
                                                         total_literals, k, clauses_array, gen_num);
            //print_generation(generation, gen_array_size, gen_num);
            I_k += num_soln_k_pairs;
            
            // here we over estimated so if it is less than the CNF is SAT
            //if (I_k < total_possible_soln){
                //printf("sat\n");
                //return;
            //}
        // at the even step
        } else {
            unsigned long num_soln_k_pairs = gen_num_sol(&generation, &gen_array_size, total_clauses, 
                                                         total_literals, k, clauses_array, gen_num);
            //print_generation(generation, gen_array_size, gen_num);
            I_k -= num_soln_k_pairs;

            // here we underestimated so if it is equal to total_possible, then it is unsat
            //if (I_k == total_possible_soln) {
                //printf("unsat\n");
                //return;
            //}

        }
        gen_num++;
    }
    
    printf("outside loop \n");
    printf("solutions to DNF: %lu \n", I_k);
    printf("solutions to CNF: %lu \n", total_possible_soln - I_k);
    return;
}

int main(void) {
    ///manually create a DimacsInfo for testing purposes
    //Clause *clauses_array = basicTest(); // will be freed at the very end only!!
    // unsigned long numLiterals = 5;
    //unsigned long numClauses = 4;
    //DimacsInfo data = {clauses_array, numLiterals, numClauses};
    // print the Dimacs
    //printf("printing DimacsInfo: \n");
    //for (size_t i = 0; i < data.numClauses; i++) {
    // printClause(data.clauses + i);
    //}
    
    DimacsInfo data =  parseDimacs();

    Clause *clauses_array = data.clauses;
    //for (size_t i = 0; i < data.numClauses; ++i){
       // printClause(&clause_array[i]);
    //}
    
    // Calculate the time taken by sat_solver()
    clock_t t;
    t = clock();
    for(int i = 0; i < NUM_TRIALS; ++i){
        sat_solver(data);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
 
    printf("sat_solver took %f seconds to execute on average \n", time_taken/NUM_TRIALS);
    

    // now free the clause array
    for (int i = 0; i < data.numClauses; ++i){
       free(clauses_array[i].literals);
    }
    free(clauses_array);
    return 0;
}
