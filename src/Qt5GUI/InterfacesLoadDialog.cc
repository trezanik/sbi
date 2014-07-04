
/**
 * @file	InterfacesLoadDialog.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "generated/ui_InterfacesLoadDialog.h"	// uic generated header
#include "InterfacesLoadDialog.h"		// prototypes
#include "UI.h"					// UI core
#include <api/version.h>
#include <api/utils.h>
#include <api/Log.h>
#include <api/Configuration.h>
#include <api/Runtime.h>


using namespace APP_NAMESPACE;



InterfacesLoadDialog::InterfacesLoadDialog(
	QWidget* parent
)
: QDialog(parent), _dlg(new Ui_InterfacesLoadDialog)
{
	_dlg->setupUi(this);
	// prevent resizing
	setFixedSize(size());

	QObject::connect(_dlg->button_close, SIGNAL(clicked()), this, SLOT(close()));
	QObject::connect(_dlg->button_load, SIGNAL(clicked()), this, SLOT(OnLoadInterface()));
	QObject::connect(_dlg->tree_available, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(OnSelectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}



InterfacesLoadDialog::~InterfacesLoadDialog()
{
}



void
InterfacesLoadDialog::OnClose()
{
	close();
}



void
InterfacesLoadDialog::OnLoadInterface()
{
	QTreeWidgetItem*	current_item = _dlg->tree_available->currentItem();
	char*			item_text;
	const uint32_t		column = 0;
	QByteArray		ba;
	int			spawn_res;

	if ( current_item == nullptr )
		return;

	_dlg->button_load->setEnabled(false);

	ba = current_item->text(column).toUtf8();
	item_text = ba.data();

	for ( auto t : _avail_interfaces )
	{
#if 0
		if ( t->file_name.compare(item_text) == 0 )
		{
			// got the file; execute the spawn function
			if ( (spawn_res = t->pf_spawn_interface()) != (int)EInterfaceStatus::Ok )
			{
				// todo: interface status to error str, msg box
				LOG(ELogLevel::Error) << "spawn_interface failed\n";
				return;
			}

			current_item->parent()->removeChild(current_item);
			// let items be deleted on dialog closure automatically
			return;
		}
#endif
	}

	LOG(ELogLevel::Error) << "None of the interface items matched the one to load ("
		<< item_text << ")!\n";
}



void
InterfacesLoadDialog::OnSelectionChanged(
	QTreeWidgetItem* current_item,
	QTreeWidgetItem* previous_item
)
{
	bool	enable_state = true;

	if ( current_item == nullptr )
		enable_state = false;

	/* if it's a top level item (i.e. the directory identifiers), then we
	 * can't load that, so disable the load button. */
	if ( current_item->parent() == nullptr )
		enable_state = false;

	_dlg->button_load->setEnabled(enable_state);
}



void
InterfacesLoadDialog::SetModel(
	const UI* model
)
{
	std::vector<QTreeWidgetItem*>	root_items;
	QTreeWidgetItem*		current_top = nullptr;
	QTreeWidgetItem*		toplvl = nullptr;
	QByteArray			ba;
	Configuration*			cfg = runtime.Config();

	// we're loading, use this opportunity to populate interfaces
	_avail_interfaces = get_available_interfaces();

	for ( auto t : _avail_interfaces )
	{
		QTreeWidgetItem*	twi = new QTreeWidgetItem;

		/* get source, add as top level item if it doesn't exist, else
		 * use the existing one if it's still the same name */

		if ( current_top != nullptr )
		{
			ba = current_top->text(0).toUtf8();

			if ( strcmp(ba.data(), t->group.c_str()) != 0 )
			{
				toplvl = new QTreeWidgetItem;
				current_top = toplvl;
				toplvl->setText(0, t->group.c_str());
				root_items.push_back(toplvl);
				
			}
		}
		else if ( toplvl == nullptr || current_top != toplvl )
		{
			toplvl = new QTreeWidgetItem;
			current_top = toplvl;
			toplvl->setText(0, t->group.c_str());
			root_items.push_back(toplvl);
		}

		// add the interface to a new item, current top level
		twi->setText(0, t->file_name.c_str());
		toplvl->addChild(twi);
	}

	// add search paths even if they contain no entries (for GUI)
	for ( auto c : cfg->interfaces.search_paths.data )
	{
		bool	present = false;

		for ( auto i : root_items )
		{
			ba = i->text(0).toUtf8();

			if ( strcmp(c[0].c_str(), ba.data()) == 0 )
			{
				present = true;
				break;
			}
		}

		if ( !present )
		{
			toplvl = new QTreeWidgetItem;
			toplvl->setText(0, c.c_str());
			root_items.push_back(toplvl);
		}
	}
		
	// add all the actual root items to the tree
	for ( auto i : root_items )
	{
		_dlg->tree_available->addTopLevelItem(i);
	}
		
	_dlg->tree_available->expandAll();
}

