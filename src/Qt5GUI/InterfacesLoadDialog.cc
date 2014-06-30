
/**
 * @file	InterfacesLoadDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_InterfacesLoadDialog.h"	// uic generated header
#include "InterfacesLoadDialog.h"		// prototypes
#include "UI.h"					// UI core
#include <api/version.h>
#include <api/interfaces.h>




InterfacesLoadDialog::InterfacesLoadDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_InterfacesLoadDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());
}



InterfacesLoadDialog::~InterfacesLoadDialog()
{
}



void
InterfacesLoadDialog::SetModel(
	const UI* model
)
{
	using namespace APP_NAMESPACE;
	std::vector<std::string>	avail_interfaces;
	QTreeWidgetItem*		toplvl = new QTreeWidgetItem;
	
	toplvl->setText(0, tr("Current Directory"));

	// while we're here...
	avail_interfaces = get_available_interfaces();

	for ( auto t : avail_interfaces )
	{
		QTreeWidgetItem*	twi = new QTreeWidgetItem;

		twi->setText(0, t.c_str());
		toplvl->addChild(twi);
	}

	_dlg->tree_available->addTopLevelItem(toplvl);
	//_dlg->tree_available->insertTopLevelItem(twi,i);
	_dlg->tree_available->expandAll();
}



void
InterfacesLoadDialog::OnButtonBoxAccepted()
{
	close();
}
