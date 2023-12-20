#include <stdio.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#ifndef _SUBMIT_
#include "./hpp/AGES.hpp"
#include "./hpp/Helper.hpp"
#include "./hpp/Instance.hpp"
#include "./hpp/LNS.hpp"
#include "./hpp/Solution.hpp"
#include "./hpp/Solver.hpp"
#endif
const int MAX_RUNTIME = 600;
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

#ifdef something
    ofstream os("temp.txt", ios_base::app);
    os << argv[1] << " " << instance.nTruck << " " << instance.nOrder << " "
       << instance.nHub << "\n";
    // os << setprecision(1) << fixed << sol.objective(instance) << '\n';
    os.close();
    return 0;
#endif

    // Solution onlyInit = Solver::BetterConstructionHeuristics(instance);

    Solution sol = Solver::ConstructSolution(instance);
    // Solution sol = onlyInit;
    vector<int> P(instance.nTruck);
    iota(P.begin(), P.end(), 1);
    while (getRunTime() < MAX_RUNTIME / 5) {
        random_shuffle(P.begin(), P.end());
        Solution newSol = Solver::ConstructSolution(instance, P);
        if (newSol.objective(instance) > sol.objective(instance))
            sol = newSol;
    }

    while (getRunTime() < MAX_RUNTIME) {
        sol = AGES::run(sol, instance);
        sol = LNS::run(sol, instance);
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

        os.open("printAnswer.txt", ios_base::app);
        pair<float, pair<int, int>> info = sol.betterObj(instance);
        os << Helper::currentDateTime() << "," << argv[1] << ","
           << info.second.first << "," << info.second.second << ","
           << setprecision(2) << fixed << info.first << "\n";
        os.close();
    }
    return 0;
}