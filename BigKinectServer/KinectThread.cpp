#include "KinectThread.h"

KinectProvider *_kinect = nullptr;

void setKinectProvider(KinectProvider* KP) {
	_kinect = KP;
}

DWORD __stdcall startThread(void *kinectThread) {
	KinectThread *kThread = (KinectThread*)kinectThread;
	while (true) {
		kThread->sendToClient();
	}
}

KinectThread::KinectThread() {
	if (!_kinect) throw "Kinect not set";
}

KinectThread::~KinectThread() {};

void KinectThread::connectClient(Client *C) {
	for (auto c : connectedClients) {
		if (c == C) return;
	}
	connectedClients.push_back(C);
}

void KinectThread::sendToClient() {
	char *c = 0;
	int length;
	if (connectedClients.size() == 0) return;
	run();
	getDataToSend(&c, length);
	list<Client*> removeList;
	if (c == nullptr) return;
	for (auto C : connectedClients) {
		C->send(to_string(length) + "\n"s);
		int i = C->send(c, length);
		if (i == -1) 
			removeList.push_back(C);
	}
	auto I = connectedClients.begin();
	for (auto C : removeList) {
		while (*I != C) I++;
		I = next(I);
		connectedClients.erase(prev(I));
	}
	delete[] c;
}

ImageThread::ImageThread() : KinectThread() {
	setDecay(10);
}

ImageThread::~ImageThread() {}

void ImageThread::run() {
	if (decayTime == 0) return;
	if (images.size() == decayTime) {
		auto image = images.front();
		images.pop_front();
		delete[] image;
	}
	void* img = collectImage(capacity);
	images.push_back(img);
}

void ImageThread::getDataToSend(char **c, int &length) {
	length = capacity;
	if (images.size() == 0) return;
	*c = new char[capacity];
	memcpy(*c, images.back(), capacity);
}

//Time goes in reverse
void ImageThread::getImage(int time, void *image, UINT &cap) {
	cap = capacity;
	time = times.size() - time;
	if (time < 0) time = 0;

	auto I = images.begin();
	while (time-- > 0) I++;

	image = new char*[cap];
	memcpy(image, *I, capacity);
}

void ImageThread::setDecay(unsigned newTime) {
	times.insert(newTime);
	calculateDecayTime();
}

void ImageThread::removeDecay(unsigned time) {
	times.erase(time);
	calculateDecayTime();
}

void ImageThread::calculateDecayTime() {
	if (times.size() == 0) decayTime = 0; else decayTime = *times.rbegin();
}



ColorImageThread::ColorImageThread() : ImageThread() {
	_kinect->startColorCapture();
}

ColorImageThread::~ColorImageThread() {
	_kinect->stopColorCapture();
}

void *ColorImageThread::collectImage(UINT &cap) {
	BYTE *image = nullptr;
	while (_kinect->getImage(&image, cap, true) != _kinect->OK);
	cap *= sizeof(BYTE);
	return image;
}


InfraredImageThread::InfraredImageThread() : ImageThread() {
	_kinect->startInfraredCapture();
}

InfraredImageThread::~InfraredImageThread() {
	_kinect->stopInfraredCapture();
}

void *InfraredImageThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	while (_kinect->getInfraredImage(&image, cap,true) != _kinect->OK);
	cap *= sizeof(UINT16);
	return image;
}

DepthMapThread::DepthMapThread() : ImageThread() {
	_kinect->startDepthMapCapture();
}

DepthMapThread::~DepthMapThread() {
	_kinect->stopDepthMapCapture();
}

void *DepthMapThread::collectImage(UINT &cap) {
	UINT16 *image = nullptr;
	while (_kinect->getDepthMap(&image, cap) != _kinect->OK);
	cap *= sizeof(UINT16);
	return image;
}

BodyMapThread::BodyMapThread() : ImageThread() {
	_kinect->startBodyMapCapture();
}

BodyMapThread::~BodyMapThread() {
	_kinect->stopBodyMapCapture();
}

void *BodyMapThread::collectImage(UINT &cap) {
	BYTE *image = nullptr;
	while (_kinect->getBodyMap(&image, cap) != _kinect->OK);
	cap *= sizeof(BYTE);
	return image;
}