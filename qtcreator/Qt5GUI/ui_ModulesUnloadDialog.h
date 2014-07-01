/********************************************************************************
** Form generated from reading UI file 'ModulesUnloadDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODULESUNLOADDIALOG_H
#define UI_MODULESUNLOADDIALOG_H

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

class Ui_ModulesUnloadDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTreeWidget *tree_loaded;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_1;
    QPushButton *button_unload;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *button_close;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *ModulesUnloadDialog)
    {
        if (ModulesUnloadDialog->objectName().isEmpty())
            ModulesUnloadDialog->setObjectName(QStringLiteral("ModulesUnloadDialog"));
        ModulesUnloadDialog->resize(453, 340);
        verticalLayout = new QVBoxLayout(ModulesUnloadDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tree_loaded = new QTreeWidget(ModulesUnloadDialog);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        tree_loaded->setHeaderItem(__qtreewidgetitem);
        tree_loaded->setObjectName(QStringLiteral("tree_loaded"));
        tree_loaded->header()->setVisible(false);

        verticalLayout->addWidget(tree_loaded);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        button_unload = new QPushButton(ModulesUnloadDialog);
        button_unload->setObjectName(QStringLiteral("button_unload"));
        button_unload->setEnabled(false);
        button_unload->setMinimumSize(QSize(128, 0));
        button_unload->setAutoDefault(false);

        horizontalLayout->addWidget(button_unload);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        button_close = new QPushButton(ModulesUnloadDialog);
        button_close->setObjectName(QStringLiteral("button_close"));
        button_close->setMinimumSize(QSize(128, 0));
        button_close->setDefault(true);

        horizontalLayout->addWidget(button_close);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ModulesUnloadDialog);

        QMetaObject::connectSlotsByName(ModulesUnloadDialog);
    } // setupUi

    void retranslateUi(QDialog *ModulesUnloadDialog)
    {
        ModulesUnloadDialog->setWindowTitle(QApplication::translate("ModulesUnloadDialog", "Loaded Modules", 0));
        button_unload->setText(QApplication::translate("ModulesUnloadDialog", "&Unload Module", 0));
        button_close->setText(QApplication::translate("ModulesUnloadDialog", "&Close", 0));
    } // retranslateUi

};

namespace Ui {
    class ModulesUnloadDialog: public Ui_ModulesUnloadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODULESUNLOADDIALOG_H
