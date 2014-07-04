#pragma once

/**
 * @file	InterfacesLoadDialog.h
 * @author	James Warren
 * @brief	Qt5GUI 'Load Interface' dialog
 */



#include <memory>
#include <QtWidgets/QDialog>
#include <QtWidgets/qtreewidget.h>
#include <api/definitions.h>		// namespace
#include <api/interfaces.h>		// AvailableInterfaceDetails



// Qt form
class Ui_InterfacesLoadDialog;
class UI;
BEGIN_NAMESPACE(APP_NAMESPACE)
	struct AvailableInterfaceDetails;
END_NAMESPACE



/**
 *
 */
class InterfacesLoadDialog : public QDialog
{
	Q_OBJECT

private:

	/** Designed form; nothing we can do about naming convention */
	std::unique_ptr<Ui_InterfacesLoadDialog>	_dlg;
	/** storage of available interfaces that can be loaded */
	std::vector<std::shared_ptr<APP_NAMESPACE::AvailableInterfaceDetails>>	_avail_interfaces;
	// ^= interfaces_vector_t w/ namespace;

private slots:

	void
	OnClose();

	void
	OnLoadInterface();

	void
	OnSelectionChanged(
		QTreeWidgetItem* current_item,
		QTreeWidgetItem* previous_item
	);

public:
	explicit InterfacesLoadDialog(
		QWidget* parent = nullptr
	);
	~InterfacesLoadDialog();


	void
	SetModel(
		const UI* model
	);
};
