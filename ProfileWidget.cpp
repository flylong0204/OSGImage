#include "ProfileWidget.h"
#include <QHBoxLayout>

ProfileWidget::ProfileWidget(QWidget *parent)
	: QMainWindow(parent)
{
	init();
}

ProfileWidget::~ProfileWidget()
{

}

void ProfileWidget::init()
{
	m_pView = new GraphicsView("map/maps/myearth.earth", this);
	this->setCentralWidget(m_pView);
}

