#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEditExtension : public QObject
{
Q_OBJECT
Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin NOTIFY marginsChanged);
Q_PROPERTY(int rightMargin READ rightMargin WRITE setRightMargin NOTIFY marginsChanged);
Q_PROPERTY(int topMargin READ topMargin WRITE setTopMargin NOTIFY marginsChanged);
Q_PROPERTY(int bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY marginsChanged);
public:
    LineEditExtension(QObject *);

    int leftMargin() const;
    void setLeftMargin(int);

    int rightMargin() const;
    void setRightMargin(int);

    int topMargin() const;
    void setTopMargin(int);

    int bottomMargin() const;
    void setBottomMargin(int);
signals:
    void marginsChanged();

private:
    QLineEdit *m_lineedit;
};

#endif // LINEEDIT_H
