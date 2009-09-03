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
** Form generated from reading UI file 'history.ui'
**
** Created: Tue Aug 18 19:03:31 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef HISTORY_H
#define HISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include "edittreeview.h"
#include "searchlineedit.h"

QT_BEGIN_NAMESPACE

class Ui_HistoryDialog
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *spacerItem;
    SearchLineEdit *search;
    EditTreeView *tree;
    QHBoxLayout *hboxLayout;
    QPushButton *removeButton;
    QPushButton *removeAllButton;
    QSpacerItem *spacerItem1;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *HistoryDialog)
    {
        if (HistoryDialog->objectName().isEmpty())
            HistoryDialog->setObjectName(QString::fromUtf8("HistoryDialog"));
        HistoryDialog->resize(758, 450);
        gridLayout = new QGridLayout(HistoryDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        spacerItem = new QSpacerItem(252, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 0, 0, 1, 1);

        search = new SearchLineEdit(HistoryDialog);
        search->setObjectName(QString::fromUtf8("search"));

        gridLayout->addWidget(search, 0, 1, 1, 1);

        tree = new EditTreeView(HistoryDialog);
        tree->setObjectName(QString::fromUtf8("tree"));

        gridLayout->addWidget(tree, 1, 0, 1, 2);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        removeButton = new QPushButton(HistoryDialog);
        removeButton->setObjectName(QString::fromUtf8("removeButton"));

        hboxLayout->addWidget(removeButton);

        removeAllButton = new QPushButton(HistoryDialog);
        removeAllButton->setObjectName(QString::fromUtf8("removeAllButton"));

        hboxLayout->addWidget(removeAllButton);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem1);

        buttonBox = new QDialogButtonBox(HistoryDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);

        hboxLayout->addWidget(buttonBox);


        gridLayout->addLayout(hboxLayout, 2, 0, 1, 2);


        retranslateUi(HistoryDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), HistoryDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(HistoryDialog);
    } // setupUi

    void retranslateUi(QDialog *HistoryDialog)
    {
        HistoryDialog->setWindowTitle(QApplication::translate("HistoryDialog", "History", 0, QApplication::UnicodeUTF8));
        removeButton->setText(QApplication::translate("HistoryDialog", "&Remove", 0, QApplication::UnicodeUTF8));
        removeAllButton->setText(QApplication::translate("HistoryDialog", "Remove &All", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(HistoryDialog);
    } // retranslateUi

};

namespace Ui {
    class HistoryDialog: public Ui_HistoryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // HISTORY_H
