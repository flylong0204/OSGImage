#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QtWidgets/QGraphicsView>
#include <osgGA/NodeTrackerManipulator>
#include <osgOcean/OceanScene>
#include <QtGui/QInputEvent>
#include <iostream>
#include <QSize>
#include "MyOSG.h"
#include "EventAdapter.h"
#include "MyOSGEarth.h"

using namespace std;

class GraphicsView : public EventAdapter
{
	Q_OBJECT

public:
	GraphicsView(const string filePath, QWidget* parent = 0);
	~GraphicsView();

	osgViewer::Viewer* getOSGViewer(){ return m_pViewer; }
	void initScene(const string filePath);
	void release();
	void setEMViewPort(double _long, double _lat);
	void setEMViewPortByName(const string _name);
	bool addImage(const string _filePath);
	void addPlaceImage(string _name, double lng, double lat, double alt);
protected:
	virtual void drawBackground(QPainter *painter, const QRectF& rect);
	virtual void timerEvent(QTimerEvent *event);
	virtual QSize sizeHint();
private:
	void init();
	void createSky();
	
	void addMouseCoords();
	void makeCoodinate();
	void getImagePos(osgEarth::ImageLayer* _imageLayer, osg::Vec3d& outVec3);
	void setImageViewpoart(osgEarth::ImageLayer* _imageLayer);
	void createLine(QString _path);
	void initBoundaries(osgEarth::Map* map);
	void ctreatePlaceNode(QString name, double lon, double lat, double elevation);

private:
	osg::ref_ptr<osgViewer::Viewer> m_pViewer;
	osg::ref_ptr<osg::Group> m_pRoot;
	osg::ref_ptr<osg::Group> m_pSceneData;
	osg::ref_ptr<osg::Group> m_pLineDataRoot;
	osg::ref_ptr<osg::Group> m_pPlaceNodeRoot;
	osg::ref_ptr<osgEarth::MapNode> m_pMapNode;
	osg::ref_ptr<osgEarth::Map> m_pMap;
	osg::ref_ptr<osgEarth::Util::SkyNode> m_pSkyNode;
	osg::ref_ptr<const osgEarth::SpatialReference> m_pMapSRS;
	osg::ref_ptr<osgEarth::Util::EarthManipulator> m_pEarthManipulator;
	osg::ref_ptr<osgEarth::Util::Controls::ControlCanvas> m_pCanvas;
	osg::ref_ptr<osg::CoordinateSystemNode> m_pCsn;
	osg::ref_ptr<osg::Node> imageNode;

	double equatorRadius;

signals:
	void signal_pickNodeName(std::string name);
	void signal_pickPosition(double lng, double lat, double alt);
};

#endif // GRAPHICSVIEW_H
