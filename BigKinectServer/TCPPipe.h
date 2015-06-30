
class Client;

class TCPPipe {
public:
	TCPPipe(Client *receiver);
	void send();
	void receive();
};