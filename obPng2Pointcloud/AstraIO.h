#pragma once

#include "OpenNI.h"
#include "OBExtension.h"

using namespace openni;


class AstraIO
{
public:
	AstraIO();
	virtual ~AstraIO();

public:
	void Init(Device& device);
	void UnInit();
	int GetCameraParameters(OBCameraParams& params);

private:
	OBExtension m_ObExt;
	bool m_initialized;
};

