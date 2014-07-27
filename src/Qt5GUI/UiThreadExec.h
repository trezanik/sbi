#pragma once

/**
 * @file	src/Qt5GUI/UiThreadExec.h
 * @author	James Warren
 * @brief	Class that handles GUI objects to be created in the UI thread
 */



#include <cassert>

#include <QtCore/qobject.h>
#include <QtCore/qthread.h>
#include <QtWidgets/qapplication.h>

#include <api/Allocator.h>

#include "RpcWidget.h"
#include "UI.h"



using namespace APP_NAMESPACE;
using namespace GUI_NAMESPACE;


struct window_params
{
	// The type of window to be created
	uint64_t	window_type;
	// Text to display, if applicable for the window type
	std::string	text;
	// Pointer to direct owner (usually page)
	uintptr_t	parent;
	// Pointer to QStackedWidget page
	uintptr_t	page;
};



class UiThreadExec : public QThread
{
	Q_OBJECT
private:
	NO_CLASS_ASSIGNMENT(UiThreadExec);
	NO_CLASS_COPY(UiThreadExec);


	std::shared_ptr<window_params>	_params;

public:
	UiThreadExec(
		std::shared_ptr<window_params> p
	) : _params(p)
	{
	}
	~UiThreadExec()
	{
	}


public slots:

	void
	do_create()
	{
		// ensure we are actually the GUI thread
		assert(currentThread() == QCoreApplication::instance()->thread());
		g_ui->CreateWindow(_params.get());
	}


signals:

	void
	start_create();

};
