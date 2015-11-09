#include "stdafx.h"

vector<string> split(string s, char delim) {
	vector<string> res;
	int i;
	while ((i = s.find_first_of(delim)) != -1) {
		res.push_back(s.substr(0, i));
		s = s.substr(i + 1);
	}
	res.push_back(s);
	return res;
}