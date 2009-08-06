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

#ifndef QFXITEM_H
#define QFXITEM_H

#include <QtCore/QObject>
#include <QtScript/qscriptvalue.h>
#include <QtCore/QList>
#include <QtDeclarative/qfxglobal.h>
#include <QtDeclarative/qml.h>
#include <QtDeclarative/qmlcomponent.h>
#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicstransform.h>
#include <QtGui/qfont.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QGraphicsTransform;

class QFxItem;
class Q_DECLARATIVE_EXPORT QFxContents : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal height READ height NOTIFY heightChanged)
    Q_PROPERTY(qreal width READ width NOTIFY widthChanged)
public:
    QFxContents();

    qreal height() const;

    qreal width() const;

    void setItem(QFxItem *item);

public Q_SLOTS:
    void calcHeight();
    void calcWidth();

Q_SIGNALS:
    void heightChanged();
    void widthChanged();

private:
    QFxItem *m_item;
    qreal m_height;
    qreal m_width;
};

class QmlState;
class QFxAnchorLine;
class QmlTransition;
class QFxKeyEvent;
class QFxAnchors;
class QFxItemPrivate;
class Q_DECLARATIVE_EXPORT QFxItem : public QGraphicsObject, public QmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QmlParserStatus)

    Q_PROPERTY(QFxItem * parent READ parentItem WRITE setParentItem NOTIFY parentChanged DESIGNABLE false FINAL)
    Q_PROPERTY(QmlList<QObject *> *data READ data DESIGNABLE false)
    Q_PROPERTY(QmlList<QFxItem *>* children READ children DESIGNABLE false)
    Q_PROPERTY(QmlList<QObject *>* resources READ resources DESIGNABLE false)
    Q_PROPERTY(QmlList<QmlState *>* states READ states DESIGNABLE false)
    Q_PROPERTY(QmlList<QmlTransition *>* transitions READ transitions DESIGNABLE false)
    Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged FINAL)
    Q_PROPERTY(QFxContents * contents READ contents DESIGNABLE false CONSTANT FINAL)
    Q_PROPERTY(QFxAnchors * anchors READ anchors DESIGNABLE false CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine left READ left CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine right READ right CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine horizontalCenter READ horizontalCenter CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine top READ top CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine bottom READ bottom CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine verticalCenter READ verticalCenter CONSTANT FINAL)
    Q_PROPERTY(QFxAnchorLine baseline READ baseline CONSTANT FINAL)
    Q_PROPERTY(qreal baselineOffset READ baselineOffset WRITE setBaselineOffset NOTIFY baselineOffsetChanged)
    Q_PROPERTY(bool clip READ clip WRITE setClip) // ### move to QGI/QGO, NOTIFY
    Q_PROPERTY(bool focus READ hasFocus WRITE setFocus NOTIFY focusChanged FINAL)
    Q_PROPERTY(bool activeFocus READ hasActiveFocus NOTIFY activeFocusChanged FINAL)
    Q_PROPERTY(QmlList<QGraphicsTransform *>* transform READ transform DESIGNABLE false FINAL)
    Q_PROPERTY(TransformOrigin transformOrigin READ transformOrigin WRITE setTransformOrigin)
    Q_PROPERTY(bool smooth READ smoothTransform WRITE setSmoothTransform)
    Q_ENUMS(TransformOrigin)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    enum TransformOrigin {
        TopLeft, Top, TopRight,
        Left, Center, Right,
        BottomLeft, Bottom, BottomRight
    };

    QFxItem(QFxItem *parent = 0);
    virtual ~QFxItem();

    QFxItem *parentItem() const;
    void setParentItem(QFxItem *parent);
    void setParent(QFxItem *parent) { setParentItem(parent); }

    QmlList<QObject *> *data();
    QmlList<QFxItem *> *children();
    QmlList<QObject *> *resources();

    QFxAnchors *anchors();
    QFxContents *contents();

    bool clip() const;
    void setClip(bool);

    QmlList<QmlState *>* states();
    QmlList<QmlTransition *>* transitions();

    QString state() const;
    void setState(const QString &);

    qreal baselineOffset() const;
    void setBaselineOffset(qreal);

    QmlList<QGraphicsTransform *> *transform();

    qreal width() const;
    void setWidth(qreal);

    qreal height() const;
    void setHeight(qreal);

    TransformOrigin transformOrigin() const;
    void setTransformOrigin(TransformOrigin);

    bool smoothTransform() const;
    void setSmoothTransform(bool);

    QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    virtual bool hasFocus() const;
    void setFocus(bool);
    bool hasActiveFocus() const;

    bool keepMouseGrab() const;
    void setKeepMouseGrab(bool);

Q_SIGNALS:
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void baselineOffsetChanged();
    void stateChanged(const QString &);
    void focusChanged();
    void activeFocusChanged();
    void parentChanged();
    void keyPress(QFxKeyEvent *event);
    void keyRelease(QFxKeyEvent *event);

protected:
    bool isComponentComplete() const;
    virtual bool sceneEvent(QEvent *);
    virtual bool event(QEvent *);
    virtual QVariant itemChange(GraphicsItemChange, const QVariant &);

    void setImplicitWidth(qreal);
    bool widthValid() const; // ### better name?
    void setImplicitHeight(qreal);
    bool heightValid() const; // ### better name?

    virtual void classBegin();
    virtual void componentComplete();
    virtual void focusChanged(bool);
    virtual void activeFocusChanged(bool);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void geometryChanged(const QRectF &newGeometry,
                                 const QRectF &oldGeometry);

private Q_SLOTS:
    void doUpdate();

protected:
    QFxItem(QFxItemPrivate &dd, QFxItem *parent = 0);

private:
    // ### public?
    QFxAnchorLine left() const;
    QFxAnchorLine right() const;
    QFxAnchorLine horizontalCenter() const;
    QFxAnchorLine top() const;
    QFxAnchorLine bottom() const;
    QFxAnchorLine verticalCenter() const;
    QFxAnchorLine baseline() const;

    // ### move to d-pointer
    void init(QFxItem *parent);
    friend class QmlStatePrivate;
    friend class QFxAnchors;
    Q_DISABLE_COPY(QFxItem)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr, QFxItem)
};

// ### move to QGO
template<typename T>
T qobject_cast(QGraphicsItem *item)
{
    if (!item) return 0;
    QObject *o = item->toGraphicsObject();
    return qobject_cast<T>(o);
}

QDebug Q_DECLARATIVE_EXPORT operator<<(QDebug debug, QFxItem *item);

QT_END_NAMESPACE

QML_DECLARE_TYPE(QFxContents)
QML_DECLARE_TYPE(QFxItem)
QML_DECLARE_TYPE(QGraphicsTransform)
QML_DECLARE_TYPE(QGraphicsScale)
QML_DECLARE_TYPE(QGraphicsRotation)
QML_DECLARE_TYPE(QGraphicsRotation3D)

QT_END_HEADER

#endif // QFXITEM_H
