#pragma once
#include <string>
#include <ostream>
#include <queue>
using std::string;
using std::ostream;
using std::queue;

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
#define debugGen debug_Gen()

#ifdef DeepDebug
#define debugGen2 debug_Gen()
#else
#define debugGen2  ;
#endif