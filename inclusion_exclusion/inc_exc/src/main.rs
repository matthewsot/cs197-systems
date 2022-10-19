// Forget about #1 for now and focus on #2. How can we estimate this? Well,
// pick a k and then pick k clauses at random and see if their
// merge/intersection is empty. Repeat this, say, 100 times for each k from
// 1 to 100. That should give us an estimate, for each k, of how many
// "merged clauses" we'll have to deal with at each depth. Fewer is better.

// Run this program on all of SAT-COMP. Look for instances that have few
// merges/intersections, and also that existing SAT solvers do poorly on
// (e.g., run kissat with a short timeout for each). That should give you a
// good idea of which problem instances might be most worthwhile to target.
// The goal would be to find at least one problem in SAT-COMP that
// inclusion/exclusion should be expected to do much better on.

// I expect this to be perhaps a bit easier than actually implementing the
// solver? Again, all you need here is a way to read the DIMACS input
// files, choose k of them at random (in Python, check out the
// random.choices method), and then check if any of them contain
// conflicting literals. Then run that against the SAT-COMP benchmark
// suite.

use std::{
    collections::{HashMap, VecDeque},
    fs,
    path::PathBuf,
    time::Instant,
};

use load::{Clause, DNF};

use crate::load::parse_dimacs;

mod load;

// reimplemented in solve
// /// returns a vec of vecs of indices that correspond to subsets with the given size of a CNF with the given number of clauses
// /// ```
// /// assert_eq!(gen_subsets(3,2), 3);
// /// assert_eq!(gen_subsets(9,4), 126);
// /// ```
// fn gen_subsets(clause_count: usize, size: usize) -> Vec<Vec<usize>> {
//     let mut out: Vec<Vec<usize>> = Vec::new();
//     let mut queue: VecDeque<Vec<usize>> = VecDeque::from(vec![vec![]]);
//     while let Some(set) = queue.pop_front() {
//         if set.len() == size {
//             out.push(set);
//         } else {
//             // extend by another index
//             // this new index must be greater than the last index in the set
//             for new_index in (set.last().map(|&e| e + 1).unwrap_or(0))..(clause_count) {
//                 let mut new_set = set.clone();
//                 new_set.push(new_index);
//                 queue.push_back(new_set);
//             }
//         }
//     }
//     out
// }

#[derive(Debug, PartialEq)]
enum SolutionResult {
    Inconclusive,
    Satisfiable,
    Unsatisfiable,
}

enum Merge {
    Set(Clause),
    Incompatible,
}

fn merge(a: Clause, b: &Clause) -> Merge {
    let mut output = a;
    for (&var, &value) in b {
        if let Some(old_value) = output.insert(var, value) {
            if old_value != value {
                return Merge::Incompatible;
            }
        }
    }
    Merge::Set(output)
}

// max_size is the maximum allowed size for subset generation
fn solve(dnf: &DNF, max_size: usize) -> (SolutionResult, usize, i64) {
    // a map from a set of clause indices to their merged set if it is valid
    let mut cache: HashMap<Vec<usize>, Clause> = HashMap::new();
    cache.insert(vec![], HashMap::new());
    let mut queue: VecDeque<Vec<usize>> = VecDeque::from(vec![vec![]]);
    let mut sum = 0;
    let total_vars = dnf
        .iter()
        .map(|h| h.iter())
        .flat_map(|i| i)
        .collect::<HashMap<_, _>>()
        .len() as u32;
    let mut max_seen_size = 0;
    while let Some(set) = queue.pop_front() {
        // remove from cache the merged set for this clause set.
        let cache_entry = cache
            .remove(&set)
            .expect("Must always have a previous cache entry");
        if set.len() != max_size {
            // extend by another index
            // this new index must be greater than the last index in the set
            for new_index in (set.last().map(|&e| e + 1).unwrap_or(0))..(dnf.len()) {
                // merge the cached set with the clause at the new_index
                match merge(cache_entry.clone(), &dnf[new_index]) {
                    // if the merge succeeds,
                    Merge::Set(clause) => {
                        let mut new_set = set.clone();
                        new_set.push(new_index);
                        // add this set of clauses in the queue to be explored further
                        queue.push_back(new_set.clone());
                        // add to sum the size of the solution space for this merged set
                        sum += (if new_set.len() % 2 == 0 { -1 } else { 1 })
                            * i64::pow(2, total_vars - clause.len() as u32);
                        // and insert the result of the merge into the cache
                        cache.insert(new_set, clause);
                    }
                    // if incompatible, we do not wish to further explore this and thus do not insert into queue
                    // HOWEVER, it may be wise to modify cache structure to allow storing INVALID results
                    // we may explore 1,3 before 1,2,3. if 1,3 is invalid, we may store that it is invalid and look up later
                    // however, 1,3 is not a direct ancestor of 1,2,3. thus, this search would necessitate linear probing of
                    // every possible indirect ancestor in the previous generation: 2,3; 1,3; 1,2
                    // thus, we do not implement it here
                    Merge::Incompatible => {}
                }
            }
        } else {
            // reached maximum size
            return (SolutionResult::Inconclusive, max_seen_size, sum);
        }
        // if this new length is greater than all previous seen sizes, we must have
        // reached a new level of lengths
        // by nature of the queue logic, the sizes must be always nondecreasing
        // therefore, once we "level up", we may analyze the previous level of sizes
        if set.len() > max_seen_size {
            // check the sum and see if it is (a lower bound and above 2^N) or (an upper bound and below 2^n)
            if max_seen_size % 2 == 0 && sum >= i64::pow(2, total_vars) {
                return (SolutionResult::Unsatisfiable, max_seen_size, sum);
            } else if max_seen_size % 2 == 1 && sum < i64::pow(2, total_vars) {
                return (SolutionResult::Satisfiable, max_seen_size, sum);
            }
            max_seen_size = set.len();
        }
    }
    // the last "level up" is not caught inside the loop
    // therefore, we add this final check after we've reached the maximum size of subsets and exited the loop
    if sum >= i64::pow(2, total_vars) {
        (SolutionResult::Unsatisfiable, max_seen_size, sum)
    } else {
        (SolutionResult::Satisfiable, max_seen_size, sum)
    }
}

fn main() {
    let check = |dir: &str, expected_result: SolutionResult| {
        for file in fs::read_dir(dir).unwrap().map(|entry| entry.unwrap()) {
            let dnf = parse_dimacs(&{
                let file = &file.path();
                fs::read_to_string(file)
                    .expect(&format!("Failed to open file {}", file.to_string_lossy()))
            })
            .expect("invalid DIMACS in sample input")
            .1;
            let start = Instant::now();
            let (result, _, _) = solve(&dnf, 5);
            let duration = start.elapsed();
            assert!(result == expected_result);

            println!("{:?} for {} clauses", duration, dnf.len());
        }
    };
    check("samples/sat", SolutionResult::Satisfiable);
    check("samples/unsat", SolutionResult::Unsatisfiable);
}