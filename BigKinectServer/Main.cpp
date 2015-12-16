#include "stdafx.h"
#include <fstream>
#include "BodyFrameProvider.h"
#include "Server.h"
#include "KinectThread.h"
#include <vector>
#include <iostream>
#include <set>
#include "ConnectionManager.h"
#include <uuids.h>
#include "SpeechHandler.h"

#include <sapi.h>
__pragma(warning(push))
__pragma(warning(disable:6385 6001)) // Suppress warnings in public SDK header
#include <sphelper.h>
__pragma(warning(pop))
using std::set;

#define INITGUID
#include <guiddef.h>
DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);

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
	settingsFile << password << endl;
	for (auto ip : allowedIPs) {
		settingsFile << ip << endl;
	}
	settingsFile.close();
}

bool filterFunction(Client *C, string ip) {
	cout << "Incoming connection" << endl;
	if (allowedIPs.find(ip) != allowedIPs.end()) return true;
	bool allowed = false;
	C->setTimeout(10000);				///!!!! Affects all clients coming through Filter function
	while (!allowed) {
		int i = C->send("Enter password : ");
		if (i <= 0) break;
		string pass;
		i = C->receive(pass);
		if (i <= 0) break;
		if (pass == password) break; 
		else 
			C->send("Authentication Failed. ");
	}
	C->setTimeout(0);
}

void TestServer(Client *C) {
	cout << "HELLO" << endl;
}

void AdminServer(Client *C) {
	string line;
	addConnection(C, "Admin");
	do {
		C->send(">");
		if (C->receive(line) <= 0) break;

		vector<string> command = split(line, ' ');
		if (command[0][0] == '\n' || command[0][0] == '\r') continue;
		cout << command[0] << endl;
		if (command[0] == "list") {
			for (auto ip : allowedIPs)
				C->send(ip + "\r\n"s);
		}
		else if (command[0] == "add") {
			if (command.size() == 2) {
				allowedIPs.insert(command[1]);
				saveSettings();
				C->send("Added "s + command[1] + "\r\n"s);
			}
			else {
				C->send("Incorrect usage. \r\nUsage: add <ip address>\r\n");
			}
		}
		else if (command[0] == "remove") {
			if (command.size() == 2) {
				allowedIPs.erase(command[1]);
				saveSettings();
				C->send("Removed "s + command[1] + "\r\n"s);
			}
			else {
				C->send("Incorrect usage.\r\nUsage: remove <ip address>\r\n");
			}
		}
		else if (command[0] == "password") {
			if (command.size() == 3) {
				if (command[1] == command[2]) {
					password = command[1];
					saveSettings();
					C->send("Password changed\r\n");
				}
				else {
					C->send("Passwords do not match\r\n");
				}
			}
			else {
				C->send("Incorrect usage.\r\nUsage: password <new password> <new password(to confirm)>\r\n");
			}
		}
		else if (command[0] == "connections") {
			if (command.size() == 1) {
				vector<pair<string, string>> connectionList = getConnections();
				for (auto &p : connectionList) {
					C->send(p.first + "\t\t" + p.second + "\r\n");
				}
			}
		}
		else if (command[0] == "kick") {
			if (command.size() == 3) {
				stopConnection(command[1], command[2]);
				C->send(command[1] + " kicked\r\n");
			}
			else {
				C->send("Incorrect usage.\r\nUsage: kick <ip address> <type of connection>\r\n");
			}
		}
		else {
			C->send("Invalid command\r\n");
		}
	} while (line != "exit");
	C->close();
	removeConnection(C);
}

#include <conio.h>
void DebugServer(Client *C) {
	C->setTimeout(10);
	string input, msg;

	while (true) {
		if (C->receive(msg) > 0) {
			cout << msg << endl;
		}
		else if (kbhit()) {
			char ch[100];
			cin.getline(ch,100);
			input = ch;
			if (C->send(input + "\n"s) < 0) break;
		}
	}
	C->close();
}

//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
int main(int nargs, char **args){
	bool processSpeech = true;
	if (CLSID_ExpectedRecognizer != CLSID_SpInprocRecognizer)
		processSpeech = false;

	if (processSpeech) {
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	}

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
	Sleep(1000);
	MultiClientTCPServer colorServer(10001, ColorImageServer, filterFunction);
	MultiClientTCPServer infraredServer(10002, InfraredImageServer, filterFunction);
	MultiClientTCPServer depthMapServer(10003, DepthMapServer, filterFunction);
	MultiClientTCPServer bodyMapServer(10004, BodyMapServer, filterFunction);
	MultiClientTCPServer jointMapServer(10005, BodyServer, filterFunction);
	MultiClientTCPServer speechServer(10006, SpeechServer, filterFunction);
	//MultiClientTCPServer speechDebugServer(10006, DebugServer);
	
	MultiClientTCPServer adminServer(10000, AdminServer, filterFunction);

	while (true) {
		Sleep(10);		
	}
	Gdiplus::GdiplusShutdown(gplus);
	if (processSpeech)
		CoUninitialize();
}