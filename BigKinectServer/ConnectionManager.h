#pragma once
#include "stdafx.h"
#include "TCP.h"

void addConnection(Client *client, string type);
void stopConnection(string ip, string type);
void removeConnection(Client *client);

vector<pair<string, string>> getConnections();