#include "DebugProvider.h"

#include <map>
#include <iostream>
using std::map;


class DebugProvider {
	map<int, int> numLeft;
	map<int, string> msgStore;
	unsigned int curMsg = 0;
	unsigned int numReaders = 0;
	queue<string> msgHistory;

	DebugProvider() {}
	~DebugProvider() {
		singleton = 0;
	}

	static DebugProvider* singleton;
public:
	static DebugProvider *getProvider() {
		if (!singleton) singleton = new DebugProvider();
		return singleton;
	}

	void addMsg(string s) {
		if (numReaders == 0) return;
		msgStore[curMsg++] = s;
	}

	string getMsg(int &index) {
		if (index > curMsg) return "";
		--numLeft[index];
		
		string msg = msgStore[index];

		if (numLeft[index] == 0) {
			numLeft.erase(index);
			msgStore.erase(msgStore.begin(), msgStore.find(index));
		}
		index++;

		return msg;
	}

	void addReader() {
		++numReaders;
	}

	void removeReader(int readerPos) {
		--numReaders;
		for (auto I = numLeft.find(readerPos); I != numLeft.end();) {
			I->second--;
			if (I->second == 0) {
				msgStore.erase(msgStore.begin(), msgStore.find(I->first));
				numLeft.erase(I++);
			}
			else ++I;
		}
	}
};

DebugProvider* DebugProvider::singleton = 0;

DebugGenerator::DebugGenerator() {}

DebugGenerator::~DebugGenerator() {}

void DebugGenerator::addMsg(string s) const {
	DebugProvider::getProvider()->addMsg(s);
	std::cout << s;
}

DebugGenerator &operator <<(const DebugGenerator &dg, string &s) {
	dg.addMsg(s);
}

DebugUser::DebugUser() {
	DebugProvider::getProvider()->addReader();
}

DebugUser::~DebugUser() {
	DebugProvider::getProvider()->removeReader(curIndex);
}

string DebugUser::getNextMsg() {
	return DebugProvider::getProvider()->getMsg(curIndex);
}

ostream &operator <<(ostream &o, DebugUser &du) {
	return o << du.getNextMsg();
}

DebugGenerator debugGenerator;

DebugGenerator & debug_Gen()
{
	return debugGenerator;
}
