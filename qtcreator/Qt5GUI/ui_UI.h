/********************************************************************************
** Form generated from reading UI file 'UI.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_H
#define UI_UI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionE_xit;
    QAction *action_ModuleLoad;
    QAction *action_ModuleUnload;
    QAction *action_About;
    QAction *actionAbout_Qt;
    QAction *action_Documentation;
    QAction *action_InterfaceLoad;
    QAction *action_InterfaceUnload;
    QAction *action_Preferences;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QPushButton *button_home;
    QTreeWidget *tree_interfaces;
    QStackedWidget *stacked_widget;
    QWidget *page;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *text_display;
    QWidget *page_2;
    QLineEdit *lineEdit;
    QMenuBar *menubar;
    QMenu *menu_SBI;
    QMenu *menu_Modules;
    QMenu *menu_Help;
    QMenu *menu_Interfaces;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(820, 554);
        actionE_xit = new QAction(MainWindow);
        actionE_xit->setObjectName(QStringLiteral("actionE_xit"));
        action_ModuleLoad = new QAction(MainWindow);
        action_ModuleLoad->setObjectName(QStringLiteral("action_ModuleLoad"));
        action_ModuleLoad->setEnabled(true);
        action_ModuleUnload = new QAction(MainWindow);
        action_ModuleUnload->setObjectName(QStringLiteral("action_ModuleUnload"));
        action_ModuleUnload->setEnabled(false);
        action_About = new QAction(MainWindow);
        action_About->setObjectName(QStringLiteral("action_About"));
        actionAbout_Qt = new QAction(MainWindow);
        actionAbout_Qt->setObjectName(QStringLiteral("actionAbout_Qt"));
        action_Documentation = new QAction(MainWindow);
        action_Documentation->setObjectName(QStringLiteral("action_Documentation"));
        action_InterfaceLoad = new QAction(MainWindow);
        action_InterfaceLoad->setObjectName(QStringLiteral("action_InterfaceLoad"));
        action_InterfaceUnload = new QAction(MainWindow);
        action_InterfaceUnload->setObjectName(QStringLiteral("action_InterfaceUnload"));
        action_InterfaceUnload->setEnabled(false);
        action_Preferences = new QAction(MainWindow);
        action_Preferences->setObjectName(QStringLiteral("action_Preferences"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        button_home = new QPushButton(centralwidget);
        button_home->setObjectName(QStringLiteral("button_home"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(button_home->sizePolicy().hasHeightForWidth());
        button_home->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(button_home);

        tree_interfaces = new QTreeWidget(centralwidget);
        tree_interfaces->setObjectName(QStringLiteral("tree_interfaces"));
        tree_interfaces->setFrameShape(QFrame::NoFrame);
        tree_interfaces->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tree_interfaces->setUniformRowHeights(true);

        verticalLayout_2->addWidget(tree_interfaces);


        horizontalLayout_2->addLayout(verticalLayout_2);

        stacked_widget = new QStackedWidget(centralwidget);
        stacked_widget->setObjectName(QStringLiteral("stacked_widget"));
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(page->sizePolicy().hasHeightForWidth());
        page->setSizePolicy(sizePolicy1);
        verticalLayout_4 = new QVBoxLayout(page);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        text_display = new QTextEdit(page);
        text_display->setObjectName(QStringLiteral("text_display"));

        verticalLayout_3->addWidget(text_display);


        verticalLayout_4->addLayout(verticalLayout_3);

        stacked_widget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        stacked_widget->addWidget(page_2);

        horizontalLayout_2->addWidget(stacked_widget);

        horizontalLayout_2->setStretch(0, 1);
        horizontalLayout_2->setStretch(1, 4);

        verticalLayout->addLayout(horizontalLayout_2);

        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        verticalLayout->addWidget(lineEdit);

        verticalLayout->setStretch(0, 10);

        horizontalLayout->addLayout(verticalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 820, 21));
        menu_SBI = new QMenu(menubar);
        menu_SBI->setObjectName(QStringLiteral("menu_SBI"));
        menu_Modules = new QMenu(menubar);
        menu_Modules->setObjectName(QStringLiteral("menu_Modules"));
        menu_Modules->setEnabled(false);
        menu_Help = new QMenu(menubar);
        menu_Help->setObjectName(QStringLiteral("menu_Help"));
        menu_Interfaces = new QMenu(menubar);
        menu_Interfaces->setObjectName(QStringLiteral("menu_Interfaces"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_SBI->menuAction());
        menubar->addAction(menu_Interfaces->menuAction());
        menubar->addAction(menu_Modules->menuAction());
        menubar->addAction(menu_Help->menuAction());
        menu_SBI->addAction(action_Preferences);
        menu_SBI->addSeparator();
        menu_SBI->addAction(actionE_xit);
        menu_Modules->addAction(action_ModuleLoad);
        menu_Modules->addAction(action_ModuleUnload);
        menu_Modules->addSeparator();
        menu_Help->addAction(action_About);
        menu_Help->addAction(actionAbout_Qt);
        menu_Help->addSeparator();
        menu_Help->addAction(action_Documentation);
        menu_Interfaces->addAction(action_InterfaceLoad);
        menu_Interfaces->addAction(action_InterfaceUnload);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionE_xit->setText(QApplication::translate("MainWindow", "E&xit", 0));
        action_ModuleLoad->setText(QApplication::translate("MainWindow", "&Load..", 0));
        action_ModuleUnload->setText(QApplication::translate("MainWindow", "&Unload..", 0));
        action_About->setText(QApplication::translate("MainWindow", "&About", 0));
        actionAbout_Qt->setText(QApplication::translate("MainWindow", "About &Qt", 0));
        action_Documentation->setText(QApplication::translate("MainWindow", "&Documentation", 0));
        action_InterfaceLoad->setText(QApplication::translate("MainWindow", "&Load..", 0));
        action_InterfaceUnload->setText(QApplication::translate("MainWindow", "Unload..", 0));
        action_Preferences->setText(QApplication::translate("MainWindow", "&Preferences", 0));
        button_home->setText(QApplication::translate("MainWindow", "&Home Screen", 0));
        QTreeWidgetItem *___qtreewidgetitem = tree_interfaces->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("MainWindow", "Interfaces", 0));
        menu_SBI->setTitle(QApplication::translate("MainWindow", "&SBI", 0));
        menu_Modules->setTitle(QApplication::translate("MainWindow", "&Modules", 0));
        menu_Help->setTitle(QApplication::translate("MainWindow", "&Help", 0));
        menu_Interfaces->setTitle(QApplication::translate("MainWindow", "&Interfaces", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UI_H
