#ifndef _PERTURB_HPP_
#define _PERTURB_HPP_

#ifndef _SUBMIT_
#include "Instance.hpp"
#include "Solution.hpp"
#include "Solver.hpp"
#endif

class Perturb {
   public:
    static bool pd_swap(Solution& sol,
                        Instance& instance,
                        int pd_id1,
                        int pd_id2,
                        vector<int>& tourOfPd,
                        bool doSwap = false) {
        pair<int, int> pd_pair1 = {pd_id1, pd_id1 + instance.nOrder};
        pair<int, int> pd_pair2 = {pd_id2, pd_id2 + instance.nOrder};
        vector<int>& tour1 = sol.tours[tourOfPd[pd_id1]];
        vector<int>& tour2 = sol.tours[tourOfPd[pd_id2]];
        vector<int> pos1, pos2;
        for (int i = 0; i < tour1.size(); i++)
            if (tour1[i] == pd_pair1.first || tour1[i] == pd_pair1.second)
                pos1.push_back(i);
        for (int i = 0; i < tour2.size(); i++)
            if (tour2[i] == pd_pair2.first || tour2[i] == pd_pair2.second)
                pos2.push_back(i);
        for (int i = 0; i <= 1; i++)
            swap(tour1[pos1[i]], tour2[pos2[i]]);

        bool ret = sol.isValidTruck(tourOfPd[pd_id1], instance) &&
                   sol.isValidTruck(tourOfPd[pd_id2], instance);

        if (doSwap == false || ret == false)
            for (int i = 0; i <= 1; i++)
                swap(tour1[pos1[i]], tour2[pos2[i]]);
        return ret;
    }
    static bool pd_exchange(Solution& sol,
                            Instance& instance,
                            int pd_id,
                            int newTour_id,
                            vector<int>& tourOfPd,
                            bool doExc = false) {
        // specific pd_pair
        pair<int, int> pd_pair = {pd_id, pd_id + instance.nOrder};
        vector<int>& curTour = sol.tours[tourOfPd[pd_id]];
        vector<int> backup_cur = curTour;
        // first: remove pd_pair from curTour
        for (int i = curTour.size() - 1; i >= 0; i--) {
            if (curTour[i] == pd_pair.first || curTour[i] == pd_pair.second)
                curTour.erase(curTour.begin() + i);
        }
        // TryInsert to newTour
        pair<int, pair<int, int>> tmp =
            Solver::TryInsertN2(sol, newTour_id, pd_pair, instance);
        if (tmp.first == __INT_MAX__ || doExc == false) {
            curTour = backup_cur;
            return (tmp.first != __INT_MAX__);
        }
        // must do exchange
        // can be same
        vector<int>& newTour = sol.tours[newTour_id];
        newTour.insert(newTour.begin() + tmp.second.first, pd_pair.first);
        newTour.insert(newTour.begin() + tmp.second.second, pd_pair.second);
        tourOfPd[pd_id] = newTour_id;
        return true;
    }
};

#endif
