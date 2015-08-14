#include "TCP.h"

using ClientCallBackFunc = void(*)(Client*);

Client::Client(SOCKET socket) : ClientSocket(socket) {}

int Client::send(string s) {
	return ::send(ClientSocket, s.c_str(), s.length(), 0);
}

int Client::send(char *c, int length) {
	return ::send(ClientSocket, c, length, 0);
}

int Client::receive(string &s) {
	char recvbuf[2000];
	int iRes = recv(ClientSocket, recvbuf, 2000, 0);
	if (iRes > 0) {
		s = recvbuf;
		s = s.substr(0, iRes);
	}
	return iRes;
}

void Client::close() {
	shutdown(ClientSocket, SD_SEND);
	closesocket(ClientSocket);
}

DWORD __stdcall FunctionCallThread(LPVOID lParam) {
	pair<Client*, ClientCallBackFunc>* ClientFunction = (pair<Client*, ClientCallBackFunc>*)lParam;
	ClientFunction->second(ClientFunction->first);
	delete ClientFunction->first; //For this program, the client is needed later.
	delete ClientFunction;
	return 0;
}

DWORD __stdcall AcceptConnections(LPVOID lParam) {
	pair<SOCKET, ClientCallBackFunc> *socketFunction = (pair<SOCKET, ClientCallBackFunc>*)lParam;

	while (true) {
		Client *C = new Client(accept(socketFunction->first, 0, 0));
		pair<Client*, ClientCallBackFunc> *cf = new pair < Client*, ClientCallBackFunc >;
		cf->first = C;
		cf->second = socketFunction->second;
		DWORD thread;
		CreateThread(0, 0, FunctionCallThread, cf, 0, &thread);
	}
	return 0;
}

MultiClientTCPServer::MultiClientTCPServer(int port, void(*callBackFunc)(Client*)) {
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in ServerAddress;
	ZeroMemory((char*)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	ServerAddress.sin_port = htons(port);
	bind(ListenSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
	listen(ListenSocket, SOMAXCONN);

	pair<SOCKET, ClientCallBackFunc> *socketFunction = new pair < SOCKET, ClientCallBackFunc >;
	socketFunction->first = ListenSocket;
	socketFunction->second = callBackFunc;
	DWORD threadID;
	CreateThread(0, 0, AcceptConnections, socketFunction, 0, &threadID);

}

MultiClientTCPServer::~MultiClientTCPServer() {
	WSACleanup();
}