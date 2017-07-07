#include "pickHandle.h"
#include <osgViewer/Viewer>
#include <osg/Material>
#include <osgEarth/TerrainEngineNode>
#include <osgEarth/IntersectionPicker>
#include <QDebug>
#include "MyOSGEarth.h"

PickHandle::PickHandle(osg::ref_ptr<osgViewer::Viewer> viewer, osgEarth::MapNode* _mapNode, osg::ref_ptr<osg::Group> _pPlaceNodeRoot) : m_pviewer(viewer), m_pMapNode(_mapNode), m_pPlaceNodeRoot(_pPlaceNodeRoot), lastSelect(NULL)
{
	if (m_pMapNode)
	{
		terrain = m_pMapNode->getTerrain();
	}

	
}

PickHandle::~PickHandle(){}

bool PickHandle::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	switch (ea.getEventType())
	{
		/*case osgGA::GUIEventAdapter::PUSH:
		if (ea.getButton() == 1)
		{
		Pick(ea.getX(),ea.getY());
		}
		return true;*/
		case osgGA::GUIEventAdapter::PUSH:
		{
			//if (lastSelect)
		//{
		//	osg::StateSet* state = lastSelect->getOrCreateStateSet();
		//	state->setMode(GL_BLEND, osg::StateAttribute::ON);				//Blend 打开混合色
		//	//状态属性类 osg::Material 封装了 OpenGL 的 glMaterial()和 glColorMaterial()
		//	//指令的函数功能。要在用户应用程序中设置材质属性，首先要创建一个 Material
		//	//对象，设置颜色和其它参数，然后关联到场景图形的 StateSet 中。
		//	osg::Material* mtrl = dynamic_cast<osg::Material *> (state->getAttribute(osg::StateAttribute::MATERIAL));
		//	mtrl->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));		//散射颜色
		//	mtrl->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
		//	mtrl->setTransparency(osg::Material::FRONT_AND_BACK, 0.0);
		//
		//	state->setAttributeAndModes(mtrl, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
		//	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		//	lastSelect = 0;
		//}

			Pick(ea.getX(), ea.getY());
		}
	//return true;
	}
	return false;
}


void PickHandle::Pick(float x, float y)
{
	osgUtil::LineSegmentIntersector::Intersections intersections;
	osgUtil::LineSegmentIntersector::Intersections::iterator itor;
	if (m_pviewer->computeIntersections(x, y, intersections))
	{
		for (itor = intersections.begin(); itor != intersections.end(); ++itor)
		{
			if (!itor->nodePath.empty() && !(itor->nodePath.back()->getName().empty()))
			{
				const osg::NodePath& np = itor->nodePath;

				for (int i = np.size() - 1; i >= 0; --i)
				{
					osg::Node* nd = dynamic_cast<osg::Node *> (np[i]);
					if (nd != NULL)
					{
						std::string name = nd->getName();

						if (m_pPlaceNodeRoot)
						{
							for (size_t i = 0; i < m_pPlaceNodeRoot->getNumChildren() - 1; i++)
							{
								if (nd->getParent(0))
								{
									std::string placeName = nd->getParent(0)->asGroup()->getName();
									if (placeName == m_pPlaceNodeRoot->getChild(i)->getName())
									{
										
										osg::ref_ptr<osgEarth::Annotation::PlaceNode> placeNode = dynamic_cast<osgEarth::Annotation::PlaceNode*>(m_pPlaceNodeRoot->getChild(i));

										//to do...

										//osgUtil::LineSegmentIntersector::Intersection first = *(intersections.begin());
										osg::Vec3d vec3;
										terrain->getWorldCoordsUnderMouse(m_pviewer, x, y, vec3);
										osgEarth::GeoPoint point;
										point.fromWorld(m_pMapNode->getMapSRS(), vec3);

										signal_pickedPosition(point.x(), point.y(), point.z());
										signal_pickedNodeName(placeName);
										qDebug() << QString::fromStdString(nd->getParent(0)->getName());
										qDebug() << point.x() << point.y() << point.z();
										return;
									}
								}

							}
						}
						//std::string name = nd->getName();
						//if (nd->getName() == "name_UAV" || nd->getName() == "name_Ship" || nd->getName() == "name_Vehicle")
						//{
						//	osg::StateSet *state = nd->getOrCreateStateSet();
						//	state->setMode(GL_BLEND, osg::StateAttribute::ON);
						//
						//	osg::Material* mtrl = dynamic_cast<osg::Material *> (state->getAttribute(osg::StateAttribute::MATERIAL));
						//	if (!mtrl)
						//	{
						//		mtrl = new osg::Material;
						//	}
						//
						//	mtrl->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 0.0, 0.0, 0.8));
						//	mtrl->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 0.0, 0.0, 0.8));
						//	mtrl->setTransparency(osg::Material::FRONT_AND_BACK, 0.2);
						//
						//	state->setAttributeAndModes(mtrl, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
						//	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
						//
						//	lastSelect = nd;
						//
						//	return;
						//}
					}
				}
			}
		}
	}
	return;
}

