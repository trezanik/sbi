#pragma once

/**
 * @file	InterfacesLoadDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'Load Interface' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>



// Qt form
class Ui_InterfacesLoadDialog;
class UI;




/**
 *
 */
class InterfacesLoadDialog : public QDialog
{
	Q_OBJECT

private:

	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_InterfacesLoadDialog>	_dlg;

private slots:

	void
	OnButtonBoxAccepted();

public:
	explicit InterfacesLoadDialog(
		QWidget* parent = nullptr
	);
	~InterfacesLoadDialog();


	void
	SetModel(
		const UI* model
	);
};
