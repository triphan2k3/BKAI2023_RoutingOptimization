#include <stdio.h>
#include <chrono>
#include <iomanip>
#include <iostream>

#ifndef _SUBMIT_
#include "./hpp/Helper.hpp"
#include "./hpp/Instance.hpp"
#include "./hpp/Optimiser.hpp"
#include "./hpp/Solution.hpp"
#include "./hpp/Solver.hpp"
#endif
const int MAX_RUNTIME = 250;
std::chrono::steady_clock::time_point start;
int getRunTime() {
    return duration_cast<seconds>(steady_clock::now() - start).count();
}
using namespace std;

int main(int argc, char* argv[]) {
    if (argc == 1) {
        freopen("./INPUT/01.txt", "r", stdin);
        freopen("./OUTPUT/01.txt", "w", stdout);
    } else {
        freopen(argv[1], "r", stdin);
        freopen(argv[2], "w", stdout);
    }
    start = chrono::steady_clock::now();
    Instance instance;
    instance.readInstance();
    instance.encode();

    // Solution onlyInit = Solver::BetterConstructionHeuristics(instance);

    Solution sol = Solver::RouteConstructionHeuristics(instance);
    // Solution sol = onlyInit;
    while (getRunTime() < 250) {
        sol = AGES::AGES_RUN(sol, instance);
        sol = LNS::LNS_RUN(sol, instance);
    }
    sol.printAnswer(instance);
    if (argc == 3) {
        ofstream os("temp.txt", ios_base::app);
        os << argv[1] << " " << setprecision(1) << fixed
           << sol.objective(instance) << '\n';
        // os << setprecision(1) << fixed << sol.objective(instance) << '\n';
        os.close();

        os.open("log.txt", ios_base::app);
        if (sol.isValid(instance))
            os << argv[1] << " "
               << "TRUE\n";
        else
            os << argv[1] << " "
               << "FALSE\n";
        os.close();
    }
    return 0;
}