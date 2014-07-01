
/**
 * @file	ModulesLoadDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_ModulesLoadDialog.h"	// uic generated header
#include "ModulesLoadDialog.h"			// prototypes
#include "UI.h"					// UI core
#include <api/version.h>




ModulesLoadDialog::ModulesLoadDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_ModulesLoadDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());
}



ModulesLoadDialog::~ModulesLoadDialog()
{
}



void
ModulesLoadDialog::SetModel(
	const UI* model
)
{
	
}



void
ModulesLoadDialog::OnButtonBoxAccepted()
{
	close();
}
