#include "TCP.h"

#include <iostream>
using std::cout;

///{Same as typedef} ClientCallBallFunc is type: a function pointer that takes a pointer to a Client and returns nothing
using ClientCallBackFunc = void(*)(Client*);
using FilterFunc = bool(*)(Client*, string);

Client::Client(SOCKET socket, string ip) : ClientSocket(socket), ip(ip){				//Simply initialize the ClientSocket to the socket value specified
	closed = false;
}

int Client::send(string s) {
	return ::send(ClientSocket, s.c_str(), s.length(), 0);			//The global send function defined in WinSock2 requires string as array of characters and the length of the string
}

int Client::send(char *c, int length) {
	return ::send(ClientSocket, c, length, 0);						//The normal call to the WinSock2 global send function
}

int Client::receive(string &s) {
	char recvbuf[2000];												//The receive size is limited to 2000 (can be changed)
	int iRes = recv(ClientSocket, recvbuf, 2000, 0);				//Call the WinSock2 global recv function and receive the message in a char array. The call returns the number of bytes actually received. The call is a blocking one; it waits for atleast one character
	if (iRes > 0) {
		s = recvbuf;
		s = s.substr(0, iRes);										//The recvbuf is not null terminated. So the string has to be truncated to the number of characters received.
	}
	return iRes;
}

int Client::receive(string & s, string delim)
{
	s = "";
	char recvbuf[2];
	recvbuf[0] = 0;
	recvbuf[1] = 0;
	while (delim.find_first_of(recvbuf[0]) == delim.npos) {
		int iRes = recv(ClientSocket, recvbuf, 1, 0);
		
		if (iRes <= 0) break;
		if (delim.find_first_of(recvbuf[0]) != delim.npos) break;
		s += recvbuf;
	};
	if (delim.find_first_of(recvbuf[0]) == delim.npos) return -1;
	cout << "Returning " << s << endl;
	return s.length();
}

void Client::close() {
	if (closed) return;
	shutdown(ClientSocket, SD_SEND);								//Call to the global shutdown and closesocket functions from WinSock2
	closesocket(ClientSocket);
	closed = true;
}

void Client::disableNagles() {
	BOOL noDelay = true;
	setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(BOOL));
}

void Client::setTimeout(long timeInMs)
{
	setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeInMs, sizeof(timeInMs));
}

string Client::getIP() {
	return ip;
}

///This function simply calls the ClientCallBackFunc passed to it for the given Client
DWORD __stdcall FunctionCallThread(LPVOID lParam) {
	auto ClientFunction = (tuple < Client*, ClientCallBackFunc, FilterFunc, string> *)lParam;			//Unpack to tuple
	
	auto &client = get<0>(*ClientFunction);
	auto &callbackFunc = get<1>(*ClientFunction);
	auto &filterFunc = get<2>(*ClientFunction);
	auto &ip = get<3>(*ClientFunction);

	bool pass = true;

	if (filterFunc != 0) {
		pass = filterFunc(client, ip);
	}

	if (pass)
		callbackFunc(client);																					//Call the function
	else
		client->close();
	delete client;																							//Delete the Client once the function completes
	delete ClientFunction;																					//Delete the pair created(by AcceptConnections)
	return 0;
}

///Accepts connections on specified socket and for every connection accepted, calls the ClientCallBackFunc, passed to it, on a new thread
/**
@param lParam A pointer to a pair of SOCKET and ClientCallBackFunc having values of an open listening socket and a function pointer to the function to be called for every accepted connection.
*/
DWORD __stdcall AcceptConnections(LPVOID lParam) {
	tuple<SOCKET, ClientCallBackFunc, FilterFunc> *socketFunction = (tuple<SOCKET, ClientCallBackFunc, FilterFunc>*)lParam;		//Cast from void* to pair*

	while (true) {																						//Repeat forever
		sockaddr_in addr;
		int addr_len = sizeof(addr);
		SOCKET s = accept(get<0>(*socketFunction), (sockaddr*)&addr, &addr_len);		

		string ip = to_string(int(addr.sin_addr.s_addr & 0xFF)) + "."s +
			to_string(int((addr.sin_addr.s_addr & 0xFF00) >> 8)) + "."s +
			to_string(int((addr.sin_addr.s_addr & 0xFF0000) >> 16)) + "."s +
			to_string(int((addr.sin_addr.s_addr & 0xFF000000) >> 24));

		Client *C = new Client(s, ip);									//Accept a connection and start a new client from the connection formed. This is a blocking call.


//		pair<Client*, ClientCallBackFunc> *cf = new pair < Client*, ClientCallBackFunc >;				//Form a new pair in which pointer to Client and the ClientCallBackFunc are packed
		//cf->first = C;
		//cf->second = socketFunction->second;
		auto *cf = new tuple<Client*, ClientCallBackFunc, FilterFunc, string>(make_tuple(C, get<1>(*socketFunction), get<2>(*socketFunction), ip));

		DWORD thread;
		CreateThread(0, 0, FunctionCallThread, cf, 0, &thread);											//Start a new thread for the same
																										//It was possible to CreateThread using socketFunction->second and passing it C, but that would require the socketFunction->second function to take LPVOID(void*) parameter. By creating another function, the void* to Client* cast is made before calling the CallBackFunc
	}
	return 0;
}

MultiClientTCPServer::MultiClientTCPServer(int port, void(*callBackFunc)(Client*), bool (*filterFunc)(Client*, string)) {
	WSAStartup(MAKEWORD(2, 2), &wsaData);																//Initialize for creating sockets
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);											//Create a listening socket: a server

	sockaddr_in ServerAddress;
	ZeroMemory((char*)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;																	//INET for TCP protocol
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;													//Determine the address automatically
	ServerAddress.sin_port = htons(port);																//Set the port specified. Port has to be converted to short using htons https://msdn.microsoft.com/en-us/library/windows/desktop/ms738557(v=vs.85).aspx

	bind(ListenSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));						//Bind the listening socket

	listen(ListenSocket, SOMAXCONN);																	//Start listening. Allow maximum possible connections; theoretically infinite


																										//The CreateThread function can pass only one parameter to a new thread. To pass two parameters, they have to be packed in one.
	//pair<SOCKET, ClientCallBackFunc> *socketFunction = new pair < SOCKET, ClientCallBackFunc >;			//Use pair to pack SOCKET and the ClientCallBackFunc. The pair is required from the heap so that it stays in scope after this function exits
	tuple<SOCKET, ClientCallBackFunc, FilterFunc> *socketFunction = new tuple <SOCKET, ClientCallBackFunc, FilterFunc>;
	
	std::get<0>(*socketFunction) = ListenSocket;
	std::get<1>(*socketFunction) = callBackFunc;
	std::get<2>(*socketFunction) = filterFunc;

	DWORD threadID;
	CreateThread(0, 0, AcceptConnections, socketFunction, 0, &threadID);								//Start the AcceptConnections functions on a new thread and pass it the ListenSocket and the function to be called.

}

MultiClientTCPServer::~MultiClientTCPServer() {
	WSACleanup();																//End all server and client connections and clear all memory related to TCP connections.
}