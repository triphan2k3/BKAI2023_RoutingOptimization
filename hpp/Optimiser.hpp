#ifndef _OPTIMISER_HPP_
#define _OPTIMISER_HPP_

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
                pair<int, pair<int, int>> tmp =
                    Solver::TryInsertN2(sol, truckID, pd_pair, instance);
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
                    pair<int, pair<int, int>> tmp =
                        Solver::TryInsertN2(sol, truckID, pd_pair, instance);
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

    static Solution AGES_RUN(Solution sol,
                             Instance& instance,
                             int perturbIter = 1000,
                             double pEx = 0.5) {
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
                    pair<int, pair<int, int>> tmp =
                        Solver::TryInsertN2(sol, truckId, pd_pair, instance);
                    if (tmp.first != __INT_MAX__)
                        N_fe_insert.push_back(truckId);
                }
            }
            // if N_fe_insert(h_in,sigma) != null
            if (N_fe_insert.size()) {
                int selectedTruck =
                    N_fe_insert[Helper::random() * N_fe_insert.size()];
                pair<int, pair<int, int>> tmp =
                    Solver::TryInsertN2(sol, selectedTruck, pd_pair, instance);

                // Select sigma' \in N_fe_insert randomly; update sigma = sigma'
                EP.pop_back();
                vector<int>& tour = sol.tours[selectedTruck];
                tour.insert(tour.begin() + tmp.second.first, pd_pair.first);
                tour.insert(tour.begin() + tmp.second.second, pd_pair.second);
            } else {
                // if h_in cannot be inserted in sigma
                // set p[h_in] = p[h_in] + 1
                p[pd_pair.first]++;
                // select sigma' in N_fe_EJ(h_in, sigma) such that Psum is
                // minimized
                pair<int, pair<int, int>> saveCost = {__INT_MAX__, {0, 0}};
                int bestTruck, bestK;

                for (int k = 1; k <= 2; k++) {
                    for (int truckId = 1; truckId <= instance.nTruck; truckId++)
                        if (truckId != truckRemoved) {
                            if ((k == 2 && Helper::random() < 1) || (k == 1)) {
                                pair<int, pair<int, int>> tmp =
                                    AGES::AGES_EJECT(sol, instance, k, truckId,
                                                     p, pd_pair);
                                if (tmp < saveCost) {
                                    saveCost = tmp;
                                    bestK = k;
                                    bestTruck = truckId;
                                }
                            }
                        }
                }
                if (saveCost.first < __INT_MAX__) {
                    EP.pop_back();
                    vector<int>& tour = sol.tours[bestTruck];
                    int bestU = saveCost.second.first;
                    int bestV = saveCost.second.second;
                    // update sigma = sigma'
                    // add the ejected requestts to EP
                    for (int i = tour.size() - 1; i >= 0; i--)
                        if (tour[i] == bestU ||
                            tour[i] == bestU + instance.nOrder)
                            tour.erase(tour.begin() + i);
                    EP.push_back({bestU, bestU + instance.nOrder});
                    if (bestK == 2) {
                        for (int i = tour.size() - 1; i >= 0; i--)
                            if (tour[i] == bestV ||
                                tour[i] == bestV + instance.nOrder)
                                tour.erase(tour.begin() + i);
                        EP.push_back({bestV, bestV + instance.nOrder});
                    }
                    pair<int, pair<int, int>> tmp =
                        Solver::TryInsertN2(sol, bestTruck, pd_pair, instance);
                    tour.insert(tour.begin() + tmp.second.first, pd_pair.first);
                    tour.insert(tour.begin() + tmp.second.second,
                                pd_pair.second);
                }
                // sigma = PERTURB(sigma)
                pair<vector<int>, vector<int>> infomation =
                    AGES::AGES_INSERTED_PD(instance, sol);
                vector<int>& insertedList = infomation.first;
                vector<int>& insertedPos = infomation.second;
                for (int i = 1; i <= perturbIter; i++) {
                    double Rand = Helper::random();
                    if (Rand < pEx) {
                        int r_id1 =
                            Helper::random_int(0, insertedList.size() - 1);
                        int r_id2 = Helper::random_int(0, r_id1);
                        int id1 = insertedList[r_id1];
                        int id2 = insertedList[r_id2];
                        if (id1 == id2)
                            continue;
                        bool flag = Perturb::pd_swap(sol, instance, id1, id2,
                                                     insertedPos, true);
                        if (flag)
                            swap(insertedPos[id1], insertedPos[id2]);
                    }
                }
            }
            if (EP.size() < bestEPSize) {
                bestSol = sol;
                bestEPSize = EP.size();
            }
            ++iter;
        }
        Solver::TruckRouteConstructionHeuristics(instance, bestSol,
                                                 truckRemoved);
        return bestSol;
    }
};

class LNS {
   public:
    static vector<pair<double, int>> Related(Solution& sol,
                                             Instance& instance,
                                             vector<int>& L,
                                             vector<int>& AT,
                                             int r,
                                             double phi = 9,
                                             double chi = 3,
                                             double psi = 2,
                                             double omege = 5) {
        double maxct = instance.maxDist;
        // related, u
        vector<int> isInL(instance.nOrder + 1, 0);
        for (int u : L)
            isInL[u] = 1;
        vector<pair<double, int>> related;
        int aj = r, bj = r + instance.nOrder;
        for (int u : L) {
            int ai = u, bi = u + instance.nOrder;
            int d_ = instance.moveDist(ai, aj) + instance.moveDist(bi, bj);
            int t_ = abs(AT[ai] - AT[aj]) + abs(AT[bi] - AT[bj]);
            int l_ = abs(
                instance.orderList[ai].weight + instance.orderList[ai].volume -
                instance.orderList[bi].weight + instance.orderList[bi].volume);
            related.push_back({phi * d_ + chi * t_ + psi * l_, u});
        }
        return related;
    }

    static vector<int> CalculateArriveTime(Solution& sol, Instance& instance) {
        vector<int> visitTime(instance.nOrder * 2 + 2, __INT_MAX__);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            Hub& startHub = instance.hubList[sol.start[truckId]];
            Hub preHub = startHub;
            int curTime = preHub.timeWindow.first;
            for (int u : sol.tours[truckId]) {
                Hub& hub = instance.hubList[u];
                curTime = curTime + instance.moveTime(preHub, hub, truckId);
                curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
                visitTime[u] = curTime;
                preHub = hub;
            }
        }
        return visitTime;
    }

    static vector<int> SHAW_REMOVAL(Solution& sol,
                                    int q,
                                    double p,
                                    Instance& instance) {
        // Array : L = an array containing all request from s not in D;
        vector<int> L;
        for (int truckId = 1; truckId <= instance.nTruck; truckId++)
            for (int u : sol.tours[truckId])
                if (u <= instance.nOrder)
                    L.push_back(u);
        // request : r = a randomly selected request from S;
        int r_id = Helper::random() * L.size();
        // set of requests will be deleted : D = {r};
        vector<int> D = {L[r_id]};
        L.erase(L.begin() + r_id);

        vector<int> AT = LNS::CalculateArriveTime(sol, instance);

        // while |D| < q do
        while (D.size() < q) {
            // r = a randomly selected request from D;
            int r = D[Helper::random() * D.size()];
            // Array : L = an array containing all request from s not in D;
            // sort L such that i < j <=> R(r,L[i]) < R(r,L[j]);
            vector<pair<double, int>> related =
                LNS::Related(sol, instance, L, AT, r);
            sort(related.begin(), related.end());

            // choose a random number y from the interval[0,1);
            double y = Helper::random();
            int L_id = pow(y, p) * L.size();
            // D = D \union {L[y^p |L|]}
            D.push_back(L[L_id]);
            L.erase(L.begin() + L_id);
            // End While
        }
        // remove the requests in D from s;
        vector<int> mark(instance.nOrder * 2 + 1, 0);
        for (int u : D)
            mark[u] = mark[u + instance.nOrder] = 1;
        for (vector<int>& tour : sol.tours) {
            vector<int> tempTour;
            for (int u : tour)
                if (mark[u] == 0)
                    tempTour.push_back(u);
            tour = tempTour;
        }
        return D;
    }

    static vector<int> WORST_REMOVAL(Solution& sol,
                                     int q,
                                     double p,
                                     Instance& instance) {
        vector<int> L;
        vector<int> tourId(instance.nOrder + 1, 0);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++)
            for (int u : sol.tours[truckId])
                if (u <= instance.nOrder) {
                    L.push_back(u);
                    tourId[u] = truckId;
                }
        vector<int> D;
        while (D.size() < q) {
            vector<int> cost;
            for (int u : L) {
                int truckId = tourId[u];
                cost.push_back(sol.calculatePen(truckId, instance));
                vector<int> backup = sol.tours[truckId];
                vector<int>& tour = sol.tours[truckId];
                for (int i = tour.size() - 1; i >= 0; i--)
                    if (tour[i] == u || tour[i] == u + instance.nOrder)
                        tour.erase(tour.begin() + i);
                cost.back() -= sol.calculatePen(truckId, instance);
                tour = backup;
            }
            int minId = 0;
            for (int i = 1; i < L.size(); i++)
                if (cost[i] < cost[minId])
                    minId = i;
            D.push_back(L[minId]);
            L.erase(L.begin() + minId);
        }
        // remove the requests in D from s;
        vector<int> mark(instance.nOrder * 2 + 1, 0);
        for (int u : D)
            mark[u] = mark[u + instance.nOrder] = 1;

        for (vector<int>& tour : sol.tours) {
            vector<int> tempTour;
            for (int u : tour)
                if (mark[u] == 0)
                    tempTour.push_back(u);
            tour = tempTour;
        }
        return D;
    }

    static vector<int> RANDOM_REMOVAL(Solution& sol,
                                      int q,
                                      double p,
                                      Instance& instance) {
        vector<int> L;
        for (int truckId = 1; truckId <= instance.nTruck; truckId++)
            for (int u : sol.tours[truckId])
                if (u <= instance.nOrder)
                    L.push_back(u);
        vector<int> D;
        while (D.size() < q) {
            int r_id = Helper::random() * L.size();
            D.push_back(L[r_id]);
            L.erase(L.begin() + r_id);
        }
        // remove the requests in D from s;
        vector<int> mark(instance.nOrder * 2 + 1, 0);
        for (int u : D)
            mark[u] = mark[u + instance.nOrder] = 1;
        for (vector<int>& tour : sol.tours) {
            vector<int> tempTour;
            for (int u : tour)
                if (mark[u] == 0)
                    tempTour.push_back(u);
            tour = tempTour;
        }
        return D;
    }

    static Solution LNS_RUN(Solution sol,
                            Instance& instance,
                            int maxSeg = 15,
                            int maxIter = 15,
                            int o1 = 33,
                            int o2 = 9,
                            int o3 = 13,
                            double r = 0.1,
                            double p = 6,
                            double pWorst = 3,
                            double w_ = 0.05,
                            double c_ = 0.99975,
                            int q = 10,
                            int maxTry = 10,
                            int scaler = 1000000) {
        /*
            e^-y = 0.5
            y = 0.69314718
            (f - (1-w)f)/T = 0.69314718
            wf / T = 0.69314718
            T = 1 / (0.69314718/w/f)
            f = fitness / scaler
        */
        Solution sbest = sol;
        double T =
            1.0 / (0.69314718 / w_ / (sbest.objective(instance) / scaler));
        vector<double> w = {1, 1, 1};
        for (int iSeg = 1; iSeg <= maxSeg; iSeg++) {
            vector<int> score(3, 0);
            vector<int> numberAttemp(3, 0);
            for (int iTer = 1; iTer <= maxIter; iTer++) {
                Solution s_ = sol;
                vector<pair<int, int>>& unUsed = s_.temporatorPdPairs;
                // remove q request
                int type = Helper::selectWithWeight(w);
                vector<int> D;
                if (type == 0)
                    D = SHAW_REMOVAL(s_, q, p, instance);
                else if (type == 1)
                    D = WORST_REMOVAL(s_, q, pWorst, instance);
                else if (type == 2)
                    D = RANDOM_REMOVAL(s_, q, p, instance);
                else {
                    cout << "sai o dau do\n";
                    exit(1);
                }
                // reinsert at most q request
                shuffle(D.begin(), D.end(), std::mt19937(42));
                vector<pair<int, int>> newUnused;
                for (int i = D.size() - 1; i >= 0; i--) {
                    pair<int, pair<int, int>> best = {__INT_MAX__, {0, 0}};
                    int bestTruck = 0;
                    for (int truckId = 1; truckId <= instance.nTruck;
                         truckId++) {
                        pair<int, pair<int, int>> tmp = Solver::TryInsertN2(
                            s_, truckId, {D[i], D[i] + instance.nOrder},
                            instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        tour.insert(tour.begin() + best.second.first, D[i]);
                        tour.insert(tour.begin() + best.second.second,
                                    D[i] + instance.nOrder);
                    } else
                        newUnused.push_back({D[i], D[i] + instance.nOrder});
                }

                // try to insert more
                shuffle(unUsed.begin(), unUsed.end(), std::mt19937(42));
                int nTry = maxTry;
                for (pair<int, int> pd_pair :
                     unUsed) {  // ton chi phi tinh toan
                    nTry--;
                    if (nTry < 0 &&
                        iTer != maxIter) {  // giam chi phi tinh toan
                        newUnused.push_back(pd_pair);
                        continue;
                    }
                    int flag = 0;
                    for (int& x : D)
                        if (x == pd_pair.first)
                            flag = 1;
                    if (flag)
                        continue;
                    pair<int, pair<int, int>> best = {__INT_MAX__, {0, 0}};
                    int bestTruck = 0;
                    for (int truckId = 1; truckId <= instance.nTruck;
                         truckId++) {
                        pair<int, pair<int, int>> tmp =
                            Solver::TryInsertN2(s_, truckId, pd_pair, instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        tour.insert(tour.begin() + best.second.first,
                                    pd_pair.first);
                        tour.insert(tour.begin() + best.second.second,
                                    pd_pair.second);
                    } else
                        newUnused.push_back(pd_pair);
                }
                unUsed = newUnused;

                if (s_.objective(instance) > sbest.objective(instance)) {
                    sbest = s_;
                    score[type] += o1;
                    sol = s_;  // accept
                } else {
                    int accept =
                        (s_.objective(instance) - sol.objective(instance)) /
                        scaler;
                    if (accept > 0) {
                        score[type] += o2;
                        sol = s_;
                    } else {
                        double Rand = Helper::random();
                        double prob = exp(-accept / T);
                        if (prob >= Rand) {
                            sol = s_;
                            score[type] += o3;
                        }
                    }
                }
                T = T * c_;
            }
            for (int i = 0; i < 3; i++)
                w[i] = w[i] * (1 - r) + r * score[i] / numberAttemp[i];
        }
        return sbest;
    }
};

#endif
