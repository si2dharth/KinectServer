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
#include "DebugProvider.h"

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

bool settingsLoaded = false;

void loadSettings() {
	ifstream settingsFile("conf.txt");
	if (settingsFile.bad() || !settingsFile.is_open()) {
		debugGen << "Conf file not found. Add conf.txt to current directory\n";
	}
	else {
		settingsFile >> password;
		debugGen << "Password : " << password << "\nIPs allowed to connect: \n";
		while (settingsFile) {
			string s;
			settingsFile >> s;
			if (!settingsFile) break;
			debugGen << s << "\n";
			allowedIPs.insert(s);
		}
		settingsLoaded = true;
		settingsFile.close();
	}
}

void saveSettings() {
	ofstream settingsFile("conf.txt");
	debugGen << "Saving settings...\n";
	settingsFile << password << endl;
	for (auto ip : allowedIPs) {
		settingsFile << ip << endl;
	}
	settingsFile.close();
	debugGen << "Done\n";
}

bool filterFunction(Client *C, string ip) {
	debugGen << "Incoming connection..." << ip << "\n";
	if (!settingsLoaded) {
		debugGen << "!!! conf file does not exist. Allowed by default !!!\n";
		return true;
	}
	if (allowedIPs.find(ip) != allowedIPs.end()) {
		debugGen << "Allowed to connect without password\n";
		return true;
	}
	bool allowed = false;
	C->setTimeout(10000);				///!!!! Affects all clients coming through Filter function
	while (!allowed) {
		debugGen << "Waiting for password...\n";
		int i = C->send(" Enter password : ");
		if (i <= 0) {
			debugGen << "Client disconnected\n";
			break;
		}
		string pass;
		i = C->receive(pass);
		if (i <= 0) {
			debugGen << "Client disconnected due to timeout\n";
			break;
		}
		if (pass == password) {
			debugGen << "Password correct\n";
			break;
		}
		else {
			debugGen << "Password does not match\n";
			C->send("Authentication Failed. ");
		}
	}
	C->setTimeout(0);
	return allowed;
}

void AdminServer(Client *C) {
	string line;
	addConnection(C, "Admin");
	do {
		C->send(">");
		if (C->receive(line) <= 0) {
			debugGen << "Error : Admin connection lost from " << C->getIP() << "\n";
			break;
		}

		vector<string> command = split(line, ' ');
		if (command[0][0] == '\n' || command[0][0] == '\r') continue;
		cout << command[0] << endl;
		debugGen << "Admin: " << C->getIP() << "; Command : " << command[0] << "\n";

		if (command[0] == "list") {
			for (auto ip : allowedIPs)
				C->send(ip + "\r\n"s);
		}
		else if (command[0] == "add") {
			if (command.size() == 2) {
				debugGen << "Admin: Adding " << command[1] << " to allowed IPs\n";
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
				debugGen << "Admin: Removing " << command[1] << " from allowed IPs\n";
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
					debugGen << "Admin: Changing password to " << command[2] << "\n";
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
				debugGen << "Sending list of connected clients\n";
				vector<pair<string, string>> connectionList = getConnections();
				for (auto &p : connectionList) {
					C->send(p.first + "\t\t" + p.second + "\r\n");
				}
			}
			else {
				C->send("Incorrect usage.\r\nUsage: connections");
			}
		}
		else if (command[0] == "kick") {
			if (command.size() == 3) {
				debugGen << "Admin: Kicking off " << command[1] << " from " << command[2] << " server\n";
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

//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR lpCmdLine, int nShowCmd) {
int main(int nargs, char **args){
	bool processSpeech = true;
	if (CLSID_ExpectedRecognizer != CLSID_SpInprocRecognizer)
		processSpeech = false;

	if (processSpeech) {
		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	}
	else {
		debugGen << "Speech processor off\n";
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

	debugGen << "Starting Image server...\n";
	MultiClientTCPServer colorServer(10001, ColorImageServer, filterFunction);

	debugGen << "Starting Infrared image server...\n";
	MultiClientTCPServer infraredServer(10002, InfraredImageServer, filterFunction);

	debugGen << "Starting Depth map server...\n";
	MultiClientTCPServer depthMapServer(10003, DepthMapServer, filterFunction);

	debugGen << "Starting Body map server...\n";
	MultiClientTCPServer bodyMapServer(10004, BodyMapServer, filterFunction);

	debugGen << "Starting Body skeleton server...\n";
	MultiClientTCPServer jointMapServer(10005, BodyServer, filterFunction);

	debugGen << "Starting Speed recognition server...\n";
	MultiClientTCPServer speechServer(10006, SpeechServer, filterFunction);

	debugGen << "Starting Debug server(unprotected)...\n";
	MultiClientTCPServer debugServer(10007, DebugServer);
	
	debugGen << "Starting Admin server...\n";
	MultiClientTCPServer adminServer(10000, AdminServer, filterFunction);

	debugGen << "All servers started\n";
	while (true) {
		Sleep(10);		
	}
	Gdiplus::GdiplusShutdown(gplus);
	if (processSpeech)
		CoUninitialize();
}