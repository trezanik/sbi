/********************************************************************************
** Form generated from reading UI file 'InterfacesLoadDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERFACESLOADDIALOG_H
#define UI_INTERFACESLOADDIALOG_H

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

class Ui_InterfacesLoadDialog
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

    void setupUi(QDialog *InterfacesLoadDialog)
    {
        if (InterfacesLoadDialog->objectName().isEmpty())
            InterfacesLoadDialog->setObjectName(QStringLiteral("InterfacesLoadDialog"));
        InterfacesLoadDialog->resize(453, 340);
        verticalLayout = new QVBoxLayout(InterfacesLoadDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tree_available = new QTreeWidget(InterfacesLoadDialog);
        tree_available->setObjectName(QStringLiteral("tree_available"));
        tree_available->header()->setVisible(false);

        verticalLayout->addWidget(tree_available);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        button_load = new QPushButton(InterfacesLoadDialog);
        button_load->setObjectName(QStringLiteral("button_load"));
        button_load->setEnabled(false);
        button_load->setMinimumSize(QSize(128, 0));
        button_load->setAutoDefault(false);

        horizontalLayout->addWidget(button_load);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        button_close = new QPushButton(InterfacesLoadDialog);
        button_close->setObjectName(QStringLiteral("button_close"));
        button_close->setMinimumSize(QSize(128, 0));
        button_close->setDefault(true);

        horizontalLayout->addWidget(button_close);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(InterfacesLoadDialog);

        QMetaObject::connectSlotsByName(InterfacesLoadDialog);
    } // setupUi

    void retranslateUi(QDialog *InterfacesLoadDialog)
    {
        InterfacesLoadDialog->setWindowTitle(QApplication::translate("InterfacesLoadDialog", "Available Interfaces", 0));
        button_load->setText(QApplication::translate("InterfacesLoadDialog", "&Load Interface", 0));
        button_close->setText(QApplication::translate("InterfacesLoadDialog", "&Close", 0));
    } // retranslateUi

};

namespace Ui {
    class InterfacesLoadDialog: public Ui_InterfacesLoadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERFACESLOADDIALOG_H
