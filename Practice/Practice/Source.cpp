#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <fstream>
#include <algorithm>
using namespace std;

namespace planets {
	map<string, int> planets;
	vector<int> connections[500];

	float X[500], Y[500], Z[500];
	float range[500];
	int K = 0;

	struct node {
		int index;
		node *prevNode;

		node(int i) {
			prevNode = 0;
			index = i;
		}

		int length() {
			if (prevNode) return prevNode->length() + 1; else return 1;
		}

		bool inPath(int ind) {
			if (index == ind) return true;
			if (prevNode) return prevNode->inPath(ind); else return false;
		}
	};

	set<int> findPlanetsInPath(int startPlanet) {
		node* nodes[500] = { 0 };
		nodes[startPlanet] = new node(startPlanet);
		list<node*> openList;
		openList.push_back(nodes[startPlanet]);
		while (openList.size() > 0) {
			node* cur = openList.front();
			int curLength = cur->length();
			openList.pop_front();
			for (int c : connections[cur->index]) {
				if (!nodes[c]) {
					nodes[c] = new node(c);
					nodes[c]->prevNode = cur;
					openList.push_back(nodes[c]);
				}
				else {
					if (!cur->inPath(c))
						if (nodes[c]->length() < curLength + 1) {
							nodes[c]->prevNode = cur;
						}
				}
			}
		}
		node *longestPath = nodes[startPlanet];
		int maxLength = 1;
		for (int i = 0; i < K; i++) {
			if (nodes[i]) {
				int l = nodes[i]->length();
				if (l > maxLength) {
					maxLength = l;
					longestPath = nodes[i];
				}
			}
		}
		set<int> res;
		while (longestPath) {
			res.insert(longestPath->index);
			longestPath = longestPath->prevNode;
		}
		for (int i = 0; i < K; i++) {
			if (nodes[i]) delete nodes[i];
		}
		return res;
	}

	vector<string> split(string s) {
		int i;
		vector<string> res;
		while ((i = s.find_first_of(',')) != s.npos) {
			res.push_back(s.substr(0, i));
			s = s.substr(i + 1);
		}
		res.push_back(s);
		return res;
	}

	int main() {
		cin >> K;
		for (int i = 0; i < K; i++) {
			string inp;
			cin >> inp;
			vector<string> Str = split(inp);
			X[i] = stof(Str[0]);
			Y[i] = stof(Str[1]);
			Z[i] = stof(Str[2]);
			range[i] = stof(Str[3]);
			string planet;
			planet = Str[4];
			planets[planet] = i;

			for (int j = 0; j < i; j++) {
				float dist = (X[i] - X[j]) * (X[i] - X[j]) + (Y[i] - Y[j])*(Y[i] - Y[j]) + (Z[i] - Z[j])*(Z[i] - Z[j]);
				if (dist <= range[i] * range[i]) {
					connections[i].push_back(j);
				}
				if (dist <= range[j] * range[j]) {
					connections[j].push_back(i);
				}
			}
		}

		set<int> largestSet = findPlanetsInPath(0);

		bool visit[500] = { false };
		for (auto i : largestSet) {
			visit[i] = true;
		}

		bool first = true;
		for (auto p : planets) {
			if (visit[p.second]) {
				if (!first) cout << ",";
				first = false;
				cout << p.first;
			}
		}
		cin >> K;
		return 0;
	}
}

namespace fungi {

#define absent 0
#define present 1
#define checking 2
#define inRing 3
#define outOfRing 4


	ifstream cin("G:\\prac.txt");

	int H, W;
	short img[200][200] = { absent };
	short tempImg[200][200] = { absent };
	int count = 0;

	void eraseUnconnected(int x, int y) {
		if (x < 0) return;
		if (x >= H) return;
		if (y < 0) return;
		if (y >= W) return;
		if (img[x][y] == absent) return;
		img[x][y] = absent;
		tempImg[x][y] = present;
		eraseUnconnected(x - 1, y);
		eraseUnconnected(x + 1, y);
		eraseUnconnected(x, y - 1);
		eraseUnconnected(x, y + 1);
		eraseUnconnected(x - 1, y - 1);
		eraseUnconnected(x + 1, y - 1);
		eraseUnconnected(x - 1, y + 1);
		eraseUnconnected(x + 1, y + 1);
	}

	bool isInRing(int x, int y) {
		if (x < 0) return false;
		if (x >= H) return false;
		if (y < 0) return false;
		if (y >= W) return false;
		if (tempImg[x][y] != absent) return true;
		tempImg[x][y] = checking;
		bool result = true;
		if (!isInRing(x - 1, y))	 result = false;
		if (!isInRing(x + 1, y))	 result = false;
		if (!isInRing(x, y - 1))	 result = false;
		if (!isInRing(x, y + 1))	 result = false;
		if (!isInRing(x - 1, y - 1)) result = false;
		if (!isInRing(x + 1, y - 1)) result = false;
		if (!isInRing(x - 1, y + 1)) result = false;
		if (!isInRing(x + 1, y + 1)) result = false;
		return result;
	}

	int main() {
		int N;
		fungi::cin >> H >> W >> N;

		while (N-- > 0) {
			int x, y;
			fungi::cin >> x >> y;
			img[x][y] = present;
		}
		multiset<int> rings;

		for (int m = 0; m < H; m++) {
			for (int n = 0; n < W; n++) {
				if (img[m][n] == present) {
					int count = 0;
					memset(tempImg, 0, sizeof tempImg);
					eraseUnconnected(m, n);
					//cout << m << " < " << n << endl;
					for (int i = 0; i < H; i++) {
						for (int j = 0; j < W; j++) {
							if (tempImg[i][j] == absent)
								if (isInRing(i, j)) {
									count++;
									//	cout << i << " " << j << endl;
								}
						}
					}
					rings.insert(count);
				}
			}
		}
		for (auto i : rings) cout << i << " ";
		std::cin >> N;
		return 0;
	}
};

namespace minesweeper {

	int rows, cols;
	int V[300][300];
	int v[300][300];
	int t;
	int R[300], C[300];
	int r[300], c[300];
	int nSol = 0;
	//int fact[100000] = { 0 };
	int nWays[3001][300] = { -1 };

	int getWays(int number, int spaces) {
		if (number < 0) return 0;
		if (number > 3000) return 0;
		if (spaces == 0) return 0;
		if (nWays[number][spaces] != -1) return nWays[number][spaces];
		nWays[number][spaces] = 0;
		if (spaces == 1)
			if (number >= 0 && number <= 9) {
				nWays[number][spaces] = 1;
				return 1;
			}
			else {
				return 0;
			}

			for (int i = 0; i < 10; i++) {
				nWays[number][spaces] = (nWays[number][spaces] + getWays(number - i, spaces - 1)) % 10007;
			}
			return nWays[number][spaces];
	}
	/*
	int getFact(int n) {
		if (n == 1) return 1;
		if (fact[n] != 0) return fact[n];
		fact[n] = getFact(n - 1) * n % 10007;
		return fact[n];
	}*/

	using change = tuple<int, int, bool>;	//Location + increment

	bool computeSolution() {
		for (int i = 0; i < rows; i++) {
			r[i] = (R[i] - t) / (cols - 1);
		}

		for (int j = 0; j < cols; j++) {
			c[j] = (C[j] - t) / (rows - 1);
		}

		int res[300][300];
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				res[i][j] = r[i] + c[j] - V[i][j];
				if (res[i][j] < 0 || res[i][j] > 9) return false;
			}
		}

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				v[i][j] = res[i][j];
			}
		}
		return true;
	}

	void displaySolution() {
		if (nSol != 1) cout << nSol << " SOLUTIONS"; else {
			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					cout << v[i][j] << " ";
				}
				cout << endl;
			}
		}
	}

	int main() {
		cin >> rows >> cols;
		memset(nWays, -1, sizeof nWays);
		t = 0;
		for (int i = 0; i < rows; i++) {
			R[i] = 0;
			for (int j = 0; j < cols; j++) {
				cin >> V[i][j];
				R[i] += V[i][j];
			}
			t += R[i];
		}

		for (int i = 0; i < cols; i++) {
			C[i] = 0;
			for (int j = 0; j < rows; j++) {
				C[i] += V[j][i];
			}
		}

		bool inc = false, dec = false;
		if ((t + 1) % (rows + cols - 1) == 0) inc = true;
		if ((t - 1) % (rows + cols - 1) == 0) dec = true;

		if (rows > 1 && cols > 1) {
			set<change> changes;

			for (int i = 0; i < cols; i++) {
				if (inc) {
					int T = (t + 1) / (rows + cols - 1);
					if ((C[i] + 1 - T) % (rows - 1) == 0) {
						for (int j = 0; j < rows; j++) {
							if ((R[j] + 1 - T) % (cols - 1) == 0) {
								changes.insert(change(j, i, true));
							}
						}
					}
				}

				if (dec) {
					int T = (t - 1) / (rows + cols - 1);
					if ((C[i] - 1 - T) % (rows - 1) == 0) {
						for (int j = 0; j < rows; j++) {
							if ((R[j] - 1 - T) % (cols - 1) == 0) {
								changes.insert(change(j, i, false));
							}
						}
					}
				}
			}

			for (auto c : changes) {
				int i = get<0>(c), j = get<1>(c);
				bool Inc = get<2>(c);
				int n = Inc ? 1 : -1;
				t += n;
				R[i] += n;
				C[j] += n;
				V[i][j] += n;
				t /= (rows + cols - 1);
				if (computeSolution()) nSol++;
				t *= (rows + cols - 1);
				t -= n;
				R[i] -= n;
				C[j] -= n;
				V[i][j] -= n;
			}
		}
		else {
			int n1 = 0, n2 = 0, count1 = 0, count2 = 0;
			nSol = 0;
			bool error = false;
			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					if (count1 == 0 || n1 == V[i][j]) {
						n1 = V[i][j];
						count1++;
					}
					else if (count2 == 0 || n2 == V[i][j]) {
						n2 = V[i][j];
						count2++;
					}
					else {
						error = true;
						break;
					}
				}
				if (error) break;
			}
			if (count1 > 0 && count2 > 0) if (abs(n1 - n2) != 1) error = true;
			if (count1 != 1 && count2 != 1) error = true;

			if (!error) {
				if (count1 == 1) {
					if (n1 + 1 == n2 || count2 == 0)
						nSol += getWays(n1 + 1, rows + cols - 1);
					if (n1 - 1 == n2 || count2 == 0)
						nSol += getWays(n1 - 1, rows + cols - 1);
				}
				if (count2 == 1) {
					if (n1 + 1 == n2 || count1 == 0)
						nSol += getWays(n2 - 1, rows + cols - 1);
					if (n1 - 1 == n2 || count1 == 0)
						nSol += getWays(n2 + 1, rows + cols - 1);
				}


				if (nSol == 1) {
					if (n1 == 0) v[0][0] = 1; else v[0][0] = 8;
				}
			}
		}
		displaySolution();
		cin >> rows;

		return 0;
	}
};

namespace elevators {
	vector<string> split(string s) {
		vector <string> res;
		string curStr = "";
		for (auto c : s) {
			if (c == ' ') {
				res.push_back(curStr);
				curStr = "";
			}
			else curStr += c;
		}
		res.push_back(curStr);
		return res;
	}

	int E, F;
	vector<set<int>*> floorAccess;
	list<int> groundFloorAccess;
	int *access = 0;

	int main() {
		cin >> F >> E;
		char s[2];
		cin.getline(s, 2);
		access = new int[F + 1];
		
		for (int i = 0; i < E; i++) {
			access[i] = 0;
			set<int> *s = new set<int>();
			floorAccess.push_back(s);
		}

		for (int i = 0; i < E; i++) {
			char Str[1000];
			cin.getline(Str, 1000);
			string str = Str;
			vector<string> sFloors = split(str);
			for (auto s : sFloors) {
				floorAccess[i]->insert(stoi(s));
				if (s == "0") groundFloorAccess.push_back(i);
			}
		}
		access[0] = 1;
		list<int> openList = groundFloorAccess;
		bool found = false;
		int curAccess = 0;
		while (openList.size() > 0 && !found) {
			curAccess++;
			int e = openList.front();
			openList.pop_front();
			for (auto f : *floorAccess[e]) {
				if (access[f] == 0) openList.push_back(f);
				if (f == F + 1) found = true;
			}
		}
		cout << curAccess;

		for (int i = 0; i < E; i++)
			delete floorAccess[i];

		cin >> E;
		return 0;
	}
};

namespace dirtyStack{
	int main() {
		int N;
		cin >> N;
		int top = 0;
		vector<int> stack;
		vector<bool> dirty;
		for (int i = 0; i < N; i++) {
			int k;
			cin >> k;
			stack.push_back(k);
			dirty.push_back(false);
		}

		while (top < N) {
			if (top + stack[top] >= stack.size()) {
				top++;
			}
			else if (dirty[top + stack[top]]) {
				top++;
			}
			else {
				int second = top + stack[top];
				int t = stack[top];
				stack[top] = stack[second];
				stack[second] = t;
				dirty[top] = dirty[second];
				dirty[second] = true;
			}
		}

		bool first = true;
		for (int i : stack) {
			if (!first) cout << ",";
			first = false;
			cout << i;
		}
		cin >> N;
		return 0;
	}

};

#include <list>
namespace examRoom {
	int main() {
		int N, S;
		cin >> N >> S;
		int n = N;
		map<int, int> desks;	//Desks and distance to next desk
		while (N-- > 0) {
			int p;
			cin >> p;
			desks[p] = 0;
		}

		for (auto I = desks.begin(); next(I) != desks.end(); I++) {
			(I)->second = (next(I))->first - (I)->first;
		}

		for (int i = 0; i < n - S; i++) {
			auto I = desks.begin();
			int shortest = 10000;
			auto shortestIter = I;
			while (next(next(I)) != desks.end()) {
				if (I->second + (next(I)->second) < shortest) {
					shortest = I->second + next(I)->second;
					shortestIter = next(I);
				}
				I++;
			}
			
			prev(shortestIter)->second = shortest;
			desks.erase(shortestIter);
		}

		int shortest = 10000;
		auto I = desks.begin();
		while (next(I) != desks.end()) {
			if (I->second < shortest) {
				shortest = I->second;
			}
			I++;
		}
		cout << shortest;
		//cin >> shortest;
		return 0;
	}

};

namespace substringAnagrams {
	
	int main() {
		int T;
		cin >> T;
		while (T-- > 0) {
			string s;
			cin >> s;
			vector<string> stringsToCheck;
			for (auto c : s) {
				stringsToCheck.push_back(string(1, c));
			}
			int i = 0;
			while (i < stringsToCheck.size() - 1) {
				if (stringsToCheck[i].length() == stringsToCheck[i+1].length())
					stringsToCheck.push_back(stringsToCheck[i][0] + stringsToCheck[i+1]);
				i++;
			}

			auto I = stringsToCheck.begin();

			for (I = stringsToCheck.begin(); I != stringsToCheck.end(); I++) {
				sort(I->begin(), I->end());
			}

			int totalCount = 0;
			int count = 1;
			sort(stringsToCheck.begin(), stringsToCheck.end());
			I = stringsToCheck.begin();
			while (next(I) != stringsToCheck.end()) {
				if (*I == *next(I)) {
					count++;
				}
				else {
					totalCount += count * (count - 1) / 2;
					count = 1;
				}
				I++;
			}
			totalCount += count * (count - 1) / 2;
			cout << totalCount << endl;
		}
		return 0;
	}
};

namespace reverseShuffle {
	int main() {
		string s;
		cin >> s;
		map<char, int> remainCount;
		map<char, int> sourceCount;
		for (auto c : s) {
			if (remainCount.find(c) == remainCount.end()) remainCount[c] = 1;
			else remainCount[c]++;
		}
		sourceCount = remainCount;
		string A;
		auto I = remainCount.begin();
		vector<char> discarded;
		for (int i = s.length() - 1; i >= 0; i--) {
			if (I == remainCount.end()) break;
			char curChar = I->first;
			if (s[i] == curChar) {
				A += s[i];
				discarded.clear();
				I->second -= 2;
				while (I->second == 0) {
					I++;
					if (I == remainCount.end()) break;
				}
				if (I == remainCount.end()) break;
			}
			else if (remainCount[s[i]] / 2 == sourceCount[s[i]]) {
				char smallest = 0;
				int smallestIndex = 0;
				int j = 0;
				if (discarded.size() > 0) smallest = discarded[0];
				while (smallestIndex < (int)discarded.size() - 1) {
					j = smallestIndex;
					smallestIndex = -1;
					for (; j < discarded.size(); j++) {
						char c = discarded[j];
						if (c < s[i] && remainCount[c] > 0) {
							if (smallest > c || smallestIndex == -1) {
								smallest = c;
								smallestIndex = j;
							}
						}
					}
					if (smallestIndex != -1) {
						A += smallest;
						remainCount[smallest] -= 2;
						smallestIndex++;
					}
					else smallestIndex = discarded.size();
				}

				A += s[i];
				remainCount[s[i]] -= 2;
				discarded.clear();
			}
			else discarded.push_back(s[i]);
			sourceCount[s[i]]--;
		}

		cout << A;
		return 0;
	}
};

namespace URL_Parser {

	int findStr(string s, string s2) {
		for (int i = 0; i < (int)(s.length() - s2.length() + 1); i++) {
			if (s[i] == s2[0]) {
				int j;
				for (j = 1; j < s2.length(); j++) {
					if (s[i + j] != s2[j]) break;
				}
				if (j == s2.length()) return i;
			}
		}
		return 0;
	}
	
	int main() {
		ifstream f_in;
		ofstream f_out;
		string curStr;
		f_in.open("urls.txt");
		map<string, set<string>> newsSet;
		int count = 0;
		while (!f_in.eof()) {
			f_in >> curStr;
			curStr = curStr.substr(findStr(curStr, "/lg/")+4);
			string state = curStr.substr(0,curStr.find_first_of('_'));
			string news = curStr.substr(curStr.find_first_of('_') + 1);
			news = news.substr(0, news.find_first_of('.'));
			newsSet[state].insert(news);
			count++;
			cout << state << " < " << news << endl;
		}
		f_in.close();
		f_out.open("out.txt");
		for (auto state : newsSet) {
			f_out << state.first << endl;
			for (auto paper : state.second) {
				f_out << paper << endl;
			}
			f_out << "." << endl;
		}
		f_out.close();
		cout << count;
		return 0;
	}
};

namespace Multiples35 {

	int main() {
		int T;
		cin >> T;
		while (T-- > 0) {
			int N;
			cin >> N;
			int maxN3 = N - 1, maxN5 = N - 1, maxN15 = N - 1;
			while (maxN3 % 3 != 0) maxN3--;
			while (maxN5 % 5 != 0) maxN5--;
			while (maxN15 % 15 != 0) maxN15--;

			int sum = ((N-1) / 3 * (3+maxN3)+(N-1) / 5 * (5+maxN5)-(N-1) / 15 * (15+maxN15)) >> 1;
			cout << sum << endl;
		}
		return 0;
	}

}

int main() {
	return Multiples35::main();
}