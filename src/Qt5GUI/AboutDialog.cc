
/**
 * @file	AboutDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_AboutDialog.h"		// uic generated header
#include "AboutDialog.h"			// prototypes
#include "UI.h"					// UI core
#include <api/version.h>




AboutDialog::AboutDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_AboutDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());
}



AboutDialog::~AboutDialog()
{
}



void
AboutDialog::SetModel(
	const UI* model
)
{
	// replace %VERSION% with app version or unspecified string
	_dlg->label_appversion->setText(model ? model->ApplicationVersion() : "unspecified");
	
}



void
AboutDialog::OnButtonBoxAccepted()
{
	close();
}
