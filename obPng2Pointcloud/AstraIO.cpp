#include "AstraIO.h"


AstraIO::AstraIO()
{
	m_initialized = false;
}


AstraIO::~AstraIO()
{
}


void AstraIO::Init(Device& device)
{
	if (!m_initialized)
	{
		OBExtension_Init(&m_ObExt, &device);
		m_initialized = true;
	}
}

void AstraIO::UnInit()
{
	if (m_initialized)
	{
		OBExtension_Deinit(&m_ObExt);
	}
}

int AstraIO::GetCameraParameters(OBCameraParams& params)
{
	if (m_initialized)
	{
		OBExtension_GetProperty(&m_ObExt, OBEXTENSION_ID_CAM_PARAMS, (uint8_t*)&params, OBExtensionList[OBEXTENSION_ID_CAM_PARAMS].datasize);

		if (params.is_mirror == 0xFF)
		{
			return -1; // Data is not valid
		}

		return 0;
	}

	return -1;
}


