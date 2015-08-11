#pragma once

#include "TCP.h"
#include "KinectProvider.h"

void initAll(KinectProvider *kinect);

void ColorImageServer(Client* C);
void InfraredImageServer(Client *C);
void DepthMapServer(Client *C);
void BodyMapServer(Client *C);