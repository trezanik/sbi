/********************************************************************************
** Form generated from reading UI file 'ModulesLoadDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODULESLOADDIALOG_H
#define UI_MODULESLOADDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ModulesLoadDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTreeWidget *tree_available;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_1;
    QPushButton *button_load;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *button_close;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *ModulesLoadDialog)
    {
        if (ModulesLoadDialog->objectName().isEmpty())
            ModulesLoadDialog->setObjectName(QStringLiteral("ModulesLoadDialog"));
        ModulesLoadDialog->resize(453, 340);
        verticalLayout = new QVBoxLayout(ModulesLoadDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tree_available = new QTreeWidget(ModulesLoadDialog);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        tree_available->setHeaderItem(__qtreewidgetitem);
        tree_available->setObjectName(QStringLiteral("tree_available"));
        tree_available->header()->setVisible(false);

        verticalLayout->addWidget(tree_available);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        button_load = new QPushButton(ModulesLoadDialog);
        button_load->setObjectName(QStringLiteral("button_load"));
        button_load->setEnabled(false);
        button_load->setMinimumSize(QSize(128, 0));
        button_load->setAutoDefault(false);

        horizontalLayout->addWidget(button_load);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        button_close = new QPushButton(ModulesLoadDialog);
        button_close->setObjectName(QStringLiteral("button_close"));
        button_close->setMinimumSize(QSize(128, 0));
        button_close->setDefault(true);

        horizontalLayout->addWidget(button_close);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ModulesLoadDialog);

        QMetaObject::connectSlotsByName(ModulesLoadDialog);
    } // setupUi

    void retranslateUi(QDialog *ModulesLoadDialog)
    {
        ModulesLoadDialog->setWindowTitle(QApplication::translate("ModulesLoadDialog", "Available Modules", 0));
        button_load->setText(QApplication::translate("ModulesLoadDialog", "&Load Module", 0));
        button_close->setText(QApplication::translate("ModulesLoadDialog", "&Close", 0));
    } // retranslateUi

};

namespace Ui {
    class ModulesLoadDialog: public Ui_ModulesLoadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODULESLOADDIALOG_H
