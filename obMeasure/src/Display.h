#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <mrpt/gui.h>
#include <mrpt/opengl.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/maps/CColouredPointsMap.h>

using namespace mrpt;
using namespace mrpt::poses;
using namespace mrpt::maps;


#include <mrpt/gui/CDisplayWindow3D.h>
#include <mrpt/random.h>
#include <mrpt/utils/CTicTac.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/opengl/CGridPlaneXY.h>
#include <mrpt/opengl/CPointCloud.h>
#include <mrpt/opengl/stock_objects.h>
#include <mrpt/opengl/CTexturedPlane.h>

#include <mrpt/gui.h>
#include <mrpt/opengl.h>
#include <mrpt/maps/CColouredPointsMap.h>
#include <mrpt/system/threads.h>

using namespace mrpt::math;


class Display
{
public:
	void InitScene();
	void DrawPointCloud(CMatrixDouble* pData);
	void DrawPlane(opengl::CTexturedPlanePtr pPlane, double a, double b, double c, double d);
	void DrawFrame(CColouredPointsMap* pFrameMap);
	void DrawMessage(const std::string txt);

	gui::CDisplayWindow3D window;
	opengl::CTexturedPlanePtr glPlane;

private:

	opengl::COpenGLScenePtr	scene;

	opengl::CPointCloudPtr  pRandomPoints;
	opengl::CPointCloudColouredPtr pFramePoints;
};

#endif
