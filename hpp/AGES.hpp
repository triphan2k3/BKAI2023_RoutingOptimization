#ifndef _AGES_HPP_
#define _AGES_HPP_

#include <math.h>
#include <algorithm>
#include <numeric>
#include <random>
using namespace std;
#ifndef _SUBMIT_
#include "Instance.hpp"
#include "Perturb.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#endif

class AGES {
   public:
    static pair<vector<int>, vector<int>> AGES_INSERTED_PD(Instance& instance,
                                                           Solution sol) {
        vector<int> mark(instance.nOrder + 1, 0);
        for (pair<int, int> x : sol.temporatorPdPairs)
            mark[x.first] = 1;
        vector<int> ret_first;
        for (int i = 1; i <= instance.nOrder; i++)
            if (mark[i] == 0)
                ret_first.push_back(i);
        vector<int> ret_second(instance.nOrder + 1, 0);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++)
            for (int x : sol.tours[truckId])
                if (x <= instance.nOrder)
                    ret_second[x] = truckId;
        return make_pair(ret_first, ret_second);
    }

    static pair<int, pair<int, int>> AGES_EJECT(Solution& sol,
                                                Instance& instance,
                                                int k,
                                                int truckID,
                                                vector<int>& p,
                                                pair<int, int> pd_pair) {
        pair<int, pair<int, int>> tmp;
        vector<int>& tour = sol.tours[truckID];
        vector<int> backupTour = tour;
        int bestScore = __INT_MAX__;
        int bestU = 0;
        int bestV = 0;
        if (k == 1) {
            for (int i = 0; i < tour.size(); i++) {
                if (instance.hubList[tour[i]].type != instance.PICK)
                    continue;
                int j = i + 1;
                for (; j < tour.size(); j++)
                    if (tour[j] - tour[i] == instance.nOrder)
                        break;
                tour.erase(tour.begin() + j);
                tour.erase(tour.begin() + i);
                tmp = Solver::SlowInsert(sol, truckID, pd_pair, instance);
                // reverse tour before continue :)))
                tour = backupTour;
                if (tmp.first == __INT_MAX__)
                    continue;
                if (p[tour[i]] < bestScore) {
                    bestScore = p[tour[i]];
                    bestU = tour[i];
                }
            }
        } else if (k == 2) {
            vector<int> pd_list;
            for (int i = 0; i < tour.size(); i++)
                if (instance.hubList[tour[i]].type == instance.PICK)
                    pd_list.push_back(tour[i]);
            for (int i = 0; i < pd_list.size(); i++)
                for (int j = i + 1; j < pd_list.size(); j++) {
                    tour.clear();
                    int ej1 = pd_list[i];
                    int ej2 = pd_list[i] + instance.nOrder;
                    int ej3 = pd_list[j];
                    int ej4 = pd_list[j] + instance.nOrder;
                    for (int u : backupTour)
                        if (u != ej1 && u != ej2 && u != ej3 && u != ej4)
                            tour.push_back(u);
                    tmp = Solver::SlowInsert(sol, truckID, pd_pair, instance);
                    if (tmp.first == __INT_MAX__)
                        continue;
                    if (p[ej1] + p[ej3] < bestScore) {
                        bestScore = p[ej1] + p[ej3];
                        if (bestScore < 0) {
                            int hihihi = 1;
                        }
                        bestU = ej1;
                        bestV = ej3;
                    }
                }
        }
        tour = backupTour;
        return {bestScore, {bestU, bestV}};
    }

    /*
    @brief run AGES for Solution sol
    @return a Solution
    */
    static Solution run(Solution sol,
                        Instance& instance,
                        int perturbIter = 1000,
                        double pEx = 0.5,
                        int maxK = 2) {
        // create variable
        pair<int, pair<int, int>> tmp, saveCost;

        // select and remove a route randomly from sol
        int truckRemoved = ceil(Helper::random() * instance.nTruck);
        int bestTemporatorSize = sol.temporatorPdPairs.size();
        for (int u : sol.tours[truckRemoved]) {
            if (u <= instance.nOrder)
                sol.temporatorPdPairs.push_back({u, u + instance.nOrder});
        }
        sol.tours[truckRemoved].clear();
        // initialize EP with the requests in the removed route
        vector<pair<int, int>>& EP = sol.temporatorPdPairs;
        // initialize all penalty counters p[h] = 1 (h = 1...instance.nOrder)
        vector<int> p(instance.nOrder + 1, 1);
        // while (EP != NULL or termination condition is not meet do)
        Solution bestSol = sol;
        int iter = 1;
        int bestEPSize = EP.size();
        while (EP.size() && iter <= 1000) {
            // select and remove request h_in (pd_pair) from EP with LIFO
            // strategy
            int oldSize = EP.size();
            pair<int, int> pd_pair = EP.back();
            vector<int> N_fe_insert;
            for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
                if (truckId != truckRemoved) {
                    tmp = Solver::SlowInsert(sol, truckId, pd_pair, instance);
                    if (tmp.first != __INT_MAX__)
                        N_fe_insert.push_back(truckId);
                }
            }
            // if N_fe_insert(h_in,sigma) != null
            if (N_fe_insert.size()) {
                int selectedTruck =
                    N_fe_insert[Helper::random() * N_fe_insert.size()];
                tmp = Solver::SlowInsert(sol, selectedTruck, pd_pair, instance);

                // Select sigma' \in N_fe_insert randomly; update sigma = sigma'
                EP.pop_back();
                Solver::insertToTour(sol.tours[selectedTruck], pd_pair,
                                     tmp.second);
                if (sol.isValid(instance) == false) {
                    int z = 1;
                }
            } else {
                // if h_in cannot be inserted in sigma
                // set p[h_in] = p[h_in] + 1
                p[pd_pair.first]++;
                // select sigma' in N_fe_EJ(h_in, sigma) such that Psum is
                // minimized
                saveCost = {__INT_MAX__, {0, 0}};
                int bestTruck, bestK;

                for (int k = 1; k <= maxK; k++) {
                    for (int truckId = 1; truckId <= instance.nTruck; truckId++)
                        if (truckId != truckRemoved) {
                            tmp = AGES::AGES_EJECT(sol, instance, k, truckId, p,
                                                   pd_pair);
                            if (tmp < saveCost) {
                                saveCost = tmp;
                                bestK = k;
                                bestTruck = truckId;
                            }
                        }
                }
                if (saveCost.first < __INT_MAX__) {
                    EP.pop_back();
                    int u = saveCost.second.first;
                    int v = saveCost.second.second;
                    pair<int, int> pdu = {u, u + instance.nOrder};
                    pair<int, int> pdv = {v, v + instance.nOrder};
                    // update sigma = sigma'
                    // add the ejected requestts to EP
                    vector<int>& tour = sol.tours[bestTruck];
                    Solver::removeFromTour(tour, pdu);
                    EP.push_back(pdu);

                    if (bestK == 2) {
                        Solver::removeFromTour(tour, pdv);
                        if (tour.size() % 2) {
                            int z = 3;
                        }
                        EP.push_back(pdv);
                    }
                    tmp = Solver::SlowInsert(sol, bestTruck, pd_pair, instance);
                    Solver::insertToTour(tour, pd_pair, tmp.second);
                    if (sol.isValid(instance) == false) {
                        int z = 1;
                    }
                }
                // sigma = PERTURB(sigma)
                pair<vector<int>, vector<int>> infomation =
                    AGES::AGES_INSERTED_PD(instance, sol);
                vector<int>& insertedList = infomation.first;
                vector<int>& insertedPos = infomation.second;
                for (int i = 1; i <= perturbIter; i++) {
                    double Rand = Helper::random();
                    int r_id1 = Helper::random_int(0, insertedList.size() - 1);
                    int r_id2 = Helper::random_int(0, r_id1);
                    if (Rand < pEx) {
                        int id1 = insertedList[r_id1];
                        int id2 = insertedList[r_id2];
                        if (id1 == id2)
                            continue;
                        bool flag = Perturb::pd_swap(sol, instance, id1, id2,
                                                     insertedPos, true);
                        if (flag)
                            swap(insertedPos[id1], insertedPos[id2]);
                        if (sol.isValid(instance) == false) {
                            int z = 1;
                        }
                    }
                }
            }
            if (EP.size() < bestEPSize) {
                bestSol = sol;
                bestEPSize = EP.size();
            }
            ++iter;
        }
        Solver::ConstructTruck(instance, bestSol, truckRemoved);
        return bestSol;
    }
};

#endif
