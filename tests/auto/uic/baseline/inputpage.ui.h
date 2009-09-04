/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

/********************************************************************************
** Form generated from reading UI file 'inputpage.ui'
**
** Created: Tue Aug 18 19:03:31 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef INPUTPAGE_H
#define INPUTPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InputPage
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *spacerItem;
    QLabel *label;
    QHBoxLayout *hboxLayout;
    QLineEdit *fileLineEdit;
    QToolButton *browseButton;
    QSpacerItem *spacerItem1;

    void setupUi(QWidget *InputPage)
    {
        if (InputPage->objectName().isEmpty())
            InputPage->setObjectName(QString::fromUtf8("InputPage"));
        InputPage->resize(417, 242);
        gridLayout = new QGridLayout(InputPage);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        spacerItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout->addItem(spacerItem, 0, 2, 1, 1);

        label = new QLabel(InputPage);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        fileLineEdit = new QLineEdit(InputPage);
        fileLineEdit->setObjectName(QString::fromUtf8("fileLineEdit"));

        hboxLayout->addWidget(fileLineEdit);

        browseButton = new QToolButton(InputPage);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));

        hboxLayout->addWidget(browseButton);


        gridLayout->addLayout(hboxLayout, 1, 1, 1, 2);

        spacerItem1 = new QSpacerItem(20, 31, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem1, 2, 1, 1, 1);


        retranslateUi(InputPage);

        QMetaObject::connectSlotsByName(InputPage);
    } // setupUi

    void retranslateUi(QWidget *InputPage)
    {
        InputPage->setWindowTitle(QApplication::translate("InputPage", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("InputPage", "File name:", 0, QApplication::UnicodeUTF8));
        browseButton->setText(QApplication::translate("InputPage", "...", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(InputPage);
    } // retranslateUi

};

namespace Ui {
    class InputPage: public Ui_InputPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // INPUTPAGE_H
