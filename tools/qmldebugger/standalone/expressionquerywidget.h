#ifndef EXPRESSIONQUERYWIDGET_H
#define EXPRESSIONQUERYWIDGET_H

#include <QWidget>

#include <QtDeclarative/qmldebug.h>

QT_BEGIN_NAMESPACE

class QGroupBox;
class QTextEdit;
class QLineEdit;
class QPushButton;

class ExpressionQueryWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        SeparateEntryMode,
        ShellMode
    };

    ExpressionQueryWidget(Mode mode = SeparateEntryMode, QmlEngineDebug *client = 0, QWidget *parent = 0);
    
    void setEngineDebug(QmlEngineDebug *client);
    void clear();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void setCurrentObject(const QmlDebugObjectReference &obj);

private slots:
    void executeExpression();
    void showResult();

private:
    void appendPrompt();
    void checkCurrentContext();
    void showCurrentContext();
    void updateTitle();

    Mode m_mode;

    QmlEngineDebug *m_client;
    QmlDebugExpressionQuery *m_query;
    QTextEdit *m_textEdit;
    QLineEdit *m_lineEdit;
    QPushButton *m_button;
    QString m_prompt;
    QString m_expr;
    QString m_lastExpr;

    QString m_title;

    QmlDebugObjectReference m_currObject;
    QmlDebugObjectReference m_objectAtLastFocus;
};

QT_END_NAMESPACE

#endif

