/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt QML Debugger of the Qt Toolkit.
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
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
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
#include <QtCore/qdebug.h>

#include <QtGui/qlabel.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qevent.h>
#include <QtGui/qgroupbox.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qlayout.h>

#include "expressionquerywidget.h"

QT_BEGIN_NAMESPACE

ExpressionQueryWidget::ExpressionQueryWidget(Mode mode, QmlEngineDebug *client, QWidget *parent)
    : QWidget(parent),
      m_mode(mode),
      m_client(client),
      m_query(0),
      m_textEdit(new QTextEdit),
      m_lineEdit(0)
{
    m_prompt = QLatin1String(">> ");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_textEdit);

    updateTitle();

    if (m_mode == SeparateEntryMode) {
        m_lineEdit = new QLineEdit;
        connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(executeExpression()));
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->setMargin(5);
        hbox->setSpacing(5);
        hbox->addWidget(new QLabel(tr("Expression:")));
        hbox->addWidget(m_lineEdit);
        layout->addLayout(hbox);

        m_textEdit->setReadOnly(true);
        m_lineEdit->installEventFilter(this);
    } else {
        m_textEdit->installEventFilter(this);
        appendPrompt();
    }
}

void ExpressionQueryWidget::setEngineDebug(QmlEngineDebug *client)
{
    m_client = client;
}

void ExpressionQueryWidget::clear()
{
    m_textEdit->clear();
    if (m_lineEdit)
        m_lineEdit->clear();
    if (m_mode == ShellMode)
        appendPrompt();
}

void ExpressionQueryWidget::updateTitle()
{
    if (m_currObject.debugId() < 0) {
        m_title = tr("Expression queries");
    } else {
        QString desc = QLatin1String("<")
            + m_currObject.className() + QLatin1String(": ")
            + (m_currObject.name().isEmpty() ? QLatin1String("<unnamed>") : m_currObject.name())
            + QLatin1String(">");
        m_title = tr("Expression queries (using context for %1)" , "Selected object").arg(desc);
    }
}

void ExpressionQueryWidget::appendPrompt()
{
    m_textEdit->moveCursor(QTextCursor::End);

    if (m_mode == SeparateEntryMode) {
        m_textEdit->insertPlainText("\n");
    } else {
        m_textEdit->setTextColor(Qt::gray);
        m_textEdit->append(m_prompt);
    }
}

void ExpressionQueryWidget::setCurrentObject(const QmlDebugObjectReference &obj)
{
    m_currObject = obj;
    updateTitle();
}

void ExpressionQueryWidget::checkCurrentContext()
{
    m_textEdit->moveCursor(QTextCursor::End);

    if (m_currObject.debugId() != -1 && m_currObject.debugId() != m_objectAtLastFocus.debugId())
        showCurrentContext();
    m_objectAtLastFocus = m_currObject;
}

void ExpressionQueryWidget::showCurrentContext()
{
    if (m_mode == ShellMode) {
        // clear the initial prompt
        if (m_textEdit->document()->lineCount() == 1)
            m_textEdit->clear();
    }

    m_textEdit->moveCursor(QTextCursor::End);
    m_textEdit->setTextColor(Qt::darkGreen);
    m_textEdit->append(m_currObject.className()
            + QLatin1String(": ")
            + (m_currObject.name().isEmpty() ? QLatin1String("<unnamed object>") : m_currObject.name()));
    appendPrompt();
}

void ExpressionQueryWidget::executeExpression()
{
    if (!m_client)
        return;
        
    if (m_mode == SeparateEntryMode)
        m_expr = m_lineEdit->text().trimmed();
    else
        m_expr = m_expr.trimmed();

    if (!m_expr.isEmpty() && m_currObject.debugId() != -1) {
        if (m_query)
            delete m_query;
        m_query = m_client->queryExpressionResult(m_currObject.debugId(), m_expr, this);
        if (!m_query->isWaiting())
            showResult();
        else
            QObject::connect(m_query, SIGNAL(stateChanged(QmlDebugQuery::State)),
                            this, SLOT(showResult()));

        m_lastExpr = m_expr;
        if (m_lineEdit)
            m_lineEdit->clear();
    }
}

void ExpressionQueryWidget::showResult()
{
    if (m_query) {
        m_textEdit->moveCursor(QTextCursor::End);
        QVariant value = m_query->result();
        QString result;
        
        if (value.type() == QVariant::List || value.type() == QVariant::StringList) {
            result = tr("<%1 items>", "%1 = number of items").arg(value.toList().count());
        } else if (value.isNull()) {
            result = QLatin1String("<no value>");
        } else {
            result = value.toString();
        }

        if (m_mode == SeparateEntryMode) {
            m_textEdit->setTextColor(Qt::black);
            m_textEdit->setFontWeight(QFont::Bold);
            m_textEdit->insertPlainText(m_expr + " : ");
            m_textEdit->setFontWeight(QFont::Normal);
            m_textEdit->insertPlainText(result);
        } else {
            m_textEdit->setTextColor(Qt::darkGreen);
            m_textEdit->insertPlainText(" => ");
            m_textEdit->setTextColor(Qt::black);
            m_textEdit->insertPlainText(result);
        }
        appendPrompt();
        m_expr.clear();
    }
}

bool ExpressionQueryWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit) {
        switch (event->type()) {
            case QEvent::KeyPress:
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                int key = keyEvent->key();
                if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                    executeExpression();
                    return true;
                } else if (key == Qt::Key_Backspace) {
                    // ensure m_expr doesn't contain backspace characters
                    QTextCursor cursor = m_textEdit->textCursor();
                    bool atLastLine = !(cursor.block().next().isValid());
                    if (!atLastLine)
                        return true;
                    if (cursor.columnNumber() <= m_prompt.count())
                        return true;
                    cursor.deletePreviousChar();
                    m_expr = cursor.block().text().mid(m_prompt.count());
                    return true;
                } else {
                    m_textEdit->moveCursor(QTextCursor::End);
                    m_textEdit->setTextColor(Qt::black);
                    m_expr += keyEvent->text();
                }
                break;
            }
            case QEvent::FocusIn:
                checkCurrentContext();
                m_textEdit->moveCursor(QTextCursor::End);
                break;
            default:
                break;
        }
    } else if (obj == m_lineEdit) {
        switch (event->type()) {
            case QEvent::KeyPress:
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                int key = keyEvent->key();
                if (key == Qt::Key_Up && m_lineEdit->text() != m_lastExpr) {
                    m_expr = m_lineEdit->text();
                    if (!m_lastExpr.isEmpty())
                        m_lineEdit->setText(m_lastExpr);
                } else if (key == Qt::Key_Down) {
                    m_lineEdit->setText(m_expr);
                }
                break;
            }
            case QEvent::FocusIn:
                checkCurrentContext();
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

QT_END_NAMESPACE
