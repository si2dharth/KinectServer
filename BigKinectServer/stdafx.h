#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <d2d1.h>
#include <Kinect.h>
#include <string>
#include <vector>
#pragma comment(lib, "d2d1.lib")

using namespace std;

vector<string> split(string s, char delim = '|');