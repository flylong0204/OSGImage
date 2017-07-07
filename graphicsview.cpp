#include "graphicsview.h"
#include <QtWidgets/QGraphicsScene>
#include <QtGui/QPaintEngine>
#include <QtOpenGL/QGLWidget>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include "pickhandle.h"
#include "common.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;
using namespace osgEarth::Util;

GraphicsView::GraphicsView(const string filePath, QWidget *parent) : EventAdapter(parent)
{
	this->setScene(new QGraphicsScene);
	m_pSkyNode = nullptr;
	m_pMapNode = nullptr;
	imageNode = nullptr;
	init();
	initScene(filePath);
	addMouseCoords();
	//createLine("E:/mapdata/xll/PipeLinePos.txt");
	ctreatePlaceNode("xian", 108.93, 34.27, 100);
	addPlaceImage("Xian", 108.93, 34.27, 100);
	addPlaceImage("Beijing", 117.5, 39.38, 100);
}

GraphicsView::~GraphicsView()
{
	//release();
}

void GraphicsView::release()
{
	
}

QSize GraphicsView::sizeHint()
{
	return QSize(1024, 768);
}

void GraphicsView::init()
{
	QGLWidget* glViewPort = new QGLWidget;
	glViewPort->setMouseTracking(true);
	glViewPort->setMaximumSize(2000, 2000);
	this->setViewport(glViewPort);
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	startTimer(10);

	m_pRoot = new osg::Group;
	m_pRoot->setName("root");

	//抗锯齿
	osg::Multisample* pms = new osg::Multisample;
	pms->setSampleCoverage(1, true);
	m_pRoot->getOrCreateStateSet()->setMode(GL_MULTISAMPLE_ARB, true);
	m_pRoot->getOrCreateStateSet()->setAttributeAndModes(pms, osg::StateAttribute::ON);

	m_pSceneData = new osg::Group;
	m_pSceneData->setName("SceneData");

	m_pLineDataRoot = new osg::Group;
	m_pLineDataRoot->setName("LineDataRoot"); 

	m_pPlaceNodeRoot = new osg::Group;
	m_pPlaceNodeRoot->setName("PlaceNodeRoot");

	m_pRoot->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	m_pRoot->addChild(m_pSceneData);
	m_pRoot->addChild(m_pLineDataRoot);
	m_pRoot->addChild(m_pPlaceNodeRoot);

}


void GraphicsView::timerEvent(QTimerEvent *event)
{
	this->scene()->update();
}

void GraphicsView::drawBackground(QPainter *painter, const QRectF& rect)
{
	if (painter->paintEngine()->type() != QPaintEngine::OpenGL2)
	{
		return;
	}

	painter->save();
	painter->beginNativePainting();

	m_pViewer->frame();
	//qDebug() << "NearFarRatio : " << m_pViewer->getCamera()->getNearFarRatio() << endl;

	painter->endNativePainting();
	painter->restore();
}

void GraphicsView::initScene(const string filePath)
{
	if (filePath.empty())
		return;

	release();
	// 初始化地球数据;
	osg::Node* node = osgDB::readNodeFile(filePath);
	if (nullptr == node)
		return;

	m_pMapNode = osgEarth::MapNode::findMapNode(node);
	m_pMapNode->setName("MapNode");
	m_pMap = m_pMapNode->getMap();
	m_pMapSRS = m_pMapNode->getMapSRS();
	equatorRadius = m_pMapSRS->getEllipsoid()->getRadiusEquator();
	m_pViewer = new osgViewer::Viewer;
	m_pEarthManipulator = new osgEarth::Util::EarthManipulator;
	//m_pEarthManipulator->getSettings()->setMinMaxPitch(-90.0, -10.0);
	m_pViewer->setCameraManipulator(m_pEarthManipulator);
	m_pViewer->addEventHandler(new osgViewer::StatsHandler);	
	m_pViewer->addEventHandler(new osgGA::StateSetManipulator(m_pViewer->getCamera()->getOrCreateStateSet()));
	m_pViewer->getCamera()->addCullCallback(new osgEarth::Util::AutoClipPlaneCullCallback(m_pMapNode));
	m_pViewer->getCamera()->setNearFarRatio(0.0000001);
	m_pViewer->getCamera()->setSmallFeatureCullingPixelSize(1.0f);  //new
	//m_pViewer->getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);发现问题 ： 视角接近node被转换成像素量
	m_pViewer->setUpViewerAsEmbeddedInWindow(0, 0, width(), height());
	m_pViewer->setSceneData(m_pRoot);
	m_pGraphicsWindow = dynamic_cast<osgViewer::GraphicsWindow*>(
		m_pViewer->getCamera()->getGraphicsContext());

	// Tell the database pager to not modify the unref settings
	m_pViewer->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);

	// thread-safe initialization of the OSG wrapper manager. Calling this here
	// prevents the "unsupported wrapper" messages from OSG
	osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper("osg::Image");

	//赤道半径
	double equatorRadius = m_pMapSRS->getEllipsoid()->getRadiusEquator();//6378137.0
	//初始视点正对中国西安
	m_pEarthManipulator->setHomeViewpoint(osgEarth::Util::Viewpoint("", 105.93, 31.95, 0, 0, -90, equatorRadius * 4), 5);
	m_pEarthManipulator->setViewpoint(osgEarth::Viewpoint("", 105.93, 31.95, 0, 0, -90, equatorRadius * 4), 3);
	//m_pEarthManipulator->setHomeViewpoint(osgEarth::Util::Viewpoint("", 108.93, 34.27, 0, 0, -90, equatorRadius * 4), 5);
	//m_pEarthManipulator->setViewpoint(osgEarth::Viewpoint("", 108.93, 34.27, 0, 0, -90, equatorRadius * 4), 3);

	//m_pEarthManipulator->setHomeViewpoint(osgEarth::Util::Viewpoint("", 110.255, 37.70, 0, 0, -90, equatorRadius * 4), 5);
	//m_pEarthManipulator->setViewpoint(osgEarth::Viewpoint("", 110.255, 37.70, 0, 0, -90, equatorRadius * 4), 3);

	m_pEarthManipulator->getSettings()->getBreakTetherActions().push_back(osgEarth::Util::EarthManipulator::ACTION_GOTO);

	createSky();

	osg::ref_ptr<PickHandle> pickHandle = new PickHandle(m_pViewer, m_pMapNode, m_pPlaceNodeRoot);
	m_pViewer->addEventHandler(pickHandle);

	connect(pickHandle, SIGNAL(signal_pickedNodeName(std::string)), this, SIGNAL(signal_pickNodeName(std::string)));
	connect(pickHandle, SIGNAL(signal_pickedPosition(double, double, double)), this, SIGNAL(signal_pickPosition(double, double, double)));

	
}

void GraphicsView::createSky()
{
	osgEarth::Util::SkyOptions kk;
	kk.ambient() = 0.20f;

	m_pSkyNode = osgEarth::Util::SkyNode::create(kk, m_pMapNode);
	m_pSkyNode->setName("SkyNode");
	// 设置时间;
	osgEarth::DateTime dateTime(QDateTime::currentDateTime().toTime_t());
	osgEarth::Util::Ephemeris* ephemeris = new osgEarth::Util::Ephemeris;
	m_pSkyNode->setEphemeris(ephemeris);
	m_pSkyNode->setDateTime(dateTime);
	m_pSkyNode->attach(m_pViewer, 0);
	m_pSkyNode->setLighting(true);
	m_pSkyNode->addChild(m_pMapNode);
	m_pRoot->addChild(m_pSkyNode);
}

bool GraphicsView::addImage(const string _filePath)
{
	if (_filePath.empty() || !QString::fromStdString(_filePath).endsWith(".tif") && !QString::fromStdString(_filePath).endsWith(".png"))
	{
		QMessageBox::about(this, "Error", tr("Must be a TIF file"));
		return false;
	}

	osgEarth::Drivers::GDALOptions imagelayerOpt;
	imagelayerOpt.url() = osgEarth::URI(_filePath);
	//imagelayerOpt.blackExtensions();	

	osgEarth::ImageLayerOptions layerOpt(imagelayerOpt);
	osg::ref_ptr<osgEarth::Util::ChromaKeyColorFilter> chromaKey = new osgEarth::Util::ChromaKeyColorFilter;
	//layerOpt.transparentColor() = osg::Vec4ub(0, 0, 0, 255);
	//layerOpt.colorFilters().push_back(chromaKey);
	//const osg::Vec4ub& ck = *layerOpt.transparentColor();
	chromaKey->setColor(osg::Vec3(0, 0, 0));
	chromaKey->setDistance(0.3);

	osg::ref_ptr<osgEarth::ImageLayer> imageLayer = new  osgEarth::ImageLayer(layerOpt); 
	imageLayer->setName(COMMON::getFilePathName(_filePath));

	m_pMap->addImageLayer(imageLayer.get());

	setImageViewpoart(imageLayer);
	
	createLine("E:/mapdata/xll/PipeLinePos.txt");

	return true;
}

void GraphicsView::addPlaceImage(string _name, double lng, double lat, double alt)
{
	if (nullptr == imageNode)
	{
		imageNode = osgDB::readNodeFile("../data/glider.osg");
		imageNode->setName("PlaceNode");
	}

	osg::ref_ptr<osg::Group> xianNode = new osg::Group;
	xianNode->addChild(imageNode);
	xianNode->setName(_name);
	osgEarth::GeoPoint point(m_pMapSRS, lng, lat, 10, osgEarth::ALTMODE_ABSOLUTE);

	osg::Matrix m;

	point.createLocalToWorld(m);

	osg::MatrixTransform* mt = new osg::MatrixTransform(m);
	mt->addChild(xianNode);

	m_pPlaceNodeRoot->addChild(mt);
}

void GraphicsView::addMouseCoords()
{
	m_pCanvas = new osgEarth::Util::Controls::ControlCanvas;
	m_pSceneData->addChild(m_pCanvas);

	osgEarth::Util::Controls::LabelControl* mouseCoordsLabel = new osgEarth::Util::Controls::LabelControl("", osg::Vec4(1.0, 1.0, 1.0, 1.0));
	mouseCoordsLabel->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
	mouseCoordsLabel->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
	mouseCoordsLabel->setBackColor(0, 0, 0, 0.0);
	mouseCoordsLabel->setSize(300, 50);
	mouseCoordsLabel->setMargin(0);
	m_pCanvas->addControl(mouseCoordsLabel);

	m_pViewer->addEventHandler(new osgEarth::Util::MouseCoordsTool(m_pMapNode, mouseCoordsLabel));
}

void GraphicsView::setEMViewPort(double _long, double _lat)
{
	double eRadius = m_pMapSRS->getEllipsoid()->getRadiusEquator();
	if (m_pEarthManipulator)
	{
		m_pEarthManipulator->setViewpoint(osgEarth::Viewpoint("", _long, _lat, 0, 0, -90, eRadius), 3);
	}
}

void GraphicsView::makeCoodinate()
{
	m_pCsn = new osg::CoordinateSystemNode;
	m_pCsn->setEllipsoidModel(new osg::EllipsoidModel());
}



void GraphicsView::setEMViewPortByName(const string _name)
{
	osg::ref_ptr<osgEarth::ImageLayer> i = m_pMap->getImageLayerByName(_name);
	setImageViewpoart(i);
}

void GraphicsView::getImagePos(osgEarth::ImageLayer* _imageLayer, osg::Vec3d& outVec3)
{
	if (_imageLayer)
	{
		if (_imageLayer->getProfile())
		{
			outVec3 = _imageLayer->getProfile()->getLatLongExtent().getCentroid();
		}
	}
	//osgEarth::DataExtentList dataExtentList;
	//_imageLayer->getDataExtents(dataExtentList);
	//GeoPoint point;
	//for (auto i : dataExtentList)
	//{
	//	outVec3 = i.getCentroid();
	//	i.getCentroid(point);
	//}
}

void GraphicsView::setImageViewpoart(osgEarth::ImageLayer* _imageLayer)
{
	osg::Vec3d vec3;
	if (_imageLayer)
	{
		if (_imageLayer->getProfile())
		{
			vec3 = _imageLayer->getProfile()->getLatLongExtent().getCentroid();
			setEMViewPort(vec3.x(), vec3.y());
		}
	}
	else
	{
		setEMViewPort(108.93, 34.27);
	}
}

void GraphicsView::createLine(QString _filePath)
{
	QFile file(_filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	
	QVector<QStringList> vec;
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QString str(line);
		QStringList slist = str.split(" ", QString::SkipEmptyParts);
		vec.append(slist);
	}

	const SpatialReference* mapSRS = m_pMapNode->getMapSRS();
	const SpatialReference* geoSRS = m_pMapNode->getMapSRS()->getGeographicSRS();

	FeatureNode* pathNode = 0;
	{
		Geometry* path = new LineString();
		for (auto& i : vec)
		{
			path->push_back(osg::Vec3d(i.at(0).toDouble(), i.at(1).toDouble(), 0));
		}

		Feature* pathFeature = new Feature(path, geoSRS);
		pathFeature->geoInterp() = GEOINTERP_GREAT_CIRCLE;
		Style pathStyle;
		pathStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Red;
		pathStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
		pathStyle.getOrCreate<LineSymbol>()->tessellationSize() = 1000;
		pathStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
		pathStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;//TECHNIQUE_DRAPE
		pathNode = new FeatureNode(m_pMapNode, pathFeature, pathStyle);
		//pathNode->setCullingActive(false);
		m_pLineDataRoot->addChild(pathNode);
	}
}


void GraphicsView::initBoundaries(osgEarth::Map* map)
{
	//osgEarth::Drivers::OGRFeatureOptions featureOptions;
	//featureOptions.url() = "E:/mapdata/shpFile/sdfgh.shp";
	//
	//osgEarth::Drivers::FeatureGeomModelOptions geomOptions;
	//geomOptions.featureOptions() = featureOptions;
	//osgEarth::Symbology::Style labelStyle;
	//geomOptions.styles()->addStyle(labelStyle);
	//geomOptions.enableLighting() = false;
	//
	//osgEarth::ModelLayerOptions layerOptions("china_boundaries", geomOptions);
	//layerOptions.overlay()=true; 
	//
	//
	////osgEarth::Drivers::FeatureGeomModelOptions worldBoundaries;
	////osgEarth::Drivers::OGRFeatureOptions ogrOptions; ogrOptions.url() = "world.shp";
	////worldBoundaries.compilerOptions() = ogrOptions; //worldBoundaries.styles(). 
	//map->addModelLayer(new osgEarth::ModelLayer("world", geomOptions));
}

void GraphicsView::ctreatePlaceNode(QString name, double lon, double lat, double elevation)
{

	// 增加西安和北京的地标
	osgEarth::Symbology::Style labelStyle;
	labelStyle.getOrCreate<osgEarth::Symbology::TextSymbol>()->alignment() = osgEarth::Symbology::TextSymbol::ALIGN_CENTER_CENTER;
	labelStyle.getOrCreate<osgEarth::Symbology::TextSymbol>()->fill()->color() = osgEarth::Symbology::Color::Yellow;

	osgEarth::Symbology::Style pm;
	pm.getOrCreate<osgEarth::Symbology::IconSymbol>()->url()->setLiteral("../data/placemark32.png");
	pm.getOrCreate<osgEarth::Symbology::IconSymbol>()->declutter() = true;
	pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->halo() = osgEarth::Symbology::Color("#5f5f5f");

	osg::ref_ptr<osgEarth::Annotation::PlaceNode> placeBeijing = new osgEarth::Annotation::PlaceNode(m_pMapNode, osgEarth::GeoPoint(m_pMapSRS, 117.5, 39.38), "Beijing", pm);
	placeBeijing->setName(placeBeijing->getText());
	osg::ref_ptr<osgEarth::Annotation::PlaceNode> placeXian = new osgEarth::Annotation::PlaceNode(m_pMapNode, osgEarth::GeoPoint(m_pMapSRS, 108.93, 34.27), "Xian", pm);
	placeXian->setName(placeXian->getText());

	m_pPlaceNodeRoot->addChild(placeBeijing);
	m_pPlaceNodeRoot->addChild(placeXian);


	//osgEarth::Drivers::SimpleModelOptions opt;
	//
	//opt.url() = "glider.osg";
	//
	//opt.location() = osg::Vec3d(lon, lat, elevation);//lon和lat单位是度，elevation单位是米，例如(112, 36, 1000)
	//
	//m_pMap->addModelLayer(new osgEarth::ModelLayer(name.toStdString(), opt));
	//
	//osg::ref_ptr<osgEarth::Util::ObjectLocatorNode> objectLocatorNode = new osgEarth::Util::ObjectLocatorNode(m_pMap);
	//osg::ref_ptr<osgEarth::Util::ObjectLocator> objectNode = new osgEarth::Util::ObjectLocator(m_pMap);
	//
	//m_pPlaceNodeRoot->addChild(objectLocatorNode);
}
