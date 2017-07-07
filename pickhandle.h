#pragma  once
//#include "AdapterWidget.h"
#include <QObject>
#include <osgGA/GUIEventHandler>
#include <osgEarth/SpatialReference>
#include <osgViewer/Viewer>
#include <osgEarthUtil/RTTPicker>
#include <osgEarth/MapNode>
#include <string>
//#include <>

class PickHandle : public QObject, public osgGA::GUIEventHandler
{
	Q_OBJECT
public:
	PickHandle(osg::ref_ptr<osgViewer::Viewer> viewer, osgEarth::MapNode* _mapNode, osg::ref_ptr<osg::Group> _pPlaceNodeRoot);
	~PickHandle();

protected:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
	void Pick(float x, float y);
	osg::ref_ptr<osgViewer::Viewer> m_pviewer;
	osg::ref_ptr<osg::Group> m_pPlaceNodeRoot;
	osgEarth::MapNode* m_pMapNode;
	osg::Node* lastSelect;
	osg::NodePath nodePath;
	const osgEarth::Terrain*   terrain;
	osg::ref_ptr<osgEarth::Util::RTTPicker> m_pPicker;

public slots:

signals :
	void signal_pickedNodeName(std::string name);
	void signal_pickedPosition(double lng, double lat, double alt);
};

