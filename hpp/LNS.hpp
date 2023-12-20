#ifndef _LNS_HPP_
#define _LNS_HPP_

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
                cost.push_back(
                    sol.costRoute(truckId, instance, sol.tours[truckId]));
                vector<int> backup = sol.tours[truckId];
                vector<int>& tour = sol.tours[truckId];
                for (int i = tour.size() - 1; i >= 0; i--)
                    if (tour[i] == u || tour[i] == u + instance.nOrder)
                        tour.erase(tour.begin() + i);
                cost.back() -=
                    sol.costRoute(truckId, instance, sol.tours[truckId]);
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

    /*
    @brief run LNS for a soluion sol
    */
    static Solution run(Solution sol,
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
                        int q = 5,
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
                numberAttemp[type]++;
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
                        pair<int, pair<int, int>> tmp = Solver::SlowInsert(
                            s_, truckId, {D[i], D[i] + instance.nOrder},
                            instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        Solver::insertToTour(
                            tour, {D[i], D[i] + instance.nOrder}, best.second);
                    } else
                        newUnused.push_back({D[i], D[i] + instance.nOrder});
                }

                // try to insert more
                shuffle(unUsed.begin(), unUsed.end(), std::mt19937(42));
                int nTry = maxTry;
                // ton chi phi tinh toan
                for (pair<int, int> pd_pair : unUsed) {
                    nTry--;
                    // giam chi phi tinh toan
                    if (nTry < 0 && iTer != maxIter) {
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
                            Solver::SlowInsert(s_, truckId, pd_pair, instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        Solver::insertToTour(tour, pd_pair, best.second);
                    } else
                        newUnused.push_back(pd_pair);
                }
                unUsed = newUnused;

                if (s_.objective(instance) > sbest.objective(instance)) {
                    sbest = s_;
                    score[type] += o1;
                    sol = s_;  // accept
                } else {
                    // simulated annealing criate
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

    /*
    @brief run LNS with LAHC for a soluion sol
    */
    static Solution run_LAHC(Solution sol,
                             Instance& instance,
                             int maxSeg = 30,
                             int maxIter = 30,
                             int o1 = 33,
                             int o2 = 9,
                             int o3 = 13,
                             double r = 0.1,
                             double p = 6,
                             double pWorst = 3,
                             int q = 10,
                             int LHC_LEN = 100,
                             int maxTry = 10,
                             int scaler = 1000000) {
        Solution sbest = sol;
        int bestObj = sbest.objective(instance);
        vector<int> C(LHC_LEN, bestObj);
        vector<double> w = {1, 1, 1};
        int x = 0;
        int curObj = bestObj;
        for (int iSeg = 1; iSeg <= maxSeg; iSeg++) {
            vector<int> score(3, 0);
            vector<int> numberAttemp(3, 0);
            for (int iTer = 1; iTer <= maxIter; iTer++) {
                Solution s_ = sol;
                vector<pair<int, int>>& unUsed = s_.temporatorPdPairs;
                // remove q request
                int type = Helper::selectWithWeight(w);
                numberAttemp[type]++;
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
                        pair<int, pair<int, int>> tmp = Solver::SlowInsert(
                            s_, truckId, {D[i], D[i] + instance.nOrder},
                            instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        Solver::insertToTour(
                            tour, {D[i], D[i] + instance.nOrder}, best.second);
                    } else
                        newUnused.push_back({D[i], D[i] + instance.nOrder});
                }

                // try to insert more
                shuffle(unUsed.begin(), unUsed.end(), std::mt19937(42));
                int nTry = maxTry;
                // ton chi phi tinh toan
                for (pair<int, int> pd_pair : unUsed) {
                    nTry--;
                    // giam chi phi tinh toan
                    if (nTry < 0 && iTer != maxIter) {
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
                            Solver::SlowInsert(s_, truckId, pd_pair, instance);
                        if (tmp < best) {
                            best = tmp;
                            bestTruck = truckId;
                        }
                    }
                    if (best.first != __INT_MAX__) {
                        vector<int>& tour = s_.tours[bestTruck];
                        Solver::insertToTour(tour, pd_pair, best.second);
                    } else
                        newUnused.push_back(pd_pair);
                }
                unUsed = newUnused;
                int newObj = s_.objective(instance);
                if (newObj > bestObj) {
                    score[type] += o1;
                    curObj = bestObj = newObj;
                    sol = sbest = s_;

                } else if (newObj >= C[x] || newObj >= curObj) {
                    sol = s_;
                    curObj = newObj;
                    if (newObj >= curObj)
                        score[type] += o2;
                    else
                        score[type] += o3;
                }
                x = (x + 1) % LHC_LEN;
            }
            for (int i = 0; i < 3; i++)
                w[i] = w[i] * (1 - r) + r * score[i] / numberAttemp[i];
        }
        return sbest;
    }
};

#endif
