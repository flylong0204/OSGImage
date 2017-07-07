#ifndef OSGPROFILE_H
#define OSGPROFILE_H

#include <QtWidgets/QWidget>
#include <QSize>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "ui_OSGProfile.h"
#include "graphicsview.h"
//#include "ProfileWidget.h"


class OSGProfile : public QWidget
{
	Q_OBJECT

public:
	OSGProfile(QWidget *parent = 0);
	~OSGProfile();

protected:
	void init();
	void connectWidgets();
	virtual QSize sizeHint();

public slots:
	void slot_setViewPort();
	void slot_loadingTifFile();
	void slot_currentIndexChanged(QString);
	void slot_pickEarthNodeName(std::string name);
	void slot_pickEarthPosition(double lng, double lat, double alt);

signals:
	void signal_pickEarthNodeName(std::string name);
	void signal_pickEarthPosition(double lng, double lat, double alt);
private:
	Ui::OSGProfileClass ui;
	GraphicsView* m_pView;
	QHBoxLayout* hLayout;
	QLineEdit* plong, *plat, *palt;
	QPushButton* pGo, *pLoadBtn;
	QComboBox* m_pComboBox;
	QLabel* comboLabel, *testLabel, *testTitle;
};

#endif // OSGPROFILE_H
