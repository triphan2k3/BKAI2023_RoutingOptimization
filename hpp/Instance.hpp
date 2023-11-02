#ifndef _INSTANCE_HPP_
#define _INSTANCE_HPP_

#include <cmath>
#include <vector>

#ifndef _SUBMIT_
#include "Helper.hpp"
#endif
// class Instance;
// class Order;
// class Truck;
class Hub;
class Order {
   public:
    int id;
    int hubStart, hubEnd;
    double weight, volume;
    int timePick, timeDelivery;
    pair<int, int> windowPick, windowDelivery;
    Order(){};
    void readOrder(int id) {
        this->id = id;
        cin >> hubStart >> hubEnd;
        cin >> weight >> volume;
        cin >> timePick >> timeDelivery;
        windowPick = Helper::ReadTimeLine();
        windowDelivery = Helper::ReadTimeLine();
    }
};
class Truck {
   public:
    int id;
    int hubStart;
    pair<int, int> window;
    double maxWeight, maxVolume, velocity;
    Truck(){};
    void readTruck(int id) {
        this->id = id;
        cin >> hubStart;
        window = Helper::ReadTimeLine();
        cin >> maxWeight >> maxVolume >> velocity;
        velocity /= 3.6;
    }
};

class Instance {
   public:
    const int PICK = 1, DELIVERY = 2, TRUCK = 3;
    int maxDist;
    int nHub, nTruck, nOrder;
    vector<vector<int>> dist;
    vector<Truck> truckList;
    vector<Order> orderList;
    Instance(){};
    vector<Hub> hubList;

    int moveTime(int hub1, int hub2, int truckId);
    int moveTime(Hub& hub1, Hub& hub2, int truckId);
    int moveDist(int hub1, int hub2);
    int moveDist(Hub& hub1, Hub& hub2);
    void readInstance();
    void encode();
    void decode();
    void debug();
};

class Hub {
   public:
    int id, hubID;
    pair<int, int> timeWindow;
    int type;
    int orderID;
    int serveTime;
    double weight, volume;
    Hub() {
        id = hubID = type = orderID = serveTime = weight = volume = 0;
        timeWindow = {0, 0};
    }
    void debug() {
        cout << "id: " << id << "\n";
        cout << "hubID: " << hubID << "\n";
        cout << "timeWindow: " << Helper::IntToTime(timeWindow.first) << " "
             << Helper::IntToTime(timeWindow.second) << "\n";
        cout << "type: " << type << "\n";
        cout << "orderID: " << orderID << "\n";
        cout << "serveTime: " << serveTime << "\n";
        cout << "volume: " << volume << "\n";
        cout << "\n";
    }

    void AssignForOrder(Instance& instance, int orderID, int type, int count);
    void AssignForTruck(Instance& instance, int truckID, int type, int count);
};

int Instance::moveTime(int hub1, int hub2, int truckId) {
    int id1 = hubList[hub1].hubID;
    int id2 = hubList[hub2].hubID;
    int distance = dist[id1][id2];
    return ceil(distance / truckList[truckId].velocity);
}

int Instance::moveTime(Hub& hub1, Hub& hub2, int truckId) {
    int id1 = hub1.hubID;
    int id2 = hub2.hubID;
    int distance = dist[id1][id2];
    return ceil(distance / truckList[truckId].velocity);
}

int Instance::moveDist(int hub1, int hub2) {
    int id1 = hubList[hub1].hubID;
    int id2 = hubList[hub2].hubID;
    int distance = dist[id1][id2];
    return distance;
}

int Instance::moveDist(Hub& hub1, Hub& hub2) {
    int id1 = hub1.hubID;
    int id2 = hub2.hubID;
    int distance = dist[id1][id2];
    return distance;
}

void Instance::readInstance() {
    cin >> nHub;
    dist.resize(nHub + 1, vector<int>(nHub + 1, 0));
    for (int i = 1; i <= nHub; i++)
        for (int j = 1; j <= nHub; j++) {
            cin >> dist[i][j];
            dist[i][j] *= 1000;
            maxDist = max(maxDist, dist[i][j]);
        }
    cin >> nTruck;
    truckList.resize(nTruck + 1);
    for (int i = 1; i <= nTruck; i++)
        truckList[i].readTruck(i);

    cin >> nOrder;
    orderList.resize(nOrder + 1);
    for (int i = 1; i <= nOrder; i++)
        orderList[i].readOrder(i);
}

void Hub::AssignForOrder(Instance& instance, int orderID, int type, int count) {
    this->id = count;
    this->type = type;
    this->orderID = orderID;
    Order& order = instance.orderList[orderID];
    serveTime = (type == instance.PICK) ? order.timePick : order.timeDelivery;
    hubID = (type == 1) ? order.hubStart : order.hubEnd;
    timeWindow = (type == 1) ? order.windowPick : order.windowDelivery;
    volume = (type == 1) ? order.volume : -order.volume;
    weight = (type == 1) ? order.weight : -order.weight;
    if (type == 1)
        order.hubStart = count;
    else
        order.hubEnd = count;
}

void Hub::AssignForTruck(Instance& instance, int truckID, int type, int count) {
    this->id = count;
    this->type = type;
    this->orderID = orderID;
    Truck& truck = instance.truckList[truckID];
    serveTime = 0;
    hubID = truck.hubStart;
    timeWindow = truck.window;
    truck.hubStart = count;
}

void Instance::encode() {
    hubList.resize(nOrder * 2 + nTruck + 1);
    for (int i = 1; i <= nOrder; i++)
        hubList[i].AssignForOrder(*this, i, PICK, i);
    for (int i = 1; i <= nOrder; i++)
        hubList[i + nOrder].AssignForOrder(*this, i, DELIVERY, nOrder + i);

    for (int i = 1; i <= nTruck; i++)
        hubList[nOrder * 2 + i].AssignForTruck(*this, i, TRUCK, nOrder * 2 + i);
}

void Instance::decode() {
    for (int i = 1; i <= nOrder; i++) {
        orderList[i].hubStart = hubList[i].hubID;
        orderList[i].hubEnd = hubList[i + nOrder].hubID;
    }
    for (int i = 1; i <= nTruck; i++)
        truckList[i].hubStart = hubList[i + 2 * nOrder].hubID;
}

void Instance::debug() {
    for (Hub hub : hubList) {
        hub.debug();
    }
}

#endif
