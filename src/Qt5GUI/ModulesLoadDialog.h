#pragma once

/**
 * @file	src/Qt5GUI/ModulesLoadDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'Load Module' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>



// Qt form
class Ui_ModulesLoadDialog;
class UI;




/**
*
*/
class ModulesLoadDialog : public QDialog
{
	Q_OBJECT

private:

	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_ModulesLoadDialog>	_dlg;

private slots:

	void
	OnButtonBoxAccepted();

public:
	explicit ModulesLoadDialog(
		QWidget* parent = nullptr
	);
	~ModulesLoadDialog();


	void
	SetModel(
		const UI* model
	);
};
