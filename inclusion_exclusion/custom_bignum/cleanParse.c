#include "satsolver.h"

#define  DEFAULT_CLAUSE_SIZE 8

int compareLiterals(const void* literalA, const void* literalB) {
    Literal *A = (Literal*) literalA;
    Literal *B = (Literal*) literalB;
    return A->value - B->value;
}

DimacsInfo parseDimacs(void) {
    /*
        Matthew's parser implementation was referenced when writing this parser.
    */
    // Read comment lines at the start.
    for (char c; (c = getc(stdin)) == 'c';)
        while (getc(stdin) != '\n');

    unsigned long numLiterals, numClauses;
    // Read in number of literals and clauses
        assert(scanf(" cnf %lu %lu\n", &numLiterals, &numClauses) == 2);
        //N_VARS++; why???
        Clause *clauses = malloc(sizeof(Clause) * numClauses);
        assert(clauses);

        for (size_t i = 0; i < numClauses; i++) {
            Clause clause;
            size_t max = DEFAULT_CLAUSE_SIZE;
            size_t trueSize = 0;
            clause.literals = malloc(sizeof(Literal) * DEFAULT_CLAUSE_SIZE);

            int literal = 0;
            for (assert(scanf("%d ", &literal)); literal; assert(scanf("%d ", &literal))) {
                int val = literal < 0 ? literal * -1 : literal;
                bool sign = literal > 0;
                Literal literal = {val, sign};
                clause.literals[trueSize] = literal;
                trueSize++;
                if (trueSize == max) {
                    max *= 2;
                    clause.literals = realloc(clause.literals, sizeof(Literal) * max);
                }
            }
            clause.literals = realloc(clause.literals, sizeof(Literal) * trueSize);
            clause.numLiterals = trueSize;
            qsort(clause.literals, clause.numLiterals, sizeof(Literal), compareLiterals);
            clauses[i] = clause;
        }
    DimacsInfo parseData = {clauses, numLiterals, numClauses};
    return parseData;
}


void printClause(Clause *clause) {
    unsigned long numLits = clause ? clause->numLiterals : 0;
	printf("Number of Literals: %lu \n", numLits);
	for (unsigned long i = 0; i < numLits; i++) {
        if (clause->literals[i].sign) printf("-");
	    printf("%lu ", clause->literals[i].value);
	}
	printf("\n");
}

/*
FOR TESTING
int main(void) {
  DimacsInfo data = parseDimacs();
    for (size_t i = 0; i < data.numClauses; i++) {
       printf("Clause #%lu ", i + 1);
       printClause(data.clauses + i);
    }
    return 0;
}
*/