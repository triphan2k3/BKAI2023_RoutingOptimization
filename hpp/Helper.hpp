
#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
using namespace std;
using namespace std::chrono;

class Helper {
   public:
    static const std::string currentDateTime() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
        // for more information about date/time format
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

        return buf;
    }

    static int TimeToInt(string time) {
        int hh = (time[0] - '0') * 10 + (time[1] - '0');
        int mm = (time[3] - '0') * 10 + (time[4] - '0');
        int ss = (time[6] - '0') * 10 + (time[7] - '0');
        return hh * 3600 + mm * 60 + ss;
    }
    static string IntToTime(int time) {
        // return to_string(time);
        int ss = time % 60;
        time /= 60;
        int mm = time % 60;
        int hh = time / 60;
        string hhs = to_string(hh);
        string mms = to_string(mm);
        string sss = to_string(ss);
        if (hhs.size() < 2)
            hhs = "0" + hhs;
        if (mms.size() < 2)
            mms = "0" + mms;
        if (sss.size() < 2)
            sss = "0" + sss;
        return hhs + ":" + mms + ":" + sss;
    }

    static void PrintTime(int time) {
        // return to_string(time);
        int ss = time % 60;
        time /= 60;
        int mm = time % 60;
        int hh = time / 60;
        string hhs = to_string(hh);
        string mms = to_string(mm);
        string sss = to_string(ss);
        cout << setw(2) << setfill('0') << hhs << ":";
        cout << setw(2) << setfill('0') << mms << ":";
        cout << setw(2) << setfill('0') << sss;
    }

    static pair<int, int> ReadTimeLine() {
        string e, l;
        cin >> e >> l;
        return make_pair(TimeToInt(e), TimeToInt(l));
    }
    static double random(double from = 0, double to = 1) {
        // thread_local static std::mt19937 gen(1);
        thread_local static std::mt19937 gen(std::random_device{}());

        using dist_type = typename std::conditional<
            std::is_integral<double>::value,
            std::uniform_int_distribution<double>,
            std::uniform_real_distribution<double>>::type;

        thread_local static dist_type dist;

        return dist(gen, typename dist_type::param_type{from, to});
    }

    static int random_int(int from, int to) {
        return int(Helper::random() * (to - from + 1)) + from;
    }
    template <typename T>
    static int selectWithWeight(vector<T> w) {
        T sum = 0;
        accumulate(w.begin(), w.end(), sum);
        T Rand = Helper::random() * sum;
        T csum = 0;
        for (int i = 0; i < w.size(); i++) {
            csum += w[i];
            if (csum > Rand)
                return i;
        }
        return -1;
    }
};

#endif
