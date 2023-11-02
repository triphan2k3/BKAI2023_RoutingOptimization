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

    void debug(Instance instance) {
        for (int i = 1; i <= instance.nTruck; i++) {
            cout << start[i] << "\n";
            for (int x : tours[i])
                cout << x << " ";
            cout << endl;
        }
    }

    // void removeFromTour(int truckId, pair<int, int> pd_pair) {
    //     vector<int>& tour = tours[truckId];
    //     for (int i = tour.size() - 1; i >= 0; i--)
    //         if (tour[i] == pd_pair.first || tour[i] == pd_pair.second)
    //             tour.erase(tour.begin() + i);
    //     tourIdOfPdPair[pd_pair.first] = 0;
    // }

    // void insertToTour(int truckId, pair<int, int> pd_pair, pair<int, int>
    // pos) {
    //     vector<int>& tour = tours[truckId];
    //     tour.insert(tour.begin() + pos.first, pd_pair.first);
    //     tour.insert(tour.begin() + pos.second, pd_pair.second);
    //     tourIdOfPdPair[pd_pair.first] = truckId;
    // }

    int calculatePen(int truckId, Instance& instance) {
        Hub& startHub = instance.hubList[start[truckId]];
        Hub preHub = startHub;
        int curTime = preHub.timeWindow.first;
        double maxWeigh = instance.truckList[truckId].maxWeight;
        double maxVolume = instance.truckList[truckId].maxVolume;
        for (int u : tours[truckId]) {
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

    bool isValidTruck(int truckId, Instance& instance) {
        Hub& startHub = instance.hubList[start[truckId]];
        Hub preHub = startHub;
        int curTime = preHub.timeWindow.first;
        double maxWeigh = instance.truckList[truckId].maxWeight;
        double maxVolume = instance.truckList[truckId].maxVolume;

        vector<int> visited(instance.nOrder * 2 + 1, 0);

        for (int u : tours[truckId]) {
            if (visited[u])
                return false;
            if (u > instance.nOrder && visited[u - instance.nOrder] == 0)
                return false;
            visited[u] = 1;

            Hub& hub = instance.hubList[u];
            curTime = curTime + instance.moveTime(preHub, hub, truckId);
            if (curTime > hub.timeWindow.second)
                return false;
            curTime = max(curTime, hub.timeWindow.first) + hub.serveTime;
            maxWeigh -= hub.weight;
            maxVolume -= hub.volume;
            if (maxWeigh < 0 || maxVolume < 0)
                return false;
            preHub = hub;
        }
        curTime += instance.moveTime(preHub, startHub, truckId);
        if (curTime > startHub.timeWindow.second)
            return false;
        return true;
    }

    bool isValid(Instance& instance) {
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            if (isValidTruck(truckId, instance) == false)
                return false;
        }
        return true;
    }
    int numHub(int truckId, Instance instance) {
        vector<int> hubs;
        hubs.push_back(instance.hubList[start[truckId]].hubID);
        for (int u : tours[truckId])
            hubs.push_back(instance.hubList[u].hubID);
        hubs.push_back(instance.hubList[start[truckId]].hubID);
        int ret = 1;
        for (int i = 1; i < hubs.size(); i++)
            if (hubs[i] != hubs[i - 1])
                ++ret;
        return ret;
    }
    // con bug
    void printAnswer(Instance& instance) {
        for (int truckId = 1; truckId <= instance.nTruck; truckId++) {
            Hub& startHub = instance.hubList[start[truckId]];
            Hub preHub = startHub;

            cout << numHub(truckId, instance) << "\n";

            int curTime = preHub.timeWindow.first;
            // first:arriveTime, second:id of hubList
            vector<pair<int, int>> arriveTime;
            arriveTime.push_back({curTime, startHub.id});
            int nItem = 0;
            tours[truckId].push_back(start[truckId]);
            for (int u : tours[truckId]) {
                Hub& hub = instance.hubList[u];
                if (hub.hubID != preHub.hubID) {
                    cout << instance.hubList[arriveTime[0].second].hubID << " "
                         << nItem << " "
                         << Helper::IntToTime(arriveTime[0].first) << " "
                         << Helper::IntToTime(curTime) << "\n";
                    for (int i = 0; i < arriveTime.size(); i++)
                        if (instance.hubList[arriveTime[i].second].type !=
                            instance.TRUCK) {
                            Hub& hub = instance.hubList[arriveTime[i].second];
                            if (hub.type == instance.PICK)
                                cout << hub.id << " "
                                     << Helper::IntToTime(arriveTime[i].first)
                                     << "\n";
                            else
                                cout << hub.id - instance.nOrder << " "
                                     << Helper::IntToTime(arriveTime[i].first)
                                     << "\n";
                        }
                    arriveTime.clear();
                    nItem = 0;
                }
                if (hub.type != instance.TRUCK)
                    ++nItem;
                curTime = curTime + instance.moveTime(preHub, hub, truckId);
                curTime = max(curTime, hub.timeWindow.first);
                arriveTime.push_back({curTime, hub.id});
                curTime += hub.serveTime;
                preHub = hub;
            }
            if (arriveTime.size() == 1) {
                cout << instance.hubList[arriveTime[0].second].hubID << " "
                     << nItem << " " << Helper::IntToTime(arriveTime[0].first)
                     << " " << Helper::IntToTime(curTime) << "\n";
            } else {
                arriveTime.pop_back();
                cout << instance.hubList[arriveTime[0].second].hubID << " "
                     << nItem << " " << Helper::IntToTime(arriveTime[0].first)
                     << " " << Helper::IntToTime(curTime) << "\n";
                for (int i = 0; i < arriveTime.size(); i++)
                    if (instance.hubList[arriveTime[i].second].type !=
                        instance.TRUCK) {
                        Hub& hub = instance.hubList[arriveTime[i].second];
                        if (hub.type == instance.PICK)
                            cout << hub.id << " "
                                 << Helper::IntToTime(arriveTime[i].first)
                                 << "\n";
                        else
                            cout << hub.id - instance.nOrder << " "
                                 << Helper::IntToTime(arriveTime[i].first)
                                 << "\n";
                    }
                arriveTime.clear();
            }
            tours[truckId].pop_back();
        }
    }

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
