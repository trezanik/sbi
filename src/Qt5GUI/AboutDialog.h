#pragma once

/**
 * @file	src/Qt5GUI/AboutDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'About' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>



// Qt form
class Ui_AboutDialog;
class UI;




/** 
 * "About" dialog box
 */
class AboutDialog : public QDialog
{
	Q_OBJECT

private:
	
	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_AboutDialog>		_dlg;

private slots:

	void
	OnButtonBoxAccepted();

public:
	explicit AboutDialog(
		QWidget* parent = nullptr
	);
	~AboutDialog();


	void
	SetModel(
		const UI* model
	);
};
