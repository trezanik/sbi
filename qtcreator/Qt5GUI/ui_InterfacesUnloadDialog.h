/********************************************************************************
** Form generated from reading UI file 'InterfacesUnloadDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERFACESUNLOADDIALOG_H
#define UI_INTERFACESUNLOADDIALOG_H

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

class Ui_InterfacesUnloadDialog
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

    void setupUi(QDialog *InterfacesUnloadDialog)
    {
        if (InterfacesUnloadDialog->objectName().isEmpty())
            InterfacesUnloadDialog->setObjectName(QStringLiteral("InterfacesUnloadDialog"));
        InterfacesUnloadDialog->resize(453, 340);
        verticalLayout = new QVBoxLayout(InterfacesUnloadDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tree_loaded = new QTreeWidget(InterfacesUnloadDialog);
        tree_loaded->setObjectName(QStringLiteral("tree_loaded"));
        tree_loaded->header()->setVisible(false);

        verticalLayout->addWidget(tree_loaded);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        button_unload = new QPushButton(InterfacesUnloadDialog);
        button_unload->setObjectName(QStringLiteral("button_unload"));
        button_unload->setEnabled(false);
        button_unload->setMinimumSize(QSize(128, 0));
        button_unload->setAutoDefault(false);

        horizontalLayout->addWidget(button_unload);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        button_close = new QPushButton(InterfacesUnloadDialog);
        button_close->setObjectName(QStringLiteral("button_close"));
        button_close->setMinimumSize(QSize(128, 0));
        button_close->setDefault(true);

        horizontalLayout->addWidget(button_close);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(InterfacesUnloadDialog);

        QMetaObject::connectSlotsByName(InterfacesUnloadDialog);
    } // setupUi

    void retranslateUi(QDialog *InterfacesUnloadDialog)
    {
        InterfacesUnloadDialog->setWindowTitle(QApplication::translate("InterfacesUnloadDialog", "Loaded Interfaces", 0));
        button_unload->setText(QApplication::translate("InterfacesUnloadDialog", "&Unload Interface", 0));
        button_close->setText(QApplication::translate("InterfacesUnloadDialog", "&Close", 0));
    } // retranslateUi

};

namespace Ui {
    class InterfacesUnloadDialog: public Ui_InterfacesUnloadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERFACESUNLOADDIALOG_H
