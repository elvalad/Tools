#include "I2CWrapper.h"
#include <iostream>
#include <PS1080.h>

using namespace std;

bool WriteDepthI2C(openni::Device& Device, int address, int value)
{
	XnControlProcessingData I2C;

	I2C.nRegister = (unsigned short)address;
	I2C.nValue = (unsigned short)value;

	int nParam = XN_MODULE_PROPERTY_DEPTH_CONTROL;

	openni::Status rc = Device.setProperty(nParam, I2C);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	return true;
}


bool  ReadDepthI2C(openni::Device& Device, int address, int& value)
{
	XnControlProcessingData I2C;

	I2C.nRegister = (unsigned short)address;

	int nParam = XN_MODULE_PROPERTY_DEPTH_CONTROL;

	if (Device.getProperty(nParam, &I2C) != openni::STATUS_OK)
	{
		cout << "GetParam failed!" << endl;
		return false;
	}

	value = I2C.nValue;

	//cout << "I2C" <<"[0x" << hex << I2C.nRegister << "] = 0x" << hex << I2C.nValue << endl;
	return true;
}