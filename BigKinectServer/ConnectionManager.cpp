#include "ConnectionManager.h"
#include <list>

struct Connection {
	string type;
	Client *client;

	Connection(Client *client, string type) {
		this->type = type;
		this->client = client;
	}
};

list<Connection> connections;

void addConnection(Client *client, string type) {
	connections.push_back(Connection(client, type));
}

void stopConnection(string ip, string type) {
	for (auto C = connections.begin(); C != connections.end();) {
		Connection &cur = *C;
		C++;
		if (cur.client->getIP() == ip && cur.type == type) {
			cur.client->close();
			connections.erase(prev(C));
		}
	}
}

void removeConnection(Client *client) {
	for (auto C = connections.begin(); C != connections.end();) {
		Client *cur = C->client;
		C++;
		if (cur == client) {
			connections.erase(prev(C));
		}
	}
}

vector<pair<string, string>> getConnections() {
	vector<pair<string, string>> result;
	for (auto &C : connections) {
		result.push_back(pair<string, string>(C.client->getIP(), C.type));
	}
	return result;
}