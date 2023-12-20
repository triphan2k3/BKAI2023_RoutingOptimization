#ifndef _PERTURB_HPP_
#define _PERTURB_HPP_

#ifndef _SUBMIT_
#include "Instance.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#endif

#define vivi pair<vector<int>, vector<int>>
#define vi vector<int>
class Perturb {
   public:
    /*
    @brief get list of inserted pd-pair from solution sol and its truckId
    @return a pair<list_insertedRequest, list_truckIdOfRequest
    */
    static vivi get_inserted_pairs(Instance& instance, Solution sol) {
        vector<int> mark(instance.nOrder + 1, 0);
        // mark non-inserted request
        for (pair<int, int> x : sol.temporatorPdPairs)
            mark[x.first] = 1;

        // ret_first contain all inserted request
        vector<int> ret_first;
        for (int i = 1; i <= instance.nOrder; i++)
            if (mark[i] == 0)
                ret_first.push_back(i);

        // ret_second contain truckId of each inserted request
        vector<int> ret_second(instance.nOrder + 1, 0);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++)
            for (int x : sol.tours[truckId])
                if (x <= instance.nOrder)
                    ret_second[x] = truckId;
        return make_pair(ret_first, ret_second);
    }

    /*
    @brief perturbate the solution sol with two operator pair_move and swap_move
    */
    static void do_perturb(Solution& sol,
                           Instance& instance,
                           int numTry,
                           int truckRemoved = 0) {
        vivi infomation = get_inserted_pairs(instance, sol);
        vector<int> pd_pairs = infomation.first;
        vector<int> id_truck = infomation.second;
        static double prob[4] = {0.25, 0.5, 0.75, 1};
        for (int i = 1; i <= numTry; i++) {
            double p = Helper::random();
            if (p < prob[0]) {
                // pair move
                // select random pair
                int p = pd_pairs[Helper::random_int(0, pd_pairs.size() - 1)];
                int d = p + instance.nOrder;
                int truck1 = id_truck[p];
                // select random truck to move above pair
                // TODO: because truckRemoved is empty, so swap move don't need to check
                int truck2 = Helper::random_int(1, instance.nTruck);
                while (truck1 == truck2 || truck2 == truckRemoved)
                    truck2 = Helper::random_int(1, instance.nTruck);
                // find a feasible insert choice
                pair<int, pair<int, int>> tmp =
                    Solver::SlowInsert(sol, truck2, {p, d}, instance, false);
                // if all are infeasible, continue
                if (tmp.first == __INT_MAX__)
                    continue;
                // remove pd from truck1 and insert to truck2
                Solver::removeFromTour(sol.tours[truck1], {p, d});
                Solver::insertToTour(sol.tours[truck2], {p, d}, tmp.second);
                // change information
                id_truck[p] = truck2;
            } else if (p < prob[1]) {
                // swap move
                int truck1 = Helper::random_int(1, instance.nTruck);
                while (sol.tours[truck1].size() == 0) {
                    truck1 = Helper::random_int(1, instance.nTruck);
                }

                int truck2 = Helper::random_int(1, instance.nTruck);

                while (sol.tours[truck2].size() == 0 || truck1 == truck2) {
                    truck2 = Helper::random_int(1, instance.nTruck);
                }

                vector<int>& tour1 = sol.tours[truck1];
                vector<int>& tour2 = sol.tours[truck2];
                if (tour1.size() > 30 || tour2.size() > 30) {
                    int z = 1;
                }
                int p1 = tour1[Helper::random_int(0, tour1.size() - 1)];
                int p2 = tour2[Helper::random_int(0, tour2.size() - 1)];

                p1 = p1 > instance.nOrder ? p1 - instance.nOrder : p1;
                p2 = p2 > instance.nOrder ? p2 - instance.nOrder : p2;
                int d1 = p1 + instance.nOrder;
                int d2 = p2 + instance.nOrder;
                vector<int> backup1 = tour1, backup2 = tour2;

                Solver::removeFromTour(tour1, {p1, d1});
                Solver::removeFromTour(tour2, {p2, d2});
                if (tour1.size() > 30 || tour2.size() > 30) {
                    int z = 1;
                }
                pair<int, pair<int, int>> tmp1 =
                    Solver::SlowInsert(sol, truck1, {p2, d2}, instance, true);
                pair<int, pair<int, int>> tmp2 =
                    Solver::SlowInsert(sol, truck2, {p1, d1}, instance, true);
                if (tmp1.first == __INT_MAX__ || tmp2.first == __INT_MAX__) {
                    tour1 = backup1;
                    tour2 = backup2;
                    continue;
                }
                Solver::insertToTour(tour1, {p2, d2}, tmp1.second);
                Solver::insertToTour(tour2, {p1, d1}, tmp2.second);
                id_truck[p1] = truck2;
                id_truck[p2] = truck1;
                if (sol.isValidTruck(truck1, instance, tour1) == false || sol.isValidTruck(truck2, instance, tour2) == false) {
                    cout << "WHY";
                }
            }
        }
    }
};

#endif
