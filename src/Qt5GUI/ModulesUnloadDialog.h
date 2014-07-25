#pragma once

/**
 * @file	src/Qt5GUI/ModulesUnloadDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'Unload Module' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>



// Qt form
class Ui_ModulesUnloadDialog;
class UI;




/**
*
*/
class ModulesUnloadDialog : public QDialog
{
	Q_OBJECT

private:

	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_ModulesUnloadDialog>	_dlg;

private slots:

	void
	OnButtonBoxAccepted();

public:
	explicit ModulesUnloadDialog(
		QWidget* parent = nullptr
	);
	~ModulesUnloadDialog();


	void
	SetModel(
		const UI* model
	);
};
