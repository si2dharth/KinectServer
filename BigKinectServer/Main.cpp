#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"
#include "Server.h"
#include "KinectThread.h"
#include <vector>
#include <iostream>
#include <set>
using std::set;

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

set<string> allowedIPs;
string password;

void loadSettings() {
	ifstream settingsFile("conf.txt");
	settingsFile >> password;
	while (settingsFile) {
		string s;
		settingsFile >> s;
		if (!settingsFile) break;
		allowedIPs.insert(s);
	}
	settingsFile.close();
}

void saveSettings() {
	ofstream settingsFile("conf.txt");
	settingsFile << password;
	for (auto ip : allowedIPs) {
		settingsFile << ip << endl;
	}
	settingsFile.close();
}

bool filterFunction(Client *C, string ip) {
	if (allowedIPs.find(ip) != allowedIPs.end()) return true;
	bool allowed = false;
	while (!allowed) {
		int i = C->send("Enter password : ");
		if (i <= 0) return false;
		string pass;
		i = C->receive(pass);
		if (i <= 0) return false;
		if (pass == password) return true; else C->send("Authentication Failed. ");
	}

}

void TestServer(Client *C) {
	cout << "HELLO" << endl;
}

void AdminServer(Client *C) {
	
}

//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
int main(int nargs, char **args){
	ULONG_PTR gplus;
	calConv();
	Gdiplus::GdiplusStartupInput gInp;
	gInp.GdiplusVersion = 1;
	gInp.SuppressBackgroundThread = FALSE;
	gInp.SuppressExternalCodecs = FALSE;
	Gdiplus::GdiplusStartup(&gplus,&gInp,0);
	loadSettings();
	KinectProvider kinect;
	initAll(&kinect);
	MultiClientTCPServer colorServer(10001, ColorImageServer, filterFunction);
	MultiClientTCPServer infraredServer(10002, InfraredImageServer, filterFunction);
	MultiClientTCPServer depthMapServer(10003, DepthMapServer, filterFunction);
	MultiClientTCPServer bodyMapServer(10004, BodyMapServer, filterFunction);
	MultiClientTCPServer jointMapServer(10005, BodyServer, filterFunction);
	
	MultiClientTCPServer testServer(10000, TestServer, filterFunction);

	while (true) {
		Sleep(10);
		
	}
	Gdiplus::GdiplusShutdown(gplus);
}