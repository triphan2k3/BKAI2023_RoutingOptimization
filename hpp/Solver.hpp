#ifndef _SOLVER_HPP_
#define _SOLVER_HPP_

#ifndef _SUBMIT_
#include "Instance.hpp"
#include "Solution.hpp"
#endif

class Solver {
   public:
    // return: first: localPen, second: localPos
    static pair<int, int> TryInsert(Solution& sol,
                                    int truckId,
                                    pair<int, int> pd_pair,
                                    Instance& instance) {
        vector<int>& tour = sol.tours[truckId];
        vector<int> backupTour = tour;
        sol.tours[truckId].push_back(pd_pair.first);
        sol.tours[truckId].push_back(pd_pair.second);
        int localPen = sol.calculatePen(truckId, instance);
        int localPos = tour.size() - 2;
        for (int i = tour.size() - 2; i >= 1; i--) {
            swap(tour[i], tour[i - 1]);
            int thisPen = sol.calculatePen(truckId, instance);
            if (thisPen < localPen && sol.isValidTruck(truckId, instance)) {
                localPen = thisPen;
                localPos = i - 1;
            }
        }
        tour = backupTour;
        return {localPen, localPos};
    }

    // first: bestPen, second: bestPosition
    // must insert first then second in position
    static pair<int, pair<int, int>> TryInsertN2(Solution& sol,
                                                 int truckId,
                                                 pair<int, int> pd_pair,
                                                 Instance& instance) {
        vector<int>& tour = sol.tours[truckId];
        vector<int> backupTour = tour;
        tour.push_back(pd_pair.first);
        tour.push_back(pd_pair.second);
        int bestPen = sol.calculatePen(truckId, instance);
        int bestI = tour.size() - 2;
        int bestJ = tour.size() - 1;
        for (int i = backupTour.size() - 1; i >= 0; i--) {
            tour.pop_back();
            swap(tour[i + 1], tour[i]);
            tour.insert(tour.begin() + i + 1, pd_pair.second);
            int tmp = sol.calculatePen(truckId, instance);
            if (tmp < bestPen && sol.isValidTruck(truckId, instance)) {
                bestPen = tmp;
                bestI = i;
                bestJ = i + 1;
            }
            for (int j = i + 2; j <= backupTour.size() + 1; j++) {
                swap(tour[j], tour[j - 1]);
                int tmp = sol.calculatePen(truckId, instance);
                if (tmp < bestPen && sol.isValidTruck(truckId, instance)) {
                    bestPen = tmp;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        tour = backupTour;
        return {bestPen, {bestI, bestJ}};
    }

    static void TruckRouteConstructionHeuristics(Instance& instance,
                                                 Solution& sol,
                                                 int truckId) {
        vector<pair<int, int>>& pd_pairs = sol.temporatorPdPairs;
        vector<int>& tour = sol.tours[truckId];
        while (true) {
            int globalPen = __INT_MAX__;
            pair<int, int> globalPD;
            int globalPos;
            for (pair<int, int> pd_pair : pd_pairs) {
                // TODO: change TryInsert to TryInsertN2
                pair<int, int> insertValue =
                    Solver::TryInsert(sol, truckId, pd_pair, instance);
                int localPen = insertValue.first;
                int localPos = insertValue.second;
                if (localPen < globalPen) {
                    globalPen = localPen;
                    globalPos = localPos;
                    globalPD = pd_pair;
                }
            }
            if (globalPen == __INT_MAX__)
                break;
            // sol.insertToTour(truckId, globalPD, {globalPos, tour.size() +
            // 1});
            tour.push_back(globalPD.second);
            tour.insert(tour.begin() + globalPos, globalPD.first);
            for (auto it = pd_pairs.begin();; it++)
                if (*it == globalPD) {
                    pd_pairs.erase(it);
                    break;
                }
        }
    }
    static Solution RouteConstructionHeuristics(Instance& instance) {
        Solution sol(instance);
        // TODO: random_shuffle tour
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            TruckRouteConstructionHeuristics(instance, sol, truckId);
        }
        return sol;
    }
    static Solution RouteConstructionHeuristics(Instance& instance,
                                                const vector<int>& orderId) {
        Solution sol(instance);
        // TODO: random_shuffle tour
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            TruckRouteConstructionHeuristics(instance, sol,
                                             orderId[truckId - 1]);
        }
        return sol;
    }

    static void BetterTruckRouteConstructionHeuristics(Instance& instance,
                                                       Solution& sol,
                                                       int truckId) {
        vector<pair<int, int>>& pd_pairs = sol.temporatorPdPairs;
        vector<int>& tour = sol.tours[truckId];
        while (true) {
            int globalPen = __INT_MAX__;
            pair<int, int> globalPD;
            pair<int, int> globalPos;
            for (pair<int, int> pd_pair : pd_pairs) {
                // TODO: change TryInsert to TryInsertN2
                pair<int, pair<int, int>> insertValue =
                    Solver::TryInsertN2(sol, truckId, pd_pair, instance);
                int localPen = insertValue.first;
                pair<int, int> localPos = insertValue.second;
                if (localPen < globalPen) {
                    globalPen = localPen;
                    globalPos = localPos;
                    globalPD = pd_pair;
                }
            }
            if (globalPen == __INT_MAX__)
                break;
            // sol.insertToTour(truckId, globalPD, {globalPos, tour.size() +
            // 1});
            tour.insert(tour.begin() + globalPos.first, globalPD.first);
            tour.insert(tour.begin() + globalPos.second, globalPD.second);
            for (auto it = pd_pairs.begin();; it++)
                if (*it == globalPD) {
                    pd_pairs.erase(it);
                    break;
                }
        }
    }

    static Solution BetterConstructionHeuristics(Instance& instance) {
        Solution sol(instance);
        // TODO: random_shuffle tour
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            BetterTruckRouteConstructionHeuristics(instance, sol, truckId);
        }
        return sol;
    }
};

#endif
