#include "OSGProfile.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include "common.h"

OSGProfile::OSGProfile(QWidget *parent)
	: QWidget(parent)
{
	//setWindowFlags(Qt::WindowMinimizeButtonHint /*| Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint*/);
	//this->setStyleSheet("QDialog{border:4px solid red;}");

	ui.setupUi(this);
	init();
	connectWidgets();
	this->show();
}

OSGProfile::~OSGProfile()
{

}

void OSGProfile::init()
{
	QVBoxLayout* v = new QVBoxLayout;
	m_pView = new GraphicsView("./ImageTest.earth", this);
	v->setContentsMargins(0, 0, 0, 0); 
	hLayout = new QHBoxLayout;
	v->addLayout(hLayout);
	v->addWidget(m_pView);
	this->setLayout(v);

	plong = new QLineEdit("long");
	plat = new QLineEdit("lat");
	palt = new QLineEdit("alt");
	pGo = new QPushButton("Go");
	
	comboLabel = new QLabel("Locate");
	m_pComboBox = new QComboBox;
	pLoadBtn = new QPushButton("Loading");

	testTitle = new QLabel("Pick Handler");
	testLabel = new QLabel("");
	hLayout->addWidget(plong);
	hLayout->addWidget(plat);
	hLayout->addWidget(palt);
	hLayout->addWidget(pGo);

	hLayout->addWidget(comboLabel);
	hLayout->addWidget(m_pComboBox);
	hLayout->addWidget(pLoadBtn);
	hLayout->addWidget(testTitle);
	hLayout->addWidget(testLabel);
	hLayout->addStretch(1);
}

void OSGProfile::connectWidgets()
{
	connect(pGo, SIGNAL(clicked()), this, SLOT(slot_setViewPort()));
	connect(pLoadBtn, SIGNAL(clicked()), this, SLOT(slot_loadingTifFile()));
	connect(m_pComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_currentIndexChanged(QString)));

	connect(m_pView, SIGNAL(signal_pickNodeName(std::string)), this, SIGNAL(signal_pickEarthNodeName(std::string)));
	connect(m_pView, SIGNAL(signal_pickPosition(double, double, double)), this, SIGNAL(signal_pickEarthPosition(double, double, double)));

	connect(m_pView, SIGNAL(signal_pickNodeName(std::string)), this, SLOT(slot_pickEarthNodeName(std::string)));
	connect(m_pView, SIGNAL(signal_pickPosition(double, double, double)), this, SLOT(slot_pickEarthPosition(double, double, double)));
}

QSize OSGProfile::sizeHint()
{
	return QSize(1200,1200);
}

void OSGProfile::slot_setViewPort()
{
	m_pView->setEMViewPort(plong->text().toDouble(), plat->text().toDouble());
}

void OSGProfile::slot_loadingTifFile()
{
	QString strPath = QFileDialog::getOpenFileName(this, tr("Loading tif File"), "./map/maps/", tr("Images (*.png *.tif *.jpg)"));
	if (m_pView->addImage(strPath.toStdString()))
	{
		QMessageBox::about(this, "", tr("Loading successful!"));
		m_pComboBox->addItem(QString::fromStdString(COMMON::getFilePathName(strPath.toStdString())));
	}
}

void OSGProfile::slot_currentIndexChanged(QString _name)
{
	m_pView->setEMViewPortByName(_name.toStdString());
}

void OSGProfile::slot_pickEarthNodeName(std::string name)
{
	testTitle->setText(QString::fromStdString(name));
}

void OSGProfile::slot_pickEarthPosition(double lng, double lat, double alt)
{
	testLabel->setText(QString::number(lng) + " " + QString::number(lat));
}
