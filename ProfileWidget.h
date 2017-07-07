#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QMainWindow>
#include "graphicsview.h"
#include "MyOSG.h"
#include "EventAdapter.h"

class ProfileWidget : public QMainWindow
{
	Q_OBJECT

public:
	ProfileWidget(QWidget *parent = NULL);
	~ProfileWidget();

	void init();

protected:

private:
	GraphicsView* m_pView;
};

#endif // PROFILEWIDGET_H
