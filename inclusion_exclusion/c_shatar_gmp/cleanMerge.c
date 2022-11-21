#include "satsolver.h"


Clause createClause(unsigned long *vals,  bool *signs, unsigned long numLits) {
/*  Params:
        vals - pointer to array of literals that make up the clause
        signs -pointer to array of booleans representing the sign on corresponding literal in the clause
        numLits - the size, i.e. number of literals, of the clause
    Return:
        CLause struct containing the data for this clause
*/
    Clause clause = {NULL, 0};  // Defaults to empty clause  
    clause.literals = malloc(sizeof(Literal)*numLits);
    assert(clause.literals);
    // create literal struct and insert in array
    for (int i = 0; i < numLits; i++) {
        Literal newLit = {vals[i], signs[i]};
        clause.literals[i] = newLit;
    } 
    clause.numLiterals = numLits;
    return clause;
}

Clause* merge(Clause *clause1, Clause *clause2) {
    /* Params:
         clause1 - pointer to first clause struct being merged
         clause2 - pointer to second clause struct being merged
       Return:
         pointer to a heap allocated clause struct or NULL if there was a conflict

    */
    // ensures enough space for union of the clauses
    unsigned long maxSize = clause1->numLiterals + clause2->numLiterals;
    Literal* newLiterals = malloc(sizeof(Literal) * maxSize);
    assert(newLiterals);
    // track our place within the merge function
    unsigned long clause1idx = 0, clause2idx = 0, newClauseSize = 0;
    Literal* lastAdded = NULL; // will allow us to detect potential conflicts later
    // iterate over ALL literals
    while(clause1idx < clause1->numLiterals || clause2idx < clause2->numLiterals) {
        Literal* next;
        // The next literal to be added is one of least value  
        if (clause1idx < clause1->numLiterals && clause2idx < clause2->numLiterals) {  
            next = clause1->literals[clause1idx].value < clause2->literals[clause2idx].value 
            ? clause1->literals + clause1idx : clause2->literals + clause2idx;
        } 
        else if (clause1idx == clause1->numLiterals) next = clause2->literals + clause2idx;
        else next = clause1->literals + clause1idx;    
        // check for conflict
        if (lastAdded && (next->value == lastAdded->value)) { 
            // clean up memory, no clause returned because of conflict
            if (next->sign != lastAdded->sign) {
                free(newLiterals); 
                return NULL;
            }
            // skip repeat literal
            if (next == clause1->literals + clause1idx) clause1idx++;
            else clause2idx++;
        } else {
         newLiterals[newClauseSize] = *next;
         newClauseSize++;
         lastAdded = next;
        }
        // increments past the newly added literal (might be able to do this earlier by the ternary assignment )
        if (lastAdded == clause1->literals + clause1idx) clause1idx = clause1idx < clause1->numLiterals ? clause1idx + 1 : clause1idx;
        if (lastAdded == clause2->literals + clause2idx) clause2idx = clause2idx < clause2->numLiterals ? clause2idx + 1 : clause2idx;
    }
    Clause result = {realloc(newLiterals, sizeof(Literal) * newClauseSize), newClauseSize};
    assert(result.literals);
    Clause *savedClause = malloc(sizeof(Clause));
    assert(savedClause);
    *savedClause = result;
    return savedClause;
}
