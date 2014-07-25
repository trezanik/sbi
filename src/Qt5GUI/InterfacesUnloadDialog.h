#pragma once

/**
 * @file	src/Qt5GUI/InterfacesUnloadDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'Unload Interface' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>



// Qt form
class Ui_InterfacesUnloadDialog;
class UI;




/**
*
*/
class InterfacesUnloadDialog : public QDialog
{
	Q_OBJECT

private:

	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_InterfacesUnloadDialog>	_dlg;

private slots:

	void
	OnButtonBoxAccepted();

public:
	explicit InterfacesUnloadDialog(
		QWidget* parent = nullptr
	);
	~InterfacesUnloadDialog();


	void
	SetModel(
		const UI* model
	);
};
