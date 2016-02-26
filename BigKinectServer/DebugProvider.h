#pragma once
#include <string>
#include <ostream>
#include <queue>
using std::string;
using std::ostream;
using std::queue;

#define debugGen debug_Gen()

#define DeepDebug FALSE

#define debugGen2 if (DeepDebug) debug_Gen()

class DebugProvider;

class DebugGenerator {
public:
	DebugGenerator();
	virtual ~DebugGenerator();
	void addMsg(string s) const;

	friend const DebugGenerator &operator << (const DebugGenerator &d, const string &s);
};

class DebugUser {
	int curIndex = 0;
public:
	DebugUser();
	virtual ~DebugUser();
	string getNextMsg();

	friend ostream &operator << (ostream &o, DebugUser &);
};


DebugGenerator &debug_Gen();

