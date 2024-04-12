#include <stdio.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "./hpp/AGES.hpp"
#include "./hpp/Helper.hpp"
#include "./hpp/Instance.hpp"
#include "./hpp/LNS.hpp"
#include "./hpp/Solution.hpp"
#include "./hpp/Solver.hpp"

const int MAX_RUNTIME = 300;
const string INIT = "INIT";
const string AGES = "AGES";
const string LNS = "LNS";
const string HYBRID = "HYBRID";

std::chrono::steady_clock::time_point Helper::start;
std::mt19937 Helper::gen;
using namespace std;

struct agruments {
    string inputFile;
    string outputFile;
    string solver;
    string runningDateTime;
} agrument;
bool doAGES = false;
bool doLNS = false;
int initTime = 30;

void setup() {
    freopen(agrument.inputFile.c_str(), "r", stdin);
    freopen(agrument.outputFile.c_str(), "w", stdout);
    if (agrument.solver == INIT)
        initTime = MAX_RUNTIME;
    if (agrument.solver == AGES || agrument.solver == HYBRID)
        doAGES = true;
    if (agrument.solver == LNS || agrument.solver == HYBRID)
        doLNS = true;
}

int main(int argc, char* argv[]) {
    int fff = 0;
    if (argc == 1) {
        agrument.inputFile = "./INPUT/03.txt";
        agrument.runningDateTime = Helper::currentDateTime();
        agrument.outputFile =
            "./OUTPUT/03.txt_" + agrument.runningDateTime + ".txt";
        agrument.solver = AGES;
    } else {
        agrument.inputFile = argv[1];
        agrument.runningDateTime = Helper::currentDateTime();
        agrument.outputFile =
            (string)argv[2] + "_" + agrument.runningDateTime + ".txt";
        agrument.solver = argv[3];
    }

    setup();
    Instance instance;
    instance.readInstance();
    instance.encode();

    /*==========================================
    ---------------INITIAL SOLUTION-------------
    ==========================================*/
    Helper::initRandomAndRunTime();
    Solution sol = Solver::ConstructSolution(instance);
    vector<int> P(instance.nTruck);
    iota(P.begin(), P.end(), 1);
    while (Helper::getRunTime() < initTime) {
        shuffle(P.begin(), P.end(), Helper::gen);
        Solution newSol = Solver::ConstructSolution(instance, P);
        if (newSol.objective(instance) > sol.objective(instance))
            sol = newSol;
    }

    /*==========================================
    ----------------LOCAL SEARCH----------------
    ==========================================*/
    while (Helper::getRunTime() < MAX_RUNTIME) {
        if (doAGES)
            sol = AGES::run(sol, instance);
        if (doLNS)
            sol = LNS::run(sol, instance);
    }

    /*==========================================
    ---------------PRINT SOLUTION---------------
    ==========================================*/

    sol.printAnswer(instance, agrument.inputFile, agrument.solver);

    ofstream os;
    // Print for fun, can be removed
    // os.open("temp.txt", ios_base::app);
    // os << agrument.inputFile << " " << setprecision(1) << fixed
    //    << sol.objective(instance) << '\n';
    // os << setprecision(1) << fixed << sol.objective(instance) << '\n';
    // os.close();

    // Print to validate the solution
    // os.open("log.txt", ios_base::app);
    // if (sol.isValid(instance))
    //     os << agrument.runningDateTime << " " << agrument.inputFile << " "
    //        << "TRUE\n";
    // else
    //     os << agrument.runningDateTime << " " << agrument.inputFile << " "
    //        << "FALSE\n";
    // os.close();

    // Print important information
    os.open("printAnswer.csv", ios_base::app);
    pair<float, pair<int, int>> info = sol.betterObj(instance);
    os << agrument.runningDateTime << "," << agrument.inputFile << ","
       << info.second.first << "," << info.second.second << ","
       << setprecision(2) << fixed << info.first << "\n";
    os.close();
    return 0;
}
