#pragma once
#include "stdafx.h"
#include "TCP.h"

class ConnectedClient {
	string ip;
	Client *clientHandle;
	DWORD threadID;
public:
	ConnectedClient(string ip, Client* clientHandle, DWORD threadID);
};

class ConnectionManager {
public:
	
};