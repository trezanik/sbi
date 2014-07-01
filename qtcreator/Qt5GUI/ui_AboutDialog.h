/********************************************************************************
** Form generated from reading UI file 'AboutDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_sideimage;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_appname;
    QLabel *label_appversion;
    QSpacerItem *horizontalSpacer;
    QLabel *label_appimage;
    QLabel *label_copyright;
    QLabel *label_notice;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *button_ok;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QStringLiteral("AboutDialog"));
        AboutDialog->resize(450, 255);
        horizontalLayout_2 = new QHBoxLayout(AboutDialog);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_sideimage = new QLabel(AboutDialog);
        label_sideimage->setObjectName(QStringLiteral("label_sideimage"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_sideimage->sizePolicy().hasHeightForWidth());
        label_sideimage->setSizePolicy(sizePolicy);
        label_sideimage->setPixmap(QPixmap(QString::fromUtf8(":/images/about")));

        horizontalLayout_2->addWidget(label_sideimage);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_appname = new QLabel(AboutDialog);
        label_appname->setObjectName(QStringLiteral("label_appname"));
        label_appname->setCursor(QCursor(Qt::IBeamCursor));
        label_appname->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        horizontalLayout->addWidget(label_appname);

        label_appversion = new QLabel(AboutDialog);
        label_appversion->setObjectName(QStringLiteral("label_appversion"));
        label_appversion->setCursor(QCursor(Qt::IBeamCursor));
        label_appversion->setText(QStringLiteral("%VERSION%"));
        label_appversion->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        horizontalLayout->addWidget(label_appversion);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_appimage = new QLabel(AboutDialog);
        label_appimage->setObjectName(QStringLiteral("label_appimage"));
        label_appimage->setMaximumSize(QSize(64, 64));
        label_appimage->setPixmap(QPixmap(QString::fromUtf8(":/images/app_image")));
        label_appimage->setScaledContents(true);

        horizontalLayout->addWidget(label_appimage);


        verticalLayout_2->addLayout(horizontalLayout);

        label_copyright = new QLabel(AboutDialog);
        label_copyright->setObjectName(QStringLiteral("label_copyright"));
        label_copyright->setCursor(QCursor(Qt::IBeamCursor));
        label_copyright->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_2->addWidget(label_copyright);

        label_notice = new QLabel(AboutDialog);
        label_notice->setObjectName(QStringLiteral("label_notice"));
        label_notice->setCursor(QCursor(Qt::IBeamCursor));
        label_notice->setWordWrap(true);
        label_notice->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_2->addWidget(label_notice);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        button_ok = new QDialogButtonBox(AboutDialog);
        button_ok->setObjectName(QStringLiteral("button_ok"));
        button_ok->setOrientation(Qt::Horizontal);
        button_ok->setStandardButtons(QDialogButtonBox::Ok);
        button_ok->setCenterButtons(true);

        verticalLayout_2->addWidget(button_ok);


        horizontalLayout_2->addLayout(verticalLayout_2);


        retranslateUi(AboutDialog);
        QObject::connect(button_ok, SIGNAL(accepted()), AboutDialog, SLOT(accept()));
        QObject::connect(button_ok, SIGNAL(rejected()), AboutDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Social Bot Interface", 0));
        label_appname->setText(QApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-weight:600;\">Social Bot Interface</span> version</p></body></html>", 0));
        label_appimage->setText(QString());
        label_copyright->setText(QApplication::translate("AboutDialog", "Copyright \302\251 2013-2014 Trezanik", 0));
        label_notice->setText(QApplication::translate("AboutDialog", "\n"
"This is experimental software.\n"
"\n"
"Distributed under the Zlib software license, see the accompanying LICENCE file or: http://www.opensource.org/licenses/zlib.", 0));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
