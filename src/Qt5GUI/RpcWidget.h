#pragma once

/**
 * @file	src/Qt5GUI/RpcWidget.h
 * @author	James Warren
 * @brief	Template class to wrap QWidgets with an ID
 */



#include <atomic>

#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qwidget.h>

#include <api/definitions.h>
#include <api/Log.h>

#include "UI.h"



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)


// UI.cc
extern std::atomic_uint		g_rpc_widget_id;



/**
 * Template class used to give all created QWidgets from RPC calls a unique ID,
 * which is used to access the window directly from remote, rather than calling
 * complex methods to find a window.
 *
 * Naming convention is simple: Rpc<OriginalQtWidgetName>, e.g.
 * - QLabel = RpcQLabel
 * - QLineEdit = RpcQLineEdit
 * - etc.
 *
 * @class RpcWidget
 */
template <class T>
class RpcWidget : public T
{
private:

	uint32_t	_id;

public:
	RpcWidget(
		QWidget* parent = nullptr
	)
	: T(parent)
	{
		_id = g_rpc_widget_id++;

		if ( _id == 0 )
		{
			throw std::runtime_error("ID overflow");
		}

		LOG(ELogLevel::Debug) << "New RpcWidget created (id=" << _id << ")\n";
	}


	uint32_t
	ID() const
	{
		return _id;
	}

};



typedef RpcWidget<QLabel>		RpcQLabel;
typedef RpcWidget<QLineEdit>		RpcQLineEdit;
typedef RpcWidget<QPushButton>		RpcQPushButton;
typedef RpcWidget<QStackedWidget>	RpcQStackedWidget;
typedef RpcWidget<QWidget>		RpcQWidget;



END_NAMESPACE
END_NAMESPACE
