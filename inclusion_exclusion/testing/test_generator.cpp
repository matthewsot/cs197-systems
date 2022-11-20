#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <experimental/random>

void print_clause(std::vector<int> clause){
    for (int var : clause){
        int sign = std::experimental::randint(0, 1);
        if (sign == 0){
            std::cout << "-" << var << " ";
        }else{
            std::cout << var << " ";
        }
    }
    std::cout << "0" << std::endl;
}

void print_clauses(int num_variables, int num_clauses){
    std::vector<int> v = {};
    for(int i = 1; i < num_variables + 1; ++i){
        v.push_back(i);
    }
    std::random_device rd;
    std::mt19937 g(rd());
 
    for (int i = 0; i < num_clauses; ++i){
        std::shuffle(v.begin(), v.end(), g);
        std::vector<int> result = {};
        int num_clause_vars = std::experimental::randint(1, num_variables);
        //std::cout << num_clause_vars << std::endl;
        for(int i = 0; i < num_clause_vars; ++i){
            result.push_back(v[i]);
        }
        print_clause(result);
    }
}

int main()
{
    int num_variables = 30;
    int num_clauses = 20;
    
    std::cout << "p cnf " << num_variables << " " << num_clauses << std::endl;

    print_clauses(num_variables, num_clauses);
}


