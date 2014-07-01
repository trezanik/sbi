
/**
 * @file	ModulesUnloadDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_ModulesUnloadDialog.h"	// uic generated header
#include "ModulesUnloadDialog.h"		// prototypes
#include "UI.h"					// UI core
#include <api/version.h>




ModulesUnloadDialog::ModulesUnloadDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_ModulesUnloadDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());
}



ModulesUnloadDialog::~ModulesUnloadDialog()
{
}



void
ModulesUnloadDialog::SetModel(
	const UI* model
)
{

}



void
ModulesUnloadDialog::OnButtonBoxAccepted()
{
	close();
}
