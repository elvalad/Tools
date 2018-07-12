#include "Sensor.h"
#include <iostream>

using namespace std;

int Sensor::Start()
{
	openni::Status rc = openni::STATUS_OK;

	openni::VideoMode	options;


	// Device is openned
	//=======================================================================================
	const char* deviceURI = openni::ANY_DEVICE;

	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK) { printf("After initialization:\n %s\n", openni::OpenNI::getExtendedError()); }
	rc = device.open(deviceURI);

	//cout << endl << "Do we have IR sensor? " << device.hasSensor(openni::SENSOR_IR);
	//cout << endl << "Do we have RGB sensor? " << device.hasSensor(openni::SENSOR_COLOR);
	//cout << endl << "Do we have Depth sensor? " << device.hasSensor(openni::SENSOR_DEPTH);

	if (rc != openni::STATUS_OK)
	{
		printf("Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 1;
	}

	// Create RGB and Depth channels
	//========================================================================================
	rc = depth.create(device, openni::SENSOR_DEPTH);
	rc = rgb.create(device, openni::SENSOR_COLOR);


	// Configure video properties
	//========================================================================================


	//device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
	registered = false;


	openni::VideoMode vm;

	options = rgb.getVideoMode();
	printf("\nInitial resolution RGB (%d, %d)", options.getResolutionX(), options.getResolutionY());
	options.setResolution(width, height);
	rgb.setVideoMode(options);
	rgb.setMirroringEnabled(false);

	options = depth.getVideoMode();
	printf("\nInitial resolution Depth(%d, %d)", options.getResolutionX(), options.getResolutionY());
	options.setResolution(width, height);
	depth.setVideoMode(options);
	depth.setMirroringEnabled(false);

	options = depth.getVideoMode();
	printf("\nNew resolution (%d, %d) \n", options.getResolutionX(), options.getResolutionY());


	rc = depth.start();
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
		depth.destroy();
	}

	rc = rgb.start();
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't start rgb stream:\n%s\n", openni::OpenNI::getExtendedError());
		rgb.destroy();
	}

	if (rc != openni::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		return 3;
	}

	if (!depth.isValid() || !rgb.isValid())
	{
		printf("No valid streams. Exiting\n");
		openni::OpenNI::shutdown();
		return 2;
	}

	return 0;
}


void Sensor::Stop()
{
	depth.destroy();
	rgb.destroy();
	openni::OpenNI::shutdown();
}


void Sensor::ReadFrame()
{
	openni::VideoFrameRef framed, framergb;

	depth.readFrame(&framed);
	rgb.readFrame(&framergb);

	if ((framed.getWidth() != framergb.getWidth()) || (framed.getHeight() != framergb.getHeight()))
	{
		cout << endl << "Both frames don't have the same size.";
	}
	else
	{
		//Read one frame
		//cout << "read frame..." << endl;

		const openni::DepthPixel* pDepthRow1 = (const openni::DepthPixel*)framed.getData();
		const openni::RGB888Pixel* pRgbRow1 = (const openni::RGB888Pixel*)framergb.getData();

		memcpy(depthBuffer, pDepthRow1, RES_WIDTH * RES_HEIGHT * sizeof(openni::DepthPixel));
		memcpy(colorBuffer, pRgbRow1, RES_WIDTH * RES_HEIGHT * sizeof(openni::RGB888Pixel));

		rowSize = framed.getStrideInBytes() / sizeof(openni::DepthPixel);
	}
}


void Sensor::toggleRegistration()
{
	if (registered)
	{
		device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
		//cout << "registration off" << endl;
	}
	else
	{
		device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		//cout << "registration on" << endl;
	}

	registered = !registered;
}


bool Sensor::getRegistration()
{
	return registered;
}