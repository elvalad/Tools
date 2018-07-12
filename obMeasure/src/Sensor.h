#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <OpenNI.h>

#define RES_WIDTH  640
#define RES_HEIGHT 480

class Sensor
{
public:
	int Start();
	void Stop();

	void ReadFrame();
	void toggleRegistration();
	bool getRegistration();

	openni::DepthPixel depthBuffer[RES_WIDTH * RES_HEIGHT];
	openni::RGB888Pixel colorBuffer[RES_WIDTH * RES_HEIGHT];
	int rowSize = 0;

private:
	openni::Device		device;
	openni::VideoStream depth, rgb;
	int width = RES_WIDTH;
	int height = RES_HEIGHT;
	bool registered;
};

#endif
