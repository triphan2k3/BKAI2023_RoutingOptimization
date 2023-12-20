#include<bits/stdc++.h>

using namespace std;

using ll = long long;
using ld = long double;
using ull = unsigned long long;

using pii = pair<int, int>;
using pll = pair<ll, ll>;
using pld = pair<ld, ld>;

#define fi first
#define se second
#define left BAO
#define right ANH
#define pb push_back
#define pf push_front
#define mp make_pair
#define ins insert
#define btpc __builtin_popcount
#define btclz __builtin_clz

#define sz(x) (int)(x.size());
#define all(x) x.begin(), x.end()
#define debug(...) " [" << #__VA_ARGS__ ": " << (__VA_ARGS__) << "] "

// mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
mt19937 rng(2109200510);

int d4x[4] = {1, 0, -1, 0}; int d4y[4] = {0, 1, 0, -1};
int d8x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
int d8y[8] = {1, 1, 0, -1, -1, -1, 0, 1};

template<class X, class Y>
    bool minimize(X &x, const Y &y) {
        if (x > y)
        {
            x = y;
            return true;
        }
        return false;
    }
template<class X, class Y>
    bool maximize(X &x, const Y &y) {
        if (x < y)
        {
            x = y;
            return true;
        }
        return false;
    }

const int MOD = 1e9 + 7; //998244353

template<class X, class Y>
	void add(X &x, const Y &y) {
		x = (x + y);
		if(x >= MOD) x -= MOD;
	}

template<class X, class Y> 
	void sub(X &x, const Y &y) {
		x = (x - y);
		if(x < 0) x += MOD;
	}

const ll INF = 1e9;
const int N = 1000 + 10;
const ld EPS = 0.000001;

//Data Hub
int distanceHub[N][N];

//Data Car
int posCar[N];
int staTimeCar[N], finTimeCar[N];
ld capacityCar[N], volumnCar[N];
ld velocityCar[N];

//Data Package
int staPosPack[N], finPosPack[N];
ld weightPack[N], volumnPack[N];
int timeOnPack[N], timeOffPack[N];
int staTimeOnPack[N], finTimeOnPack[N];
int staTimeOffPack[N], finTimeOffPack[N];
int inCar[N];

//Data Operator
struct DataTrans {
	int idPack, timeDone, type;
};
vector<DataTrans> path[N], result[N], tmpPath[N];

//Other array
int permuPack[N], lastDel[N], permuCar[N], tmp[N], costPack[N], idPermuPack[N], tmpPermu[N];
bool canComplete[N], canNotInsert[N];
struct DataOutput {
	int idHub, numOpt, timeIn, timeOut;

	struct DataOpt {
		int idPack, timeDo;
	};	

	vector<DataOpt> Opera;

	DataOutput() {
		idHub = numOpt = timeIn = timeOut = 0;
		Opera.clear();
	};
};
//End

int costTime(int u, int v, int car) {
	return ceil((1.0 * distanceHub[u][v] / velocityCar[car]) * 3600.0);
}

int convertTime(string Time) {
	//Convert to Second
	int hours = (Time[0] - '0') * 10 + (Time[1] - '0');
	int minutes = (Time[3] - '0') * 10 + (Time[4] - '0');
	int seconds = (Time[6] - '0') * 10 + (Time[7] - '0');
	return hours * 3600 + minutes * 60 + seconds;
}

string outTime(int Time) {
	int x = Time / 3600;
	Time -= 3600 * x;
	string answer = "";
	answer += char(x / 10 + '0');
	answer += char(x % 10 + '0');
	answer += ':';
	x = Time / 60;
	Time -= 60 * x;
	answer += char(x / 10 + '0');
	answer += char(x % 10 + '0');
	answer += ':';
	x = Time;
	answer += char(x / 10 + '0');
	answer += char(x % 10 + '0');
	return answer;
};

void reCal(int idCar) {
	int currTime = staTimeCar[idCar];
	int posHub = posCar[idCar];

	for(int i = 0; i < path[idCar].size(); i++) {
		int current = path[idCar][i].idPack;
		if(path[idCar][i].type) {
			//On
			currTime += costTime(posHub, staPosPack[current], idCar);
			maximize(currTime, staTimeOnPack[current]);
			currTime += timeOnPack[current];
			posHub = staPosPack[current];
			path[idCar][i].timeDone = currTime;
		} else {
			//Off
			currTime += costTime(posHub, finPosPack[current], idCar);
			maximize(currTime, staTimeOffPack[current]);
			currTime += timeOffPack[current];
			posHub = finPosPack[current];
			path[idCar][i].timeDone = currTime;
		}
	}
}

int getHub(DataTrans current) {
	return (current.type ? staPosPack[current.idPack] : finPosPack[current.idPack]);
};

int canInsert(int idPack, int idCar, bool Insert, bool keep = false) {
	//find position to insert
	int posOn = 0, posOff = 0;
	ld totW = 0, totV = 0;
	int opt = INF;
	totV = totW = 0;
	int last = 0;
	for(int i = 0; i < path[idCar].size(); i++) {
		if(path[idCar][i].type) last = i;
	}

	for(int i = 0; i <= path[idCar].size(); i++) {
		int currTime = 0;

		if(i > 0) {
			currTime = path[idCar][i - 1].timeDone;
			int current = path[idCar][i - 1].idPack;
			int posHub = (path[idCar][i - 1].type ? staPosPack[current] : finPosPack[current]);
			currTime += costTime(posHub, staPosPack[idPack], idCar);
			if(currTime > finTimeOnPack[idPack]) break;
			maximize(currTime, staTimeOnPack[idPack]);
		} else {
			currTime = staTimeCar[idCar] + costTime(posCar[idCar], staPosPack[idPack], idCar);
			if(currTime > finTimeOnPack[idPack]) break;
		}

		if((!keep || i > last) && totW + weightPack[idPack] <= capacityCar[idCar] && totV + volumnPack[idPack] <= volumnCar[idCar]) {
			ld currTotW = totW + weightPack[idPack];
			ld currTotV = totV + volumnPack[idPack];
			maximize(currTime, staTimeOnPack[idPack]);
			currTime += timeOnPack[idPack];
			int lastHub = staPosPack[idPack];

			for(int j = i; j <= path[idCar].size(); j++) {
				if(j > i) {
					int current = path[idCar][j - 1].idPack;
					if(path[idCar][j - 1].type) {
						//On
						currTime += costTime(lastHub, staPosPack[current], idCar);
						if(currTime > finTimeOnPack[current]) break;
						maximize(currTime, staTimeOnPack[current]);
						currTime += timeOnPack[current];
						currTotW += weightPack[current];
						currTotV += volumnPack[current];
						if(currTotW > capacityCar[idCar] || currTotV > volumnCar[idCar]) break;
						lastHub = staPosPack[current];
					} else {
						//Off
						currTime += costTime(lastHub, finPosPack[current], idCar);
						if(currTime > finTimeOffPack[current]) break;
						maximize(currTime, staTimeOffPack[current]);
						currTime += timeOffPack[current];
						currTotW -= weightPack[current];
						currTotV -= volumnPack[current];
						lastHub = finPosPack[current];
					}
				};

				int tmpTime = currTime + costTime(lastHub, finPosPack[idPack], idCar);
				if(tmpTime > finTimeOffPack[idPack]) break;
				maximize(tmpTime, staTimeOffPack[idPack]);
				tmpTime += timeOffPack[idPack];
				int tmpHub = finPosPack[idPack];

				bool ok = true;
				for(int k = j; k < path[idCar].size(); k++) {
					int current = path[idCar][k].idPack;
					if(path[idCar][k].type) {
						//On
						tmpTime += costTime(tmpHub, staPosPack[current], idCar);
						if(tmpTime > finTimeOnPack[current]) {
							ok = false;
							break;
						}
						maximize(tmpTime, staTimeOnPack[current]);
						tmpTime += timeOnPack[current];
						tmpHub = staPosPack[current];
					} else {
						//Off
						tmpTime += costTime(tmpHub, finPosPack[current], idCar);
						if(tmpTime > finTimeOffPack[current]) {
							ok = false;
							break;
						}
						maximize(tmpTime, staTimeOffPack[current]);
						tmpTime += timeOffPack[current];
						tmpHub = finPosPack[current];
					}
				}
				ok &= (tmpTime + costTime(tmpHub, posCar[idCar], idCar) <= finTimeCar[idCar]);

				if(ok) {
					int D = tmpTime + costTime(tmpHub, posCar[idCar], idCar);
					if(minimize(opt, D)) {
						posOn = i;
						posOff = j;
					}
				}
			}
		}

		if(i == path[idCar].size()) break;
		int current = path[idCar][i].idPack;
		if(path[idCar][i].type) {
			totV += volumnPack[current];
			totW += weightPack[current];
		} else {
			totV -= volumnPack[current];
			totW -= weightPack[current];
		}
	}	
	//End

	//Recalculate
	if(opt != INF && Insert) {
		path[idCar].insert(path[idCar].begin() + posOn, {idPack, 0, 1});
		path[idCar].insert(path[idCar].begin() + posOff + 1, {idPack, 0, 0});

		// cout << posOn << " " << posOff << " " << idPack << " " << idCar << endl;
		// for(int i = 0; i < path[idCar].size(); i++) {
		// 	int current = path[idCar][i].idPack;
		// 	int type = path[idCar][i].type;
		// 	cout << debug(current) debug(type) << endl;
		// }

		reCal(idCar);
	}

	//End

	return opt;
}

struct Solution {
	int cntPack;
	vector<vector<DataTrans>> ord;
	vector<bool> state;

	Solution(int numPack = 0) {
		cntPack = 0;
		state.resize(numPack + 1);
		ord.resize(numPack + 1);
		for(int i = 1; i <= numPack; i++) ord[i].clear(), state[i] = 0;
	}
};
Solution sol[105];

void BaoJiaoPisu() {
	//Input distance Hub Data 
	int numHub; cin >> numHub;
	for(int i = 1; i <= numHub; i++) {
		for(int j = 1; j <= numHub; j++) {
			cin >> distanceHub[i][j];
		}
	}
	//End

	//Input Car Data
	int numCar; cin >> numCar;	
	for(int i = 1; i <= numCar; i++) {
		cin >> posCar[i];
		
		string Time; cin >> Time;
		staTimeCar[i] = convertTime(Time);
		
		cin >> Time;
		finTimeCar[i] = convertTime(Time);

		cin >> capacityCar[i];
		cin >> volumnCar[i];
		cin >> velocityCar[i];
	}	
	//End

	//Input Package Data
	int numPack; cin >> numPack;
	for(int i = 1; i <= numPack; i++) {
		cin >> staPosPack[i] >> finPosPack[i];
		cin >> weightPack[i];
		cin >> volumnPack[i];

		cin >> timeOnPack[i];
		cin >> timeOffPack[i];

		string Time; cin >> Time;
		staTimeOnPack[i] = convertTime(Time);
		cin >> Time;
		finTimeOnPack[i] = convertTime(Time);
		
		cin >> Time;
		staTimeOffPack[i] = convertTime(Time);
		cin >> Time;
		finTimeOffPack[i] = convertTime(Time);
	}
	//End

	//Greedy Algorithm
	int numSol = 16;
	for(int i = 1; i <= numPack; i++) permuPack[i] = i;
	for(int i = 1; i <= numCar; i++) permuCar[i] = i;

	for(int r = 1; r <= numSol; r++) {
		cerr << r << endl;
		for(int i = 1; i <= numCar; i++) path[i].clear();
		shuffle(permuCar + 1, permuCar + 1 + numCar, rng);
		shuffle(permuPack + 1, permuPack + 1 + numPack, rng);
		sol[r] = Solution(numPack);
		for(int i = 1; i <= numCar; i++) {
			int idCar = permuCar[i];
			for(int j = 1; j <= numPack; j++) {
				int idPack = permuPack[j];
				if(sol[r].state[idPack]) continue;
				if(canInsert(idPack, idCar, 1) < INF) {
					sol[r].state[idPack] = idCar;
					sol[r].cntPack++;
				}
			}
		}

		for(int rep = 1; rep <= 10; rep++) {
			for(int i = 1; i <= numCar; i++) {
				int total = 0, ntotal = 0;
				for(int j = i; j <= min(i + 3, numCar); j++) {
					int idCar = permuCar[j];
					vector<DataTrans> newPath;
					for(int i = 0; i < path[idCar].size(); i++) {
						int current = path[idCar][i].idPack;
						if(path[idCar][i].type) {
							if(rng() % 5 == 0) {
								sol[r].state[current] = 0;
							} else {
								newPath.pb(path[idCar][i]);
							}
						} else {
							if(sol[r].state[current]) newPath.pb(path[idCar][i]);
						}
					}

					tmpPath[idCar] = path[idCar];
					total += path[idCar].size();
					path[idCar] = newPath;
					reCal(idCar);
				}

				for(int j = i; j <= min(i + 3, numCar); j++) {
					for(int i = 1; i <= numPack; i++) canNotInsert[i] = false;
					int idCar = permuCar[j];

					while(true) {
						int opt = 0, cost = INF;
						for(int i = 1; i <= numPack; i++) {
							if(sol[r].state[i] || canNotInsert[i]) continue;
							int curr = canInsert(i, idCar, 0);
							if(curr == INF) canNotInsert[i] = true;
							if(minimize(cost, curr)) opt = i;
						}

						if(!opt) break;
						sol[r].state[opt] = idCar;
						canInsert(opt, idCar, 1);
					}
					ntotal += path[idCar].size();
				}

				if(total < ntotal) {
					// cerr << total << " " << ntotal << endl;
					sol[r].cntPack += (ntotal - total) / 2;
					// cerr << "Eureka " << cntPack << endl;
				}

				if(total > ntotal) {
					for(int j = i; j <= min(i + 3, numCar); j++) {
						int idCar = permuCar[j];
						for(int i = 0; i < path[idCar].size(); i++) {
							int current = path[idCar][i].idPack;
							if(path[idCar][i].type) {
								sol[r].state[current] = 0;
							}
						}
					}

					for(int j = i; j <= min(i + 3, numCar); j++) {
						int idCar = permuCar[j];
						swap(tmpPath[idCar], path[idCar]);
						for(int i = 0; i < path[idCar].size(); i++) {
							int current = path[idCar][i].idPack;
							if(path[idCar][i].type) {
								sol[r].state[current] = idCar;
							}
						}
					}
				}
			}
		}

		for(int i = 1; i <= numCar; i++) {
			sol[r].ord[i] = path[i];
		}
	}

	sort(sol + 1, sol + 1 + numSol, [&](Solution a, Solution b) {
		return a.cntPack > b.cntPack;
	});
	while(clock() <= 297.0 * CLOCKS_PER_SEC) {
		for(int r = 1; r <= 1; r++) {
			shuffle(permuCar + 1, permuCar + 1 + numCar, rng);
			shuffle(permuPack + 1, permuPack + 1 + numPack, rng);
			int son = r + numSol;
			// int dad = rng() % (numSol) + 1;
			int dad = rng() % (numSol / 2) + 1;
			int mom = rng() % (numSol) + 1;
			while(dad == mom) {
				dad = rng() % (numSol / 2) + 1;
				mom = rng() % (numSol) + 1;
			}

			sol[son] = Solution(numPack);

			Solution MOM = sol[mom];
			Solution DAD = sol[dad];

			for(int i = 1; i <= numCar; i++) {
				int idCar = permuCar[i];
				// for(int rep = 1; rep <= 10; rep++) 
				{
					vector<DataTrans> newPath;
					path[idCar] = MOM.ord[idCar];
					vector<DataTrans> tmp = path[idCar];
					for(int j = 0; j < path[idCar].size(); j++) {
						int current = path[idCar][j].idPack;
						if(path[idCar][j].type) {
							if(rng() % 5 == 0 || sol[son].state[current]) {
								MOM.state[current] = 0;
							} else {
								newPath.pb(path[idCar][j]);
							}
						} else {
							if(MOM.state[current]) newPath.pb(path[idCar][j]);
						}
					}

					swap(path[idCar], newPath);
					reCal(idCar);
					shuffle(permuPack + 1, permuPack + 1 + numPack, rng);
					for(int j = 1; j <= numPack; j++) {
						int idPack = permuPack[j];
						if(!sol[son].state[idPack] && !MOM.state[idPack] && canInsert(idPack, idCar, 1, 0) < INF) {
							MOM.state[idPack] = 1;
						}
					}
					MOM.ord[idCar] = path[idCar];

					// if(rep > 1 && tmp.size() > path[idCar].size()) MOM.ord[idCar] = tmp;
				}

				// for(int rep = 1; rep <= 10; rep++) 
				{
					vector<DataTrans> newPath;
					path[idCar] = DAD.ord[idCar];
					vector<DataTrans> tmp = path[idCar];
					for(int j = 0; j < path[idCar].size(); j++) {
						int current = path[idCar][j].idPack;
						if(path[idCar][j].type) {
							if(rng() % 5 == 0 || sol[son].state[current]) {
								DAD.state[current] = 0;
							} else {
								newPath.pb(path[idCar][j]);
							}
						} else {
							if(DAD.state[current]) newPath.pb(path[idCar][j]);
						}
					}

					swap(path[idCar], newPath);
					reCal(idCar);
					shuffle(permuPack + 1, permuPack + 1 + numPack, rng);
					for(int j = 1; j <= numPack; j++) {
						int idPack = permuPack[j];
						if(!sol[son].state[idPack] && !DAD.state[idPack] && canInsert(idPack, idCar, 1, 0) < INF) {
							DAD.state[idPack] = 1;
						}
					}
					DAD.ord[idCar] = path[idCar];

					// if(rep > 1 && tmp.size() > path[idCar].size()) DAD.ord[idCar] = tmp;
				}

				path[idCar].clear();
				if(DAD.ord[idCar].size() < MOM.ord[idCar].size()) swap(DAD, MOM);

				{
					vector<DataTrans> newPath;
					path[idCar] = DAD.ord[idCar];
					for(auto x : DAD.ord[idCar]) {
						sol[son].state[x.idPack] = true;
					}

					for(int j = 0; j < path[idCar].size(); j++) {
						int current = path[idCar][j].idPack;
						if(path[idCar][j].type) {
							if(rng() % 5 == 0) {
								sol[son].state[current] = 0;
							} else {
								newPath.pb(path[idCar][j]);
							}
						} else {
							if(sol[son].state[current]) newPath.pb(path[idCar][j]);
						}
					}

					vector<int> curr;
					for(auto x : DAD.ord[idCar]) curr.pb(x.idPack);
					for(auto x : MOM.ord[idCar]) curr.pb(x.idPack);
					sort(all(curr)); curr.resize(unique(all(curr)) - curr.begin());
					shuffle(all(curr), rng);

					swap(path[idCar], newPath);
					reCal(idCar);
					for(auto idPack : curr) {
						if(!sol[son].state[idPack] && canInsert(idPack, idCar, 1, 0) < INF) {
							sol[son].state[idPack] = 1;
						}
					}

					sol[son].cntPack += path[idCar].size() / 2;
				}
			}
			// for(int rep = 1; rep <= 5; rep++) {
			// 	int u = rng() % numPack + 1;
			// 	int v = rng() % numPack + 1;
			// 	swap(sol[son].state[u], sol[son].state[v]);
			// }

			// for(int i = 1; i <= numPack; i++) {
			// 	int idPack = permuPack[i];
			// 	if(sol[son].state[idPack]) {
			// 		int idCar = sol[son].state[idPack];
			// 		if(canInsert(idPack, idCar, 1) < INF) {
			// 			++sol[son].cntPack;
			// 		} else {
			// 			sol[son].state[idPack] = 0;
			// 		}
			// 	}
			// }

			// int cc = 0;
			// for(int i = 1; i <= numPack; i++) cc += sol[son].state[i];
			// cout << sol[son].cntPack << " " << cc << endl;
			// for(int i = 1; i <= numCar; i++) {
			// 	int idCar = permuCar[i];
			// 	for(int j = 1; j <= numPack; j++) {
			// 		int idPack = permuPack[j];
			// 		if(sol[son].state[idPack]) continue;
			// 		if(canInsert(idPack, idCar, 1) < INF) {
			// 			sol[son].state[idPack] = 1;
			// 			sol[son].cntPack++;
			// 		}
			// 	}
			// }

			int bad_cnt = 0;
			while(true) {
				int curr = sol[son].cntPack;
				for(int i = 1; i <= numCar; i++) {
					int total = 0, ntotal = 0;
					for(int j = i; j <= min(i + 3, numCar); j++) {
						int idCar = permuCar[j];
						vector<DataTrans> newPath;
						for(int i = 0; i < path[idCar].size(); i++) {
							int current = path[idCar][i].idPack;
							if(path[idCar][i].type) {
								if(rng() % 5 == 0) {
									sol[son].state[current] = 0;
								} else {
									newPath.pb(path[idCar][i]);
								}
							} else {
								if(sol[son].state[current]) newPath.pb(path[idCar][i]);
							}
						}

						tmpPath[idCar] = path[idCar];
						total += path[idCar].size();
						path[idCar] = newPath;
						reCal(idCar);
					}

					for(int j = i; j <= min(i + 3, numCar); j++) {
						for(int i = 1; i <= numPack; i++) canNotInsert[i] = false;
						int idCar = permuCar[j];

						while(true) {
							int opt = 0, cost = INF;
							for(int i = 1; i <= numPack; i++) {
								if(sol[son].state[i] || canNotInsert[i]) continue;
								int curr = canInsert(i, idCar, 0);
								if(curr == INF) canNotInsert[i] = true;
								if(minimize(cost, curr)) opt = i;
							}

							if(!opt) break;
							sol[son].state[opt] = idCar;
							canInsert(opt, idCar, 1);
						}
						ntotal += path[idCar].size();
					}

					if(total < ntotal) {
						// cerr << total << " " << ntotal << endl;
						sol[son].cntPack += (ntotal - total) / 2;
						// cerr << "Eureka " << cntPack << endl;
					}

					if(total > ntotal) {
						for(int j = i; j <= min(i + 3, numCar); j++) {
							int idCar = permuCar[j];
							for(int i = 0; i < path[idCar].size(); i++) {
								int current = path[idCar][i].idPack;
								if(path[idCar][i].type) {
									sol[son].state[current] = 0;
								}
							}
						}

						for(int j = i; j <= min(i + 3, numCar); j++) {
							int idCar = permuCar[j];
							swap(tmpPath[idCar], path[idCar]);
							for(int i = 0; i < path[idCar].size(); i++) {
								int current = path[idCar][i].idPack;
								if(path[idCar][i].type) {
									sol[son].state[current] = idCar;
								}
							}
						}
					}
				}

				if(sol[son].cntPack == curr) {
					bad_cnt++;
					if(bad_cnt == 3) break;
				} else {
					bad_cnt = 0;
				}
			}

			for(int i = 1; i <= numCar; i++) {
				sol[son].ord[i] = path[i];
			}
		}	

		sort(sol + 1, sol + 1 + numSol + 1, [&](Solution a, Solution b) {
			return a.cntPack > b.cntPack;
		});

		for(int i = 1; i <= numSol; i++) {
			cerr << sol[i].cntPack << " ";
		}
		cerr << 1.0 * clock() / CLOCKS_PER_SEC; 
		cerr << endl;
	};

	for(int i = 1; i <= numCar; i++) result[i] = sol[1].ord[i];

	for(int i = 1; i <= numCar; i++) {
		if(result[i].empty()) {
			cout << 1 << '\n';
			cout << posCar[i] << " " << 0 << " " << outTime(staTimeCar[i]) << " " << outTime(staTimeCar[i]) << '\n';
		} else {
			vector<DataOutput> answer;
			DataOutput tmp = DataOutput();
			int currHub = posCar[i];	
			int currTime = staTimeCar[i];
			tmp.idHub = currHub;
			for(int j = 0; j < result[i].size(); j++) {
				int current = result[i][j].idPack;
				if(result[i][j].type) {
					//On
					if(staPosPack[current] != currHub) {
						tmp.timeOut = currTime;
						answer.pb(tmp);
						tmp = DataOutput();
						tmp.idHub = staPosPack[current];
					}

					currTime += costTime(currHub, staPosPack[current], i);
					maximize(currTime, staTimeOnPack[current]);
					tmp.Opera.pb({current, currTime});
					currTime += timeOnPack[current];
					currHub = staPosPack[current];
				} else {
					//Off
					if(finPosPack[current] != currHub) {
						tmp.timeOut = currTime;
						answer.pb(tmp);
						tmp = DataOutput();
						tmp.idHub = finPosPack[current];
					}

					currTime += costTime(currHub, finPosPack[current], i);
					maximize(currTime, staTimeOffPack[current]);
					tmp.Opera.pb({current, currTime});
					currTime += timeOffPack[current];
					currHub = finPosPack[current];
				}
			}

			tmp.timeOut = currTime;
			answer.pb(tmp);
			if(currHub != posCar[i]) {
				tmp = DataOutput();
				tmp.idHub = posCar[i];
				tmp.timeIn = tmp.timeOut = currTime + costTime(currHub, posCar[i], i);
				answer.pb(tmp);
			}

			answer[0].timeIn = staTimeCar[i];
			int idCar = i;
			cout << answer.size() << '\n';
			for(int i = 0; i < answer.size(); i++) {
				answer[i].numOpt = answer[i].Opera.size();
				if(answer[i].numOpt) {
					answer[i].timeIn = answer[i].Opera[0].timeDo;
				}
                           
				cout << answer[i].idHub << " " << answer[i].numOpt << " " << outTime(answer[i].timeIn) << " " << outTime(answer[i].timeOut) << '\n';
				for(auto current : answer[i].Opera) {
					cout << current.idPack << " " << outTime(current.timeDo) << '\n';
				}
			}
		}
	}
}

int main()
{
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
    #ifndef ONLINE_JUDGE
    // freopen("input.txt", "r", stdin);
    // freopen("output.txt", "w", stdout);
    #else 
    //online
    #endif

    int tc = 1, ddd = 0;
    // cin >> tc;
    while(tc--) {
        //ddd++;
        //cout << "Case #" << ddd << ": ";
        BaoJiaoPisu();
    }
}
