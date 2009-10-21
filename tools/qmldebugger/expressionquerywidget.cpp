#include <QtGui/qlabel.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qevent.h>
#include <QtGui/qgroupbox.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qlayout.h>

#include "expressionquerywidget.h"

ExpressionQueryWidget::ExpressionQueryWidget(QmlEngineDebug *client, QWidget *parent)
    : QWidget(parent),
      m_style(Compact),
      m_client(client),
      m_query(0),
      m_groupBox(0),
      m_textEdit(new QTextEdit),
      m_lineEdit(0),
      m_button(0)
{
    m_prompt = QLatin1String(">> ");

    m_groupBox = new QGroupBox;
    QVBoxLayout *vbox = new QVBoxLayout(m_groupBox);
    vbox->addWidget(m_textEdit);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_groupBox);

    updateTitle();

    if (m_style == Compact) {
        QHBoxLayout *hbox = new QHBoxLayout;
        m_button = new QPushButton(tr("Execute"));
        m_button->setEnabled(false);
        connect(m_button, SIGNAL(clicked()), SLOT(executeExpression()));
        m_lineEdit = new QLineEdit;
        connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(executeExpression()));
        connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(lineEditTextChanged(QString)));
        hbox->addWidget(new QLabel(tr("Expression:")));
        hbox->addWidget(m_lineEdit);
        hbox->addWidget(m_button);
        vbox->addLayout(hbox);

        m_textEdit->setReadOnly(true);
        m_lineEdit->installEventFilter(this);
    } else {
        m_textEdit->installEventFilter(this);
    }
}

void ExpressionQueryWidget::updateTitle()
{
    if (m_currObject.debugId() < 0) {
        m_groupBox->setTitle(tr("Expression queries"));
    } else {
        QString desc = QLatin1String("<")
            + m_currObject.className() + QLatin1String(": ")
            + (m_currObject.name().isEmpty() ? QLatin1String("<unnamed>") : m_currObject.name())
            + QLatin1String(">");
        m_groupBox->setTitle(tr("Expression queries (using context for %1)"
                , "Selected object").arg(desc));
    }
}

void ExpressionQueryWidget::appendPrompt()
{
    m_textEdit->moveCursor(QTextCursor::End);

    if (m_style == Compact) {
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
    m_textEdit->moveCursor(QTextCursor::End);
    m_textEdit->setTextColor(Qt::darkGreen);
    m_textEdit->append(m_currObject.className()
            + QLatin1String(": ")
            + (m_currObject.name().isEmpty() ? QLatin1String("<unnamed>") : m_currObject.name()));
    appendPrompt();
}

void ExpressionQueryWidget::executeExpression()
{
    if (m_style == Compact)
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
            QObject::connect(m_query, SIGNAL(stateChanged(State)),
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
        QString result;
        if (m_query->result().isNull())
            result = QLatin1String("<no value>");
        else
            result = m_query->result().toString();

        if (m_style == Compact) {
            m_textEdit->setTextColor(Qt::black);
            m_textEdit->setFontWeight(QFont::Bold);
            m_textEdit->insertPlainText(m_expr + " : ");
            m_textEdit->setFontWeight(QFont::Normal);
            m_textEdit->insertPlainText(result);
        } else {
            m_textEdit->append(result);
        }
        appendPrompt();
        m_expr.clear();
    }
}

void ExpressionQueryWidget::lineEditTextChanged(const QString &s)
{
    if (m_button)
        m_button->setEnabled(!s.isEmpty());
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
