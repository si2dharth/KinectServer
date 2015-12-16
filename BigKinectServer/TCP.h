#pragma once
#include "stdafx.h"
#include <string>
#include <WinSock2.h>
#include <map>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

///Contains methods to communicate with a connected client.
/**
Create an object from a socket handle and then use the object to communicate with the client.
This class is used with MultiClientTCPServer. The MultiClientTCPServer will always pass a Client object to the Client handling function so that a thread can start communicating with the Client directly.
*/
class Client {
	string ip;
	SOCKET ClientSocket;		///< Handle to the socket to which the client is connected
	bool closed;
public:
	///Initialize the client with the specified socket.
	/**
	@param socket Handle to the socket to which client is connected.
	*/
	Client(SOCKET socket, string ip);

	///Send a message to the client
	/**
	@param [in] s The message to be sent to the connected client
	@return The number of bytes that were sent successfully
	*/
	int send(string s);

	int send(char *c, int length);

	///Receive a message from the client
	/**
	This is a blocking function. It would wait for a message until atleast one byte has arrived.
	@param [out] s The message received
	@return The number of bytes that were received. This is essentially the length of the string
	*/
	int receive(string &s);

	///Receive messages until a specific character is found
	int receive(string &s, string delim);

	///Close the connection
	/**
	Closes the connection with the client
	*/
	void close();

	///Disable Nagle's algorithm, allowing fast transmission of small packets
	void disableNagles();

	///Set a timeout for receive
	void setTimeout(long timeInMs);

	string getIP();
};


///Creates a server that accepts infinite connections and starts a new thread for each connection
/**
The server starts on a new thread and listens for connections on port 8000 for connection requests.
On receiving a request, it creates a new thread and calls the function specified while creating the MultiClientTCPServer object.
While calling the function, a handle to the client is also passed.
Note : It is the function's job to delete the client object after use.
*/
class MultiClientTCPServer {
	WSADATA wsaData;			///< Required for initializing the TCP socket
	SOCKET ListenSocket;		///< Handle to the socket opened for listening to connections
public:
	///Start a multi-client TCP server which calls ClientHandlerFunc whenever a client is connected.
	/**
	@param [in] port The server will listen to connections on this port.
	@param [in] ClientHandlerFunc The function to call whenever a connection request is made on specified port.
	The function should accept a pointer to a Client object. The function should delete the Client object after use.
	*/
	MultiClientTCPServer(int port, void(*ClientHandlerFunc)(Client*), bool (*filterFunc)(Client*, string) = 0);

	///Closes the listening socket.
	~MultiClientTCPServer();
};