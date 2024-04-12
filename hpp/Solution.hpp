#ifndef _SOLUTION_HPP_
#define _SOLUTION_HPP_

#include <math.h>
#include <algorithm>
#include <vector>

#ifndef _SUBMIT_
#include "Helper.hpp"
#include "Instance.hpp"
#endif

class Solution {
   public:
    vector<vector<int>> tours;
    vector<pair<int, int>> temporatorPdPairs;
    vector<pair<int, int>> insertedPdPairs;
    vector<int> tourIdOfPdPair;
    vector<int> start;
    Solution(Instance& instance) {
        tours.resize(instance.nTruck + 1);
        start.resize(instance.nTruck + 1);
        tourIdOfPdPair.resize(instance.nTruck + 1);
        for (int i = 1; i <= instance.nTruck; i++)
            start[i] = 2 * instance.nOrder + i;
        for (int i = 1; i <= instance.nOrder; i++)
            temporatorPdPairs.push_back({i, i + instance.nOrder});
    }

    // void debug(Instance instance) {
    //     for (int i = 1; i <= instance.nTruck; i++) {
    //         cout << start[i] << "\n";
    //         for (int x : tours[i])
    //             cout << x << " ";
    //         cout << endl;
    //     }
    // }

    /*
    @brief return cost of truck[truckId] as the earliest time truck can leave
    last node, note that this function cannot handle duplicate pd-pairs
    @return this time, or __INT_MAX__ if invalid route
    */
    int costRoute(int truckId, Instance& instance, vector<int>& tour) {
        Hub& startHub = instance.hubList[start[truckId]];
        Hub preHub = startHub;
        int curTime = preHub.timeWindow.first;
        double maxWeigh = instance.truckList[truckId].maxWeight;
        double maxVolume = instance.truckList[truckId].maxVolume;
        for (int u : tour) {
            Hub& hub = instance.hubList[u];
            curTime = curTime + instance.moveTime(preHub, hub, truckId);
            if (curTime > hub.timeWindow.second)
                return __INT_MAX__;
            curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
            maxWeigh -= hub.weight;
            maxVolume -= hub.volume;
            if (maxWeigh < 0 || maxVolume < 0)
                return __INT_MAX__;
            preHub = hub;
        }
        curTime += instance.moveTime(preHub, startHub, truckId);
        if (curTime > startHub.timeWindow.second)
            return __INT_MAX__;
        return curTime - instance.moveTime(preHub, startHub, truckId);
    }

    /*
    @brief check if route of truck[truckId] is valid
    @return true if truck is valid, otherwise, false
    */
    bool isValidTruck(int truckId, Instance& instance, vector<int>& tour) {
        Hub& startHub = instance.hubList[start[truckId]];
        Hub preHub = startHub;
        int curTime = preHub.timeWindow.first;
        double maxWeigh = instance.truckList[truckId].maxWeight;
        double maxVolume = instance.truckList[truckId].maxVolume;

        vector<int> visited(instance.nOrder * 2 + 1, 0);
        int sumCategory = 0;
        for (int u : tour) {
            // ensure node is not visited twice
            if (visited[u])
                return false;

            // ensure number pickup node equal to number delivery node
            // combine with some other condition to ensure
            // if pickup node in tour then delivery node also in tour
            if (u > instance.nOrder)
                sumCategory++;
            else
                sumCategory--;

            // ensure delivery node is visited after pickup node
            if (u > instance.nOrder && visited[u - instance.nOrder] == 0)
                return false;

            visited[u] = 1;

            Hub& hub = instance.hubList[u];
            curTime = curTime + instance.moveTime(preHub, hub, truckId);

            // ensure arrive time is in time window
            if (curTime > hub.timeWindow.second)
                return false;
            curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
            maxWeigh -= hub.weight;
            maxVolume -= hub.volume;

            // ensure truck capacity is not violated
            if (maxWeigh < 0 || maxVolume < 0)
                return false;
            preHub = hub;
        }

        curTime += instance.moveTime(preHub, startHub, truckId);

        // ensure truck return to start hub in time window
        if (curTime > startHub.timeWindow.second || sumCategory != 0)
            return false;
        return true;
    }

    /*
    @brief check if solution is valid
    @return true if solution valid, otherwise, false
    */
    bool isValid(Instance& instance) {
        vector<int> truckLocation(instance.nOrder * 2 + 1, 0);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            for (int u : tours[truckId]) {
                // ensure node is not visited twice
                if (truckLocation[u])
                    return false;
                truckLocation[u] = truckId;
            }
        }

        // ensure pickup node and delivery node in the same route
        // which also check by isValidTruck
        for (int i = 1; i <= instance.nOrder; i++)
            if (truckLocation[i] != truckLocation[i + instance.nOrder])
                return false;

        // ensure all truck is valid
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            if (isValidTruck(truckId, instance, tours[truckId]) == false)
                return false;
        }
        return true;
    }

    /*
    @brief calculate AT for all node visited of solution
    @return vector<int> is AT of P-node or D-node, 1 index
    */
    vector<int> CalculateArriveTime(Instance& instance) {
        vector<int> visitTime(instance.nOrder * 2 + 2, __INT_MAX__);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            Hub& startHub = instance.hubList[start[truckId]];
            Hub preHub = startHub;
            int curTime = preHub.timeWindow.first;
            for (int u : tours[truckId]) {
                Hub& hub = instance.hubList[u];
                curTime = curTime + instance.moveTime(preHub, hub, truckId);
                visitTime[u] = max(curTime, hub.timeWindow.first);
                curTime = visitTime[u] + hub.serveTime;
                preHub = hub;
            }
        }
        return visitTime;
    }

    void printHeader(Instance& instance, string inputName, string solverName) {
        int processed = instance.nOrder - temporatorPdPairs.size();
        cout << "---------------------------------------------------------\n";
        cout << "INPUT:\t" << inputName << "\n";
        cout << "SOLVER:\t" << solverName << "\n";
        cout << "OBJECTIVE:\t" << (int)objective(instance) << "\n";
        cout << "SELF VALIDATE:\t" << (bool)isValid(instance) << "\n";
        cout << "TOTAL REQUEST PROCESS:\t" << processed << "/"
             << instance.nOrder << "\n";
        cout << "NUM TRUCK:\t" << instance.nTruck << "\n";
        cout << "AVG PROCESS:\t" << setprecision(3) << fixed
             << 1.0 * processed / instance.nTruck << "\n";
        cout << "---------------------------------------------------------\n\n";
    }

    pair<float, pair<int, int>> betterObj(Instance instance) {
        int processed = instance.nOrder - temporatorPdPairs.size();
        float avg = 1.0 * processed / instance.nTruck;
        int TT = 0;
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            // call TT
            Hub& startHub = instance.hubList[start[truckId]];
            Hub preHub = startHub;
            int curTime = preHub.timeWindow.first;
            for (int u : tours[truckId]) {
                Hub& hub = instance.hubList[u];
                curTime = curTime + instance.moveTime(preHub, hub, truckId);
                curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
                preHub = hub;
            }
            curTime += instance.moveTime(preHub, startHub, truckId);
            TT += curTime - startHub.timeWindow.first;
        }
        return {avg, {processed, TT}};
    }

    /*
    @brief print answer to stdout
    */
    void printAnswer(Instance& instance, string inputName, string solverName) {
        printHeader(instance, inputName, solverName);
        vector<int> AT = CalculateArriveTime(instance);
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            Hub& startHub = instance.hubList[start[truckId]];
            cout << "-------Truck: " << truckId << ":\t"
                 << tours[truckId].size() / 2 << "-------\n";
            cout << startHub.timeWindow.first << "\t"
                 << startHub.timeWindow.second << "\n";
            // first:arriveTime, second:id of hub (decoded)
            for (int u : tours[truckId]) {
                if (u > instance.nOrder)
                    cout << "D\t" << u - instance.nOrder;
                else
                    cout << "P\t" << u;
                Hub& hub = instance.hubList[u];
                cout << "\t" << AT[u] << "\t";
                cout << hub.timeWindow.first << "\t" << hub.timeWindow.second;
                if (AT[u] <= hub.timeWindow.second)
                    cout << "\tVALID TIME\n";
                else
                    cout << "\tINVALID TIME\n";
            }
            Hub& lastHub = tours[truckId].size()
                               ? instance.hubList[tours[truckId].back()]
                               : startHub;
            int lastMove = instance.moveTime(lastHub, startHub, truckId);
            if (tours[truckId].size())
                cout << AT[tours[truckId].back()] + lastMove << "\n";
            else
                cout << startHub.timeWindow.first << "\n";
        }
    }

    /*
    @brief calculate objective value of solution
    @return double
    */
    double objective(Instance& instance) {
        int O = 0, UK = 0;
        double TT = 0;
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            O += tours[truckId].size() / 2;
            UK += (tours[truckId].size() > 0);
            // call TT
            Hub& startHub = instance.hubList[start[truckId]];
            Hub preHub = startHub;
            int curTime = preHub.timeWindow.first;
            for (int u : tours[truckId]) {
                Hub& hub = instance.hubList[u];
                curTime = curTime + instance.moveTime(preHub, hub, truckId);
                curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
                preHub = hub;
            }
            curTime += instance.moveTime(preHub, startHub, truckId);
            TT += curTime - startHub.timeWindow.first;
        }
        return double(O) / instance.nOrder * 1e9 -
               (double)UK / instance.nTruck * 1e6 - (double)TT / 1000;
    }
};

#endif
