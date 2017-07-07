#pragma  once

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/FlightManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>
#include <osg/Notify>
#include <osg/TextureCubeMap>
#include <osgDB/ReadFile>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Program>
#include <osgText/Text>
#include <osg/CullFace>
#include <osg/Fog>
#include <osgText/Font>
#include <osg/Switch>
#include <osg/Texture3D>

#include <string>
#include <vector>

#include <osgOcean/Version>
#include <osgOcean/OceanScene>
#include <osgOcean/FFTOceanSurface>
#include <osgOcean/SiltEffect>
#include <osgOcean/ShaderManager>
#include <osgGA/NodeTrackerManipulator>
#include <osgGA/TerrainManipulator>
#include <iostream>
#include "SkyDome.h"

#define USE_CUSTOM_SHADER

// ----------------------------------------------------
//                  Text HUD Class
// ----------------------------------------------------

class TextHUD : public osg::Referenced
{
private:
	osg::ref_ptr< osg::Camera > _camera;
	osg::ref_ptr< osgText::Text > _modeText;
	osg::ref_ptr< osgText::Text > _cameraModeText;

public:
	TextHUD(void){
		_camera = createCamera();
		_camera->addChild(createText());
	}

	osg::Camera* createCamera(void)
	{
		osg::Camera* camera = new osg::Camera;

		camera->setViewport(0, 0, 1024, 768);
		camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		camera->setProjectionMatrixAsOrtho2D(0, 1024, 0, 768);
		camera->setRenderOrder(osg::Camera::POST_RENDER);
		camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		camera->setClearMask(GL_DEPTH_BUFFER_BIT);

		return camera;
	}

	osg::Node* createText(void)
	{
		osg::Geode* textGeode = new osg::Geode;

		osgText::Text* title = new osgText::Text;
		title->setFont("fonts/arial.ttf");
		title->setCharacterSize(14);
		title->setLineSpacing(0.4f);

		std::string versionText =
			std::string("osgOcean ") +
			std::string(osgOceanGetVersion()) +
			std::string("\nPress 'h' for options");

		title->setText(versionText);
		textGeode->addDrawable(title);

		_modeText = new osgText::Text;
		_modeText->setFont("fonts/arial.ttf");
		_modeText->setCharacterSize(14);
		_modeText->setPosition(osg::Vec3f(0.f, -40.f, 0.f));
		_modeText->setDataVariance(osg::Object::DYNAMIC);
		textGeode->addDrawable(_modeText.get());

		_cameraModeText = new osgText::Text;
		_cameraModeText->setFont("fonts/arial.ttf");
		_cameraModeText->setCharacterSize(14);
		_cameraModeText->setPosition(osg::Vec3f(0.f, -60.f, 0.f));
		_cameraModeText->setDataVariance(osg::Object::DYNAMIC);
		textGeode->addDrawable(_cameraModeText.get());

		osg::PositionAttitudeTransform* titlePAT = new osg::PositionAttitudeTransform;
		titlePAT->setPosition(osg::Vec3f(10, 70, 0.f));
		titlePAT->addChild(textGeode);

		return titlePAT;
	}

	void setSceneText(const std::string& preset)
	{
		_modeText->setText("Preset: " + preset);
	}

	void setCameraText(const std::string& mode)
	{
		_cameraModeText->setText("Camera: " + mode);
	}

	osg::Camera* getHudCamera(void)
	{
		return _camera.get();
	}
};

// ----------------------------------------------------
//               Camera Track Callback
// ----------------------------------------------------

class CameraTrackCallback : public osg::NodeCallback
{
public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
			osg::Vec3f centre, up, eye;
			// get MAIN camera eye,centre,up
			cv->getRenderStage()->getCamera()->getViewMatrixAsLookAt(eye, centre, up);
			// update position
			osg::MatrixTransform* mt = static_cast<osg::MatrixTransform*>(node);
			mt->setMatrix(osg::Matrix::translate(eye.x(), eye.y(), mt->getMatrix().getTrans().z()));
		}

		traverse(node, nv);
	}
};

static double x1 = 0.0;
class ShipPositionCallback : public osg::NodeCallback
{
public:
	ShipPositionCallback(osgOcean::OceanScene* oceanScene)
		: _oceanScene(oceanScene) {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR){
			osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(node);
			if (!mt || !_oceanScene.valid()) return;

			osg::Matrix mat = osg::computeLocalToWorld(nv->getNodePath());
			osg::Vec3d pos = mat.getTrans();
			osg::Vec3f normal;
			// Get the ocean surface height at the object's position, note
			// that this only considers one point on the object (the object's
			// geometric center) and not the whole object.
			float height = _oceanScene->getOceanSurfaceHeightAt(pos.x(), pos.y(), &normal) / 10;

			if (pos.y() < 2000)
			{
				x1 += 0.0001;
				std::cout << pos.x() << "   " << pos.y() + x1 << "   " << height << std::endl;
				mat.makeTranslate(osg::Vec3f(pos.x(), pos.y() + x1, height));

				osg::Matrix rot;
				rot.makeIdentity();
				rot.makeRotate(normal.x() / 15, osg::Vec3f(1.0f, 0.0f, 0.0f),
					normal.y() / 15, osg::Vec3f(0.0f, 1.0f, 0.0f),
					(1.0f - normal.z()) / 15, osg::Vec3f(0.0f, 0.0f, 1.0f));

				mat = rot*mat;
				mt->setMatrix(mat);
			}


		}

		traverse(node, nv);
	}

	osg::observer_ptr<osgOcean::OceanScene> _oceanScene;
};

class Follow : public osgGA::TerrainManipulator
{

public:
	Follow(osg::Group* _group, osg::MatrixTransform* _mt)
	{
		m_vPosition = osg::Vec3(0.0, 0.0, 30.0);
		m_vRotation = osg::Vec3(osg::PI_2, 0.0f, 0.0f);
		m_fMoveSpeed = 2.0;
		m_fAnglg = 2.5;
		carPosition = osg::Vec3(0.0, 20.0, 20.0);
		//mt = CreateMT();
		mt = _mt;
		mt->setMatrix(osg::Matrixd::translate(osg::Vec3(0.0, 12.0, 0.0)));
		group = _group;
		if (group)
		{
			group->addChild(mt);
		}
	}
	virtual void setByMatrix(const osg::Matrixd& matrix)
	{

	}

	virtual void setByInverseMatrix(const osg::Matrixd& matrix)
	{

	}

	virtual osg::Matrixd getMatrix() const
	{
		osg::Matrixd mat;
		mat.makeRotate(m_vRotation.x(), osg::Vec3(1.0, 0.0, 0.0),
			m_vRotation.y(), osg::Vec3(0.0, 1.0, 0.0),
			m_vRotation.z(), osg::Vec3(0.0, 0.0, 1.0));
		return mat * osg::Matrixd::translate(m_vPosition);
	}

	virtual osg::Matrixd getInverseMatrix() const
	{
		osg::Matrixd mat;
		mat.makeRotate(m_vRotation.x(), osg::Vec3(1.0, 0.0, 0.0),
			m_vRotation.y(), osg::Vec3(0.0, 1.0, 0.0),
			m_vRotation.z(), osg::Vec3(0.0, 0.0, 1.0));
		return osg::Matrixd::inverse(mat * osg::Matrixd::translate(m_vPosition));
	}

	void ChangePosition(osg::Vec3 delta)
	{
		m_vPosition = osg::Vec3(m_vPosition.x(), m_vPosition.y(), 3.0);
		m_vPosition += delta;
	}


	void ChangePositionCar(osg::Vec3 delta)
	{
		osg::Vec3 temp;
		temp = osg::Vec3(m_vPosition.x(), m_vPosition.y(), 0.0);
		mt->setMatrix(osg::Matrixd::translate(temp + delta));
		carPosition = temp + delta;
	}

	void ChangePositionCarLeftRight(osg::Vec3 delta)
	{
		carPosition += delta;
		mt->setMatrix(osg::Matrixd::translate(carPosition));
	}

	osg::Geode* CreateSphere()
	{
		osg::ref_ptr<osg::Geode> gnode = new osg::Geode;
		gnode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0, 0.0, 1.0), 1)));
		return gnode.release();
	}

	osg::MatrixTransform* CreateMT()
	{
		osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
		mt->addChild(CreateSphere());
		mt->setMatrix(osg::Matrix::translate(0.0, 0.0, 0.0));
		return mt.release();
	}



	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
	{
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::KEYDOWN:
			if (ea.getKey() == 'w' || ea.getKey() == 'W')
			{
				ChangePosition(osg::Vec3(m_fMoveSpeed * cosf(osg::PI_2 + m_vRotation[2]), m_fMoveSpeed*sinf(osg::PI_2 + m_vRotation[2]), 0));
				ChangePositionCar(osg::Vec3(m_fMoveSpeed * 6 * cosf(osg::PI_2 + m_vRotation[2]), m_fMoveSpeed * 6 * sinf(osg::PI_2 + m_vRotation[2]), 0));
			}

			else if (ea.getKey() == 's' || ea.getKey() == 'S')
			{
				ChangePosition(osg::Vec3(-m_fMoveSpeed * cosf(osg::PI_2 + m_vRotation[2]), -m_fMoveSpeed*sinf(osg::PI_2 + m_vRotation[2]), 0));
				ChangePositionCar(osg::Vec3(m_fMoveSpeed * 6 * cosf(osg::PI_2 + m_vRotation[2]), m_fMoveSpeed * 6 * sinf(osg::PI_2 + m_vRotation[2]), 0));

			}
			else if (ea.getKey() == 'a' || ea.getKey() == 'A')
			{
				ChangePosition(osg::Vec3(-m_fMoveSpeed * sinf(osg::PI_2 + m_vRotation[2]), m_fMoveSpeed * cosf(osg::PI_2 + m_vRotation[2]), 0));
				ChangePositionCarLeftRight(osg::Vec3(-m_fMoveSpeed * sinf(osg::PI_2 + m_vRotation[2]), m_fMoveSpeed * cosf(osg::PI_2 + m_vRotation[2]), 0));
			}
			else if (ea.getKey() == 0xFF53) //ÓÒÐý×ª
			{
				float temp = m_vRotation._v[2];
				m_vRotation._v[2] -= osg::DegreesToRadians(m_fAnglg);
				ChangePosition(osg::Vec3(12 * sin(m_vRotation._v[2]) - 12 * sin(temp), 12 * (1 - cos(m_vRotation._v[2])) - 12 * (1 - cos(temp)), 0));
			}
			else if (ea.getKey() == 0xFF51) //×óÐý×ª
			{
				float temp = m_vRotation._v[2];
				m_vRotation._v[2] += osg::DegreesToRadians(m_fAnglg);
				ChangePosition(osg::Vec3(12 * sin(m_vRotation._v[2]) - 12 * sin(temp), 12 * (1 - cos(m_vRotation._v[2])) - 12 * (1 - cos(temp)), 0));
			}
			else
			{
			}
		}

		return false;
	}

private:
	osg::Vec3 m_vPosition;
	osg::Vec3 m_vRotation;
	float m_fMoveSpeed;
	float m_fAnglg;
	osg::Vec3 carPosition;
	osg::MatrixTransform* mt;
	osg::Group *group;

};

// ----------------------------------------------------
//                  Scoped timer
// ----------------------------------------------------

class ScopedTimer
{
public:
	ScopedTimer(const std::string& description,
		std::ostream& output_stream = std::cout,
		bool endline_after_time = true)
		: _output_stream(output_stream)
		, _start()
		, _endline_after_time(endline_after_time)
	{
		_output_stream << description << std::flush;
		_start = osg::Timer::instance()->tick();
	}

	~ScopedTimer()
	{
		osg::Timer_t end = osg::Timer::instance()->tick();
		_output_stream << osg::Timer::instance()->delta_s(_start, end) << "s";
		if (_endline_after_time) _output_stream << std::endl;
		else                     _output_stream << std::flush;
	}

private:
	std::ostream& _output_stream;
	osg::Timer_t _start;
	bool _endline_after_time;
};

// ----------------------------------------------------
//                  Scene Model
// ----------------------------------------------------

class SceneModel : public osg::Referenced
{
public:
	enum SCENE_TYPE{ CLEAR, DUSK, CLOUDY };

private:
	SCENE_TYPE _sceneType;

	osg::ref_ptr<osgText::Text> _modeText;
	osg::ref_ptr<osg::Group> _scene;

	osg::ref_ptr<osgOcean::OceanScene> _oceanScene;
	osg::ref_ptr<osgOcean::FFTOceanSurface> _oceanSurface;
	osg::ref_ptr<osg::TextureCubeMap> _cubemap;
	osg::ref_ptr<SkyDome> _skyDome;

	std::vector<std::string> _cubemapDirs;
	std::vector<osg::Vec4f>  _lightColors;
	std::vector<osg::Vec4f>  _fogColors;
	std::vector<osg::Vec3f>  _underwaterAttenuations;
	std::vector<osg::Vec4f>  _underwaterDiffuse;

	osg::ref_ptr<osg::Light> _light;

	std::vector<osg::Vec3f>  _sunPositions;
	std::vector<osg::Vec4f>  _sunDiffuse;
	std::vector<osg::Vec4f>  _waterFogColors;

	osg::ref_ptr<osg::Switch> _islandSwitch;

public:
	SceneModel(const osg::Vec2f& windDirection = osg::Vec2f(1.0f, 1.0f),
		float windSpeed = 12.f,
		float depth = 10000.f,
		float reflectionDamping = 0.35f,
		float scale = 1e-8,
		bool  isChoppy = true,
		float choppyFactor = -2.5f,
		float crestFoamHeight = 2.2f) :
		_sceneType(CLEAR)
	{
		_cubemapDirs.push_back("sky_clear");
		_cubemapDirs.push_back("sky_dusk");
		_cubemapDirs.push_back("sky_fair_cloudy");

		_fogColors.push_back(intColor(199, 226, 255));
		_fogColors.push_back(intColor(244, 228, 179));
		_fogColors.push_back(intColor(172, 224, 251));

		_waterFogColors.push_back(intColor(27, 57, 109));
		_waterFogColors.push_back(intColor(44, 69, 106));
		_waterFogColors.push_back(intColor(84, 135, 172));

		_underwaterAttenuations.push_back(osg::Vec3f(0.015f, 0.0075f, 0.005f));
		_underwaterAttenuations.push_back(osg::Vec3f(0.015f, 0.0075f, 0.005f));
		_underwaterAttenuations.push_back(osg::Vec3f(0.008f, 0.003f, 0.002f));

		_underwaterDiffuse.push_back(intColor(27, 57, 109));
		_underwaterDiffuse.push_back(intColor(44, 69, 106));
		_underwaterDiffuse.push_back(intColor(84, 135, 172));

		_lightColors.push_back(intColor(105, 138, 174));
		_lightColors.push_back(intColor(105, 138, 174));
		_lightColors.push_back(intColor(105, 138, 174));

		_sunPositions.push_back(osg::Vec3f(326.573, 1212.99, 1275.19));
		_sunPositions.push_back(osg::Vec3f(520.f, 1900.f, 550.f));
		_sunPositions.push_back(osg::Vec3f(-1056.89f, -771.886f, 1221.18f));

		_sunDiffuse.push_back(intColor(191, 191, 191));
		_sunDiffuse.push_back(intColor(251, 251, 161));
		_sunDiffuse.push_back(intColor(191, 191, 191));

		build(windDirection, windSpeed, depth, reflectionDamping, scale, isChoppy, choppyFactor, crestFoamHeight);
	}

	void build(const osg::Vec2f& windDirection,
		float windSpeed,
		float depth,
		float reflectionDamping,
		float waveScale,
		bool  isChoppy,
		float choppyFactor,
		float crestFoamHeight)
	{
		{
			ScopedTimer buildSceneTimer("Building scene... \n", osg::notify(osg::NOTICE));

			_scene = new osg::Group;

			{
				ScopedTimer cubemapTimer("  . Loading cubemaps: ", osg::notify(osg::NOTICE));
				_cubemap = loadCubeMapTextures(_cubemapDirs[_sceneType]);
			}

			// Set up surface
			{
				ScopedTimer oceanSurfaceTimer("  . Generating ocean surface: ", osg::notify(osg::NOTICE));
				_oceanSurface = new osgOcean::FFTOceanSurface(64, 256, 17,
					windDirection, windSpeed, depth, reflectionDamping, waveScale, isChoppy, choppyFactor, 10.f, 256);

				_oceanSurface->setEnvironmentMap(_cubemap.get());
				_oceanSurface->setFoamBottomHeight(2.2f);
				_oceanSurface->setFoamTopHeight(3.0f);
				_oceanSurface->enableCrestFoam(true);
				_oceanSurface->setLightColor(_lightColors[_sceneType]);
				// Make the ocean surface track with the main camera position, giving the illusion
				// of an endless ocean surface.
				_oceanSurface->enableEndlessOcean(true);
			}

			// Set up ocean scene, add surface
			{
				ScopedTimer oceanSceneTimer("  . Creating ocean scene: ", osg::notify(osg::NOTICE));
				osg::Vec3f sunDir = -_sunPositions[_sceneType];
				sunDir.normalize();

				_oceanScene = new osgOcean::OceanScene(_oceanSurface.get());
				_oceanScene->setLightID(0);
				_oceanScene->enableReflections(true);
				_oceanScene->enableRefractions(true);

				// Set the size of _oceanCylinder which follows the camera underwater. 
				// This cylinder prevents the clear from being visible past the far plane 
				// instead it will be the fog color.
				// The size of the cylinder should be changed according the size of the ocean surface.
				_oceanScene->setCylinderSize(1900.f, 4000.f);

				_oceanScene->setAboveWaterFog(0.0012f, _fogColors[_sceneType]);
				_oceanScene->setUnderwaterFog(0.002f, _waterFogColors[_sceneType]);
				_oceanScene->setUnderwaterDiffuse(_underwaterDiffuse[_sceneType]);
				_oceanScene->setUnderwaterAttenuation(_underwaterAttenuations[_sceneType]);

				_oceanScene->setSunDirection(sunDir);
				_oceanScene->enableGodRays(true);
				_oceanScene->enableSilt(true);
				_oceanScene->enableUnderwaterDOF(true);
				_oceanScene->enableDistortion(true);
				_oceanScene->enableGlare(true);
				_oceanScene->setGlareAttenuation(0.8f);

				// create sky dome and add to ocean scene
				// set masks so it appears in reflected scene and normal scene
				_skyDome = new SkyDome(1900.f, 16, 16, _cubemap.get());
				_skyDome->setNodeMask(_oceanScene->getReflectedSceneMask() | _oceanScene->getNormalSceneMask());

				// add a pat to track the camera
				osg::MatrixTransform* transform = new osg::MatrixTransform;
				transform->setDataVariance(osg::Object::DYNAMIC);
				transform->setMatrix(osg::Matrixf::translate(osg::Vec3f(0.f, 0.f, 0.f)));
				transform->setCullCallback(new CameraTrackCallback);

				transform->addChild(_skyDome.get());

				_oceanScene->addChild(transform);

				{
					// Create and add fake texture for use with nodes without any texture
					// since the OceanScene default scene shader assumes that texture unit 
					// 0 is used as a base texture map.
					osg::Image * image = new osg::Image;
					image->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
					*(osg::Vec4ub*)image->data() = osg::Vec4ub(0xFF, 0xFF, 0xFF, 0xFF);

					osg::Texture2D* fakeTex = new osg::Texture2D(image);
					fakeTex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
					fakeTex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
					fakeTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
					fakeTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);

					osg::StateSet* stateset = _oceanScene->getOrCreateStateSet();
					stateset->setTextureAttribute(0, fakeTex, osg::StateAttribute::ON);
					stateset->setTextureMode(0, GL_TEXTURE_1D, osg::StateAttribute::OFF);
					stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::ON);
					stateset->setTextureMode(0, GL_TEXTURE_3D, osg::StateAttribute::OFF);
				}

			}

			{
				ScopedTimer islandsTimer("  . Loading islands: ", osg::notify(osg::NOTICE));
				osg::ref_ptr<osg::Node> islandModel = loadIslands();

				if (islandModel.valid())
				{
					_islandSwitch = new osg::Switch;
					_islandSwitch->addChild(islandModel.get(), true);
					_islandSwitch->setNodeMask(_oceanScene->getNormalSceneMask() | _oceanScene->getReflectedSceneMask() | _oceanScene->getRefractedSceneMask());
					_oceanScene->addChild(_islandSwitch.get());
				}
			}

			{
				ScopedTimer lightingTimer("  . Setting up lighting: ", osg::notify(osg::NOTICE));
				osg::LightSource* lightSource = new osg::LightSource;
				lightSource->setLocalStateSetModes();

				_light = lightSource->getLight();
				_light->setLightNum(0);
				_light->setAmbient(osg::Vec4d(0.3f, 0.3f, 0.3f, 1.0f));
				_light->setDiffuse(_sunDiffuse[_sceneType]);
				_light->setSpecular(osg::Vec4d(0.1f, 0.1f, 0.1f, 1.0f));
				_light->setPosition(osg::Vec4f(_sunPositions[_sceneType], 1.f)); // point light

				_scene->addChild(lightSource);
				_scene->addChild(_oceanScene.get());
				//_scene->addChild( sunDebug(_sunPositions[CLOUDY]) );
			}

			osg::notify(osg::NOTICE) << "complete.\nTime Taken: ";
		}
	}

	osgOcean::OceanTechnique* getOceanSurface(void)
	{
		return _oceanSurface.get();
	}

	osg::Group* getScene(void){
		return _scene.get();
	}

	osgOcean::OceanScene* getOceanScene()
	{
		return _oceanScene.get();
	}

	void changeScene(SCENE_TYPE type)
	{
		_sceneType = type;

		_cubemap = loadCubeMapTextures(_cubemapDirs[_sceneType]);
		_skyDome->setCubeMap(_cubemap.get());
		_oceanSurface->setEnvironmentMap(_cubemap.get());
		_oceanSurface->setLightColor(_lightColors[type]);

		_oceanScene->setAboveWaterFog(0.0012f, _fogColors[_sceneType]);
		_oceanScene->setUnderwaterFog(0.002f, _waterFogColors[_sceneType]);
		_oceanScene->setUnderwaterDiffuse(_underwaterDiffuse[_sceneType]);
		_oceanScene->setUnderwaterAttenuation(_underwaterAttenuations[_sceneType]);

		osg::Vec3f sunDir = -_sunPositions[_sceneType];
		sunDir.normalize();

		_oceanScene->setSunDirection(sunDir);

		_light->setPosition(osg::Vec4f(_sunPositions[_sceneType], 1.f));
		_light->setDiffuse(_sunDiffuse[_sceneType]);

		if (_islandSwitch.valid())
		{
			if (_sceneType == CLEAR || _sceneType == CLOUDY)
				_islandSwitch->setAllChildrenOn();
			else
				_islandSwitch->setAllChildrenOff();
		}
	}

	// Load the islands model
	// Here we attach a custom shader to the model.
	// This shader overrides the default shader applied by OceanScene but uses uniforms applied by OceanScene.
	// The custom shader is needed to add multi-texturing and bump mapping to the terrain.
	osg::Node* loadIslands(void)
	{
		osgDB::Registry::instance()->getDataFilePathList().push_back("resources/island");
		const std::string filename = "islands.ive";
		osg::ref_ptr<osg::Node> island = osgDB::readNodeFile(filename);

		if (!island.valid()){
			osg::notify(osg::WARN) << "Could not find: " << filename << std::endl;
			return NULL;
		}

#ifdef USE_CUSTOM_SHADER
		static const char terrain_vertex[] = "terrain.vert";
		static const char terrain_fragment[] = "terrain.frag";

		osg::Program* program = osgOcean::ShaderManager::instance().createProgram("terrain", terrain_vertex, terrain_fragment, true);
		program->addBindAttribLocation("aTangent", 6);
#endif
		island->setNodeMask(_oceanScene->getNormalSceneMask() | _oceanScene->getReflectedSceneMask() | _oceanScene->getRefractedSceneMask());
		island->getStateSet()->addUniform(new osg::Uniform("uTextureMap", 0));

#ifdef USE_CUSTOM_SHADER
		island->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);
		island->getStateSet()->addUniform(new osg::Uniform("uOverlayMap", 1));
		island->getStateSet()->addUniform(new osg::Uniform("uNormalMap", 2));
#endif
		osg::PositionAttitudeTransform* islandpat = new osg::PositionAttitudeTransform;
		islandpat->setPosition(osg::Vec3f(-island->getBound().center() + osg::Vec3f(0.0, 0.0, -15.f)));
		islandpat->setScale(osg::Vec3f(4.f, 4.f, 3.f));
		islandpat->addChild(island.get());

		return islandpat;
	}

	osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTextures(const std::string& dir)
	{
		enum { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

		std::string filenames[6];

		filenames[POS_X] = "resources/textures/" + dir + "/east.png";
		filenames[NEG_X] = "resources/textures/" + dir + "/west.png";
		filenames[POS_Z] = "resources/textures/" + dir + "/north.png";
		filenames[NEG_Z] = "resources/textures/" + dir + "/south.png";
		filenames[POS_Y] = "resources/textures/" + dir + "/down.png";
		filenames[NEG_Y] = "resources/textures/" + dir + "/up.png";

		osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
		cubeMap->setInternalFormat(GL_RGBA);

		cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
		cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile(filenames[NEG_X]));
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile(filenames[POS_X]));
		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile(filenames[NEG_Y]));
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile(filenames[POS_Y]));
		cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile(filenames[NEG_Z]));
		cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile(filenames[POS_Z]));

		return cubeMap;
	}

	osg::Geode* sunDebug(const osg::Vec3f& position)
	{
		osg::ShapeDrawable* sphereDraw = new osg::ShapeDrawable(new osg::Sphere(position, 15.f));
		sphereDraw->setColor(osg::Vec4f(1.f, 0.f, 0.f, 1.f));

		osg::Geode* sphereGeode = new osg::Geode;
		sphereGeode->addDrawable(sphereDraw);

		return sphereGeode;
	}

	osg::Vec4f intColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a = 255)
	{
		float div = 1.f / 255.f;
		return osg::Vec4f(div*(float)r, div*(float)g, div*float(b), div*(float)a);
	}

	osgOcean::OceanScene::EventHandler* getOceanSceneEventHandler()
	{
		return _oceanScene->getEventHandler();
	}
};

// ----------------------------------------------------
//                   Event Handler
// ----------------------------------------------------

class SceneEventHandler : public osgGA::GUIEventHandler
{
private:
	osg::ref_ptr<SceneModel> _scene;
	osg::ref_ptr<TextHUD> _textHUD;
	osgViewer::Viewer& _viewer;

	enum CameraMode
	{
		FIXED,
		FLIGHT,
		TRACKBALL
	};

	CameraMode _currentCameraMode;

public:
	SceneEventHandler(SceneModel* scene, TextHUD* textHUD, osgViewer::Viewer& viewer) :
		_scene(scene),
		_textHUD(textHUD),
		_viewer(viewer),
		_currentCameraMode(FIXED)
	{
		_textHUD->setSceneText("Clear");
		_textHUD->setCameraText("FIXED");

		osg::Vec3f eye(0.f, 0.f, 20.f);
		osg::Vec3f centre = eye + osg::Vec3f(0.f, 1.f, 0.f);
		osg::Vec3f up(0.f, 0.f, 1.f);

		_viewer.getCamera()->setViewMatrixAsLookAt(eye, centre, up);
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
	{
		switch (ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::KEYUP) :
		{
			if (ea.getKey() == '1')
			{
				_scene->changeScene(SceneModel::CLEAR);
				_textHUD->setSceneText("Clear Blue Sky");
				return false;
			}
			else if (ea.getKey() == '2')
			{
				_scene->changeScene(SceneModel::DUSK);
				_textHUD->setSceneText("Dusk");
				return false;
			}
			else if (ea.getKey() == '3')
			{
				_scene->changeScene(SceneModel::CLOUDY);
				_textHUD->setSceneText("Pacific Cloudy");
				return false;
			}
			else if (ea.getKey() == 'C' || ea.getKey() == 'c')
			{
				if (_currentCameraMode == FIXED)
				{
					_currentCameraMode = FLIGHT;
					osgGA::FlightManipulator* flight = new osgGA::FlightManipulator;
					flight->setHomePosition(osg::Vec3f(0.f, 0.f, 20.f), osg::Vec3f(0.f, 0.f, 20.f) + osg::Vec3f(0.f, 1.f, 0.f), osg::Vec3f(0, 0, 1));
					_viewer.setCameraManipulator(flight);
					_textHUD->setCameraText("FLIGHT");
				}
				else if (_currentCameraMode == FLIGHT)
				{
					_currentCameraMode = TRACKBALL;
					osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator;
					tb->setHomePosition(osg::Vec3f(0.f, 0.f, 20.f), osg::Vec3f(0.f, 20.f, 20.f), osg::Vec3f(0, 0, 1));
					_viewer.setCameraManipulator(tb);
					_textHUD->setCameraText("TRACKBALL");
				}
				else if (_currentCameraMode == TRACKBALL)
				{
					_currentCameraMode = FIXED;
					_viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3f(0.f, 0.f, 20.f), osg::Vec3f(0.f, 0.f, 20.f) + osg::Vec3f(0.f, 1.f, 0.f), osg::Vec3f(0, 0, 1));
					_viewer.setCameraManipulator(NULL);
					_textHUD->setCameraText("FIXED");
				}
			}
		}
		default:
			return false;
		}
	}

	void getUsage(osg::ApplicationUsage& usage) const
	{
		usage.addKeyboardMouseBinding("c", "Camera type (cycle through Fixed, Flight, Trackball)");
		usage.addKeyboardMouseBinding("1", "Select scene \"Clear Blue Sky\"");
		usage.addKeyboardMouseBinding("2", "Select scene \"Dusk\"");
		usage.addKeyboardMouseBinding("3", "Select scene \"Pacific Cloudy\"");
	}

};

//¶ÁÈ¡cubemap
osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTexture()
{
	osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
	cubeMap->setInternalFormat(GL_RGBA);
	cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	osg::Image* im = osgDB::readImageFile("resources/textures/sky_fair_cloudy/west.png");
	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile("resources/textures/sky_fair_cloudy/west.png"));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile("resources/textures/sky_fair_cloudy/east.png"));
	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile("resources/textures/sky_fair_cloudy/up.png"));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile("resources/textures/sky_fair_cloudy/down.png"));
	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile("resources/textures/sky_fair_cloudy/south.png"));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile("resources/textures/sky_fair_cloudy/north.png"));
	return cubeMap;
}

