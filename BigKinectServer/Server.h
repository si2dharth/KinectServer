#pragma once

#include "TCP.h"
#include "KinectProvider.h"

///Set the KinectProvider to be used by the server. This method should be called before any of the servers are started.
void initAll(KinectProvider *kinect);

///Server function which serves a client that requires color images. The function will be called on a new thread every time a client is connected.
void ColorImageServer(Client* C);

///Server function which serves a client that requires infrared images. The function will be called on a new thread every time a client is connected.
void InfraredImageServer(Client *C);

///Server function which serves a client that requires depth map. The function will be called on a new thread every time a client is connected.
void DepthMapServer(Client *C);

///Server function which serves a client that requires body index map. The function will be called on a new thread every time a client is connected.
void BodyMapServer(Client *C);

///
void BodyServer(Client *C);

///
void SpeechServer(Client *C);

void DebugServer(Client *C);