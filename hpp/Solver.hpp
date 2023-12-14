#ifndef _SOLVER_HPP_
#define _SOLVER_HPP_

#ifndef _SUBMIT_
#include "Instance.hpp"
#include "Solution.hpp"
#endif

class Solver {
   public:
    /*
    @brief remove pd_pair from truck[truckId] of Solution sol
    */
    static void removeFromTour(vector<int> &tour,
                               pair<int, int> pd_pair) {
        int prev = 0;
        for (int i = 0; i < tour.size(); i++) {
            if (tour[i] != pd_pair.first && tour[i] != pd_pair.second) {
                tour[prev] = tour[i];
                ++prev;
            }
        }
        tour.pop_back();
        tour.pop_back();
        // for (int i = tour.size() - 1; i >= 0; i--)
        //     if (tour[i] == pd_pair.first || tour[i] == pd_pair.second)
        //         tour.erase(tour.begin() + i);
    }

    /*
    @brief insert pd_pair to truck[truckId] of Solution sol where after insert
    pickup node and delivery node have position is pos_pair. It's mean I insert
    pickup node first
    */
    static void insertToTour(vector<int>& tour,
                             pair<int, int> pd_pair,
                             pair<int, int> pos_pair) {
        tour.push_back(pd_pair.first);
        for (int i = tour.size() - 1; i > pos_pair.first; i--)
            swap(tour[i], tour[i - 1]);

        tour.push_back(pd_pair.second);
        for (int i = tour.size() - 1; i > pos_pair.second; i--)
            swap(tour[i], tour[i - 1]);

        // tour.insert(tour.begin() + pos_pair.first, pd_pair.first);
        // tour.insert(tour.begin() + pos_pair.second, pd_pair.second);
        // sol.tourIdOfPdPair[pd_pair.first] = truckId;
    }

    /*
    @brief Find the best way to insert pd_pair to truck[truckId] which delivery
    node is inserted at the end of tour
    @return first: localPen, second: localPos
    */
    static pair<int, pair<int, int>> FastInsert(Solution& sol,
                                                int truckId,
                                                pair<int, int> pd_pair,
                                                Instance& instance) {
        vector<int> tour = sol.tours[truckId];
        tour.push_back(pd_pair.first);
        tour.push_back(pd_pair.second);
        int localPen = sol.costRoute(truckId, instance, tour);
        int localPos = tour.size() - 2;
        for (int i = tour.size() - 2; i >= 1; i--) {
            swap(tour[i], tour[i - 1]);
            int thisPen = sol.costRoute(truckId, instance, tour);
            if (thisPen < localPen &&
                sol.isValidTruck(truckId, instance, tour)) {
                localPen = thisPen;
                localPos = i - 1;
            }
        }
        return {localPen, {localPos, sol.tours[truckId].size() + 1}};
    }

    /*
    @brief Find the best way to insert pd_pair to truck[truckId] which delivery
    node is inserted at the end of tour. The position returned must be insert by
    order first -> second
    @return first: bestPen, second: bestPosition
    */
    static pair<int, pair<int, int>> SlowInsert(Solution& sol,
                                                int truckId,
                                                pair<int, int> pd_pair,
                                                Instance& instance) {
        vector<int> tour = sol.tours[truckId];
        vector<int> backupTour = tour;
        tour.push_back(pd_pair.first);
        tour.push_back(pd_pair.second);
        int bestPen = sol.costRoute(truckId, instance, tour);
        int bestI = tour.size() - 2;
        int bestJ = tour.size() - 1;
        for (int i = backupTour.size() - 1; i >= 0; i--) {
            tour.pop_back();
            swap(tour[i + 1], tour[i]);
            tour.insert(tour.begin() + i + 1, pd_pair.second);
            int tmp = sol.costRoute(truckId, instance, tour);
            if (tmp < bestPen && sol.isValidTruck(truckId, instance, tour)) {
                bestPen = tmp;
                bestI = i;
                bestJ = i + 1;
            }
            for (int j = i + 2; j <= backupTour.size() + 1; j++) {
                swap(tour[j], tour[j - 1]);
                int tmp = sol.costRoute(truckId, instance, tour);
                if (tmp < bestPen &&
                    sol.isValidTruck(truckId, instance, tour)) {
                    bestPen = tmp;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        return {bestPen, {bestI, bestJ}};
    }

    /*
    @brief construct truck[truckId] of Solution sol with unused pd_pairs of sol
    */
    static void ConstructTruck(Instance& instance, Solution& sol, int truckId) {
        pair<int, pair<int, int>> tmp;
        pair<int, int> bestPD, pos_pair, _pos_pair;
        int min_cost, _min_cost;

        vector<pair<int, int>>& pd_pairs = sol.temporatorPdPairs;
        vector<int>& tour = sol.tours[truckId];
        while (true) {
            min_cost = __INT_MAX__;
            for (pair<int, int> pd_pair : pd_pairs) {
                tmp = Solver::FastInsert(sol, truckId, pd_pair, instance);
                _min_cost = tmp.first;
                _pos_pair = tmp.second;
                if (_min_cost < min_cost) {
                    min_cost = _min_cost;
                    pos_pair = _pos_pair;
                    bestPD = pd_pair;
                }
            }
            if (min_cost == __INT_MAX__)
                break;
            Solver::insertToTour(tour, bestPD, pos_pair);
            for (auto it = pd_pairs.begin();; it++) {
                if (*it == bestPD) {
                    pd_pairs.erase(it);
                    break;
                }
            }
        }
    }

    /*
    @brief construct a solution with truckOrder is 1, 2, 3,...
    @return Solution
    */
    static Solution ConstructSolution(Instance& instance) {
        Solution sol(instance);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            ConstructTruck(instance, sol, truckId);
        }
        return sol;
    }

    /*
    @brief construct a solution with truck order is orderId
    @return Solution
    */
    static Solution ConstructSolution(Instance& instance,
                                      vector<int>& orderId) {
        Solution sol(instance);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            ConstructTruck(instance, sol, orderId[truckId - 1]);
        }
        return sol;
    }
};

#endif
