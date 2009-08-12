/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
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
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "graphicslayouts.h"
#include <QtGui/qgraphicswidget.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QML_DEFINE_INTERFACE(QGraphicsLayoutItem)
QML_DEFINE_INTERFACE(QGraphicsLayout)

class LinearLayoutAttached : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int stretchFactor READ stretchFactor WRITE setStretchFactor NOTIFY stretchChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
public:
    LinearLayoutAttached(QObject *parent)
    : QObject(parent), _stretch(1), _alignment(Qt::AlignCenter)
    {
    }

    int stretchFactor() const { return _stretch; }
    void setStretchFactor(int f)
    {
        if (_stretch == f)
            return;

        _stretch = f;
        emit stretchChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _stretch);
    }

    Qt::Alignment alignment() const { return _alignment; }
    void setAlignment(Qt::Alignment a)
    {
        if (_alignment == a)
            return;

        _alignment = a;
        emit alignmentChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _alignment);
    }

Q_SIGNALS:
    void stretchChanged(QGraphicsLayoutItem*,int);
    void alignmentChanged(QGraphicsLayoutItem*,Qt::Alignment);

private:
    int _stretch;
    Qt::Alignment _alignment;
};

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,QGraphicsLinearLayoutStretchItem,QGraphicsLinearLayoutStretchItemObject)

QGraphicsLinearLayoutStretchItemObject::QGraphicsLinearLayoutStretchItemObject(QObject *parent)
        : QObject(parent)
{
}

QSizeF QGraphicsLinearLayoutStretchItemObject::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return QSizeF();
}

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,QGraphicsLinearLayout,QGraphicsLinearLayoutObject)

QGraphicsLinearLayoutObject::QGraphicsLinearLayoutObject(QObject *parent)
: QObject(parent), _children(this)
{
}

QGraphicsLinearLayoutObject::~QGraphicsLinearLayoutObject()
{
}

void QGraphicsLinearLayoutObject::insertLayoutItem(int index, QGraphicsLayoutItem *item)
{
    insertItem(index, item);

    //connect attached properties
    if (QObject *obj = attachedProperties.value(item)) {
        setStretchFactor(item, static_cast<LinearLayoutAttached *>(obj)->stretchFactor());
        setAlignment(item, static_cast<LinearLayoutAttached *>(obj)->alignment());
        QObject::connect(obj, SIGNAL(stretchChanged(QGraphicsLayoutItem*,int)),
                         this, SLOT(updateStretch(QGraphicsLayoutItem*,int)));
        QObject::connect(obj, SIGNAL(alignmentChanged(QGraphicsLayoutItem*,Qt::Alignment)),
                         this, SLOT(updateAlignment(QGraphicsLayoutItem*,Qt::Alignment)));
        //XXX need to disconnect when widget is removed?
    }
}

//XXX is there a better way to do this?
void QGraphicsLinearLayoutObject::clearChildren()
{
    for (int i = 0; i < count(); ++i)
        removeAt(i);
}

void QGraphicsLinearLayoutObject::updateStretch(QGraphicsLayoutItem *item, int stretch)
{
    QGraphicsLinearLayout::setStretchFactor(item, stretch);
}

void QGraphicsLinearLayoutObject::updateAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment)
{
    QGraphicsLinearLayout::setAlignment(item, alignment);
}

QHash<QGraphicsLayoutItem*, QObject*> QGraphicsLinearLayoutObject::attachedProperties;
LinearLayoutAttached *QGraphicsLinearLayoutObject::qmlAttachedProperties(QObject *obj)
{
    // ### This is not allowed - you must attach to any object
    if (!qobject_cast<QGraphicsLayoutItem*>(obj))
        return 0;
    LinearLayoutAttached *rv = new LinearLayoutAttached(obj);
    /*if (QGraphicsLinearLayoutObject *lo = qobject_cast<QGraphicsLinearLayoutObject*>(obj->parent()))
        QObject::connect(rv, SIGNAL(stretchChanged(QGraphicsLayoutItem*,int)),
                         lo, SLOT(updateStretch(QGraphicsLayoutItem*,int)));
        QObject::connect(rv, SIGNAL(alignmentChanged(QGraphicsLayoutItem*,Qt::Alignment)),
                         lo, SLOT(updateAlignment(QGraphicsLayoutItem*,Qt::Alignment)));*/
    attachedProperties.insert(qobject_cast<QGraphicsLayoutItem*>(obj), rv);
    return rv;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// QGraphicsGridLayout-related classes
//////////////////////////////////////////////////////////////////////////////////////////////////////
class GridLayoutAttached : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int row READ row WRITE setRow)
    Q_PROPERTY(int column READ column WRITE setColumn)
    Q_PROPERTY(int rowSpan READ rowSpan WRITE setRowSpan)
    Q_PROPERTY(int columnSpan READ columnSpan WRITE setColumnSpan)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
public:
    GridLayoutAttached(QObject *parent)
    : QObject(parent), _row(-1), _column(-1), _rowspan(1), _colspan(1), _alignment(-1)
    {
    }

    int row() const { return _row; }
    void setRow(int r)
    {
        if (_row == r)
            return;

        _row = r;
        //emit rowChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _row);
    }

    int column() const { return _column; }
    void setColumn(int c)
    {
        if (_column == c)
            return;

        _column = c;
        //emit columnChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _column);
    }

    int rowSpan() const { return _rowspan; }
    void setRowSpan(int rs)
    {
        if (_rowspan == rs)
            return;

        _rowspan = rs;
        //emit rowSpanChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _rowSpan);
    }

    int columnSpan() const { return _colspan; }
    void setColumnSpan(int cs)
    {
        if (_colspan == cs)
            return;

        _colspan = cs;
        //emit columnSpanChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _columnSpan);
    }

    Qt::Alignment alignment() const { return _alignment; }
    void setAlignment(Qt::Alignment a)
    {
        if (_alignment == a)
            return;

        _alignment = a;
        //emit alignmentChanged(reinterpret_cast<QGraphicsLayoutItem*>(parent()), _alignment);
    }

Q_SIGNALS:
    //void rowChanged(QGraphicsLayoutItem*,int);
    //void columnSpanChanged(QGraphicsLayoutItem*,int);
    //void rowSpanChanged(QGraphicsLayoutItem*,int);
    //void columnChanged(QGraphicsLayoutItem*,int);
    //void alignmentChanged(QGraphicsLayoutItem*,Qt::Alignment);

private:
    int _row;
    int _column;
    int _rowspan;
    int _colspan;
    Qt::Alignment _alignment;
};

QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,QGraphicsGridLayout,QGraphicsGridLayoutObject)


QGraphicsGridLayoutObject::QGraphicsGridLayoutObject(QObject *parent)
: QObject(parent), _children(this)
{
}

QGraphicsGridLayoutObject::~QGraphicsGridLayoutObject()
{
}

void QGraphicsGridLayoutObject::addWidget(QGraphicsWidget *wid)
{
    //use attached properties
    if (QObject *obj = attachedProperties.value(qobject_cast<QGraphicsLayoutItem*>(wid))) {
        int row = static_cast<GridLayoutAttached *>(obj)->row();
        int column = static_cast<GridLayoutAttached *>(obj)->column();
        int rowSpan = static_cast<GridLayoutAttached *>(obj)->rowSpan();
        int columnSpan = static_cast<GridLayoutAttached *>(obj)->columnSpan();
        if (row == -1 || column == -1) {
            qWarning() << "Must set row and column for an item in a grid layout";
            return;
        }
        addItem(wid, row, column, rowSpan, columnSpan);
    }
}

void QGraphicsGridLayoutObject::addLayoutItem(QGraphicsLayoutItem *item)
{
    //use attached properties
    if (QObject *obj = attachedProperties.value(item)) {
        int row = static_cast<GridLayoutAttached *>(obj)->row();
        int column = static_cast<GridLayoutAttached *>(obj)->column();
        int rowSpan = static_cast<GridLayoutAttached *>(obj)->rowSpan();
        int columnSpan = static_cast<GridLayoutAttached *>(obj)->columnSpan();
        Qt::Alignment alignment = static_cast<GridLayoutAttached *>(obj)->alignment();
        if (row == -1 || column == -1) {
            qWarning() << "Must set row and column for an item in a grid layout";
            return;
        }
        addItem(item, row, column, rowSpan, columnSpan);
        if (alignment != -1)
            setAlignment(item,alignment);
    }
}

//XXX is there a better way to do this?
void QGraphicsGridLayoutObject::clearChildren()
{
    for (int i = 0; i < count(); ++i)
        removeAt(i);
}

qreal QGraphicsGridLayoutObject::spacing() const
{
    if (verticalSpacing() == horizontalSpacing())
        return verticalSpacing();
    return -1;  //XXX
}

QHash<QGraphicsLayoutItem*, QObject*> QGraphicsGridLayoutObject::attachedProperties;
GridLayoutAttached *QGraphicsGridLayoutObject::qmlAttachedProperties(QObject *obj)
{
    // ### This is not allowed - you must attach to any object
    if (!qobject_cast<QGraphicsLayoutItem*>(obj))
        return 0;
    GridLayoutAttached *rv = new GridLayoutAttached(obj);
    attachedProperties.insert(qobject_cast<QGraphicsLayoutItem*>(obj), rv);
    return rv;
}

QT_END_NAMESPACE

#include "graphicslayouts.moc"
