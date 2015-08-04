#pragma once

void ServerThread();

class RequestHelper {};

class ResponseHelper {};

class ThreadHelper {

};

class ImageHelper : public ThreadHelper {};

class PictureHelper : public ImageHelper{};

class InfraredHelper : public ImageHelper {};

class BodyMapHelper : public ImageHelper {};

class BodyDataHelper : public ThreadHelper {};

class AudioDataHelper : public ThreadHelper {};