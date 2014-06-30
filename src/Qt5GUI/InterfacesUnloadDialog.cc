
/**
 * @file	InterfacesUnloadDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_InterfacesUnloadDialog.h"	// uic generated header
#include "InterfacesUnloadDialog.h"			// prototypes
#include "UI.h"						// UI core
#include <api/version.h>




InterfacesUnloadDialog::InterfacesUnloadDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_InterfacesUnloadDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());
}



InterfacesUnloadDialog::~InterfacesUnloadDialog()
{
}



void
InterfacesUnloadDialog::SetModel(
	const UI* model
)
{
	
}



void
InterfacesUnloadDialog::OnButtonBoxAccepted()
{
	close();
}
