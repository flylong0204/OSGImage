/* ------------------------------------------------------------
**
** ----------------------------------------------------------*/

#ifndef MYOSG
#define MYOSG
#include <osg/Referenced>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osg/ShapeDrawable>
#include <osg/TexEnv>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Array>
#include <osg/CoordinateSystemNode>
#include <osg/AnimationPath>
#include <osg/Point>
#include <osg/Multisample>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgText/Text>
#include <osgText/Font>

#include <osgFX/Scribe>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ViewDependentShadowMap>

#include <osgUtil/Optimizer>

#include <osgGA/CameraManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/TrackballManipulator>

#include <osgParticle/PrecipitationEffect>

#ifdef _DEBUG
#pragma comment(lib,"osgd.lib")
#pragma comment(lib,"osgDBd.lib")
#pragma comment(lib,"osgGAd.lib")
#pragma comment(lib,"osgManipulatord.lib")
#pragma comment(lib,"osgUtild.lib")
#pragma comment(lib,"osgViewerd.lib")
#pragma comment(lib,"osgWidgetd.lib")
#pragma comment(lib,"osgFXd.lib")
#pragma comment(lib,"osgShadowd.lib")
#pragma comment(lib,"OpenThreadsd.lib")
#else
#pragma comment(lib,"osg.lib")
#pragma comment(lib,"osgDB.lib")
#pragma comment(lib,"osgGA.lib")
#pragma comment(lib,"osgManipulator.lib")
#pragma comment(lib,"osgUtil.lib")
#pragma comment(lib,"osgViewer.lib")
#pragma comment(lib,"osgWidget.lib")
#pragma comment(lib,"osgFX.lib")
#pragma comment(lib,"osgShadow.lib")
#pragma comment(lib,"OpenThreads.lib")
#endif //_debug

#endif