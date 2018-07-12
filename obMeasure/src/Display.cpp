#include "Display.h"
#include <iostream>

using namespace std;
using namespace mrpt::math;
using namespace mrpt::utils;

void Display::InitScene()
{
	//										Create scene
	//========================================================================================
	//mrpt::gui::CDisplayWindow3D  win("Set of points", 500, 500);
	mrpt::global_settings::OCTREE_RENDER_MAX_POINTS_PER_NODE = 1000000;
	window.setWindowTitle("Flatness Test v0.1");
	window.resize(800, 600);
	window.setPos(500, 50);
	window.setCameraZoom(1);
	window.setCameraAzimuthDeg(90);
	window.setCameraElevationDeg(270);

	scene = window.get3DSceneAndLock();
	//opengl::COpenGLScenePtr scene = opengl::COpenGLScene::Create();

	// Frame points
	pFramePoints = opengl::CPointCloudColoured::Create();
	pFramePoints->enablePointSmooth(true);
	pFramePoints->setPointSize(2);
	scene->insert(pFramePoints);

	// Fit plane
	glPlane = opengl::CTexturedPlane::Create(-1, 1, -1, 1);
	scene->insert(glPlane);

	// Grid plane
	scene->insert(opengl::CGridPlaneXZ::Create(-2, 2, -2, 2, -1, 0.1));

	// Axis
#if 0
	opengl::CSetOfObjectsPtr reference = opengl::stock_objects::CornerXYZ();
	reference->setScale(0.1);
	scene->insert(reference);
#endif

	window.unlockAccess3DScene();
	window.addTextMessage(5, 5, format("Push any key to exit"));
	window.repaint();
}


void Display::DrawMessage(const std::string txt)
{
	window.addTextMessage(5, 80, txt, TColorf(0, 0, 1.0));
	window.repaint();
}

void Display::DrawPointCloud(CMatrixDouble* pData)
{
	scene = window.get3DSceneAndLock();

	std::vector<float> xs, ys, zs;

	pData->extractRow(0, xs);
	pData->extractRow(1, ys);
	pData->extractRow(2, zs);
	pRandomPoints->setAllPointsFast(xs, ys, zs);

	window.unlockAccess3DScene();
	window.repaint();
}


void Display::DrawPlane(opengl::CTexturedPlanePtr pPlane, double a, double b, double c, double d)
{
	scene = window.get3DSceneAndLock();

	CPose3D   glPlanePose;
	TPlane  plane(a, b, c, d);

	plane.getAsPose3D(glPlanePose);
	pPlane->setPose(glPlanePose);

	window.unlockAccess3DScene();
	window.repaint();
}


void Display::DrawFrame(CColouredPointsMap* pFrameMap)
{
	scene = window.get3DSceneAndLock();
	pFramePoints->loadFromPointsMap<mrpt::maps::CColouredPointsMap>(pFrameMap);

	window.unlockAccess3DScene();
	window.repaint();
}

