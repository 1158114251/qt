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

#ifndef QFXMOUSEREGION_H
#define QFXMOUSEREGION_H

#include <qfxitem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class Q_DECLARATIVE_EXPORT QFxDrag : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QFxItem *target READ target WRITE setTarget)
    Q_PROPERTY(QString axis READ axis WRITE setAxis)
    Q_PROPERTY(int xmin READ xmin WRITE setXmin)
    Q_PROPERTY(int xmax READ xmax WRITE setXmax)
    Q_PROPERTY(int ymin READ ymin WRITE setYmin)
    Q_PROPERTY(int ymax READ ymax WRITE setYmax)
public:
    QFxDrag(QObject *parent=0);
    ~QFxDrag();

    QFxItem *target() const;
    void setTarget(QFxItem *);
    QString axis() const;
    void setAxis(const QString &);
    int xmin() const;
    void setXmin(int);
    int xmax() const;
    void setXmax(int);
    int ymin() const;
    void setYmin(int);
    int ymax() const;
    void setYmax(int);

private:
    QFxItem *_target;
    QString _axis;
    int _xmin;
    int _xmax;
    int _ymin;
    int _ymax;
    Q_DISABLE_COPY(QFxDrag)
};
QML_DECLARE_TYPE(QFxDrag);

class QFxMouseEvent;
class QFxMouseRegionPrivate;
class Q_DECLARATIVE_EXPORT QFxMouseRegion : public QFxItem
{
    Q_OBJECT

    Q_PROPERTY(int mouseX READ mouseX NOTIFY positionChanged)
    Q_PROPERTY(int mouseY READ mouseY NOTIFY positionChanged)
    Q_PROPERTY(bool containsMouse READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool pressed READ pressed NOTIFY pressedChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QFxDrag *drag READ drag)
public:
    QFxMouseRegion(QFxItem *parent=0);
    ~QFxMouseRegion();

    int mouseX() const;
    int mouseY() const;

    bool isEnabled() const;
    void setEnabled(bool);

    bool hovered();
    bool pressed();

    void setHovered(bool);
    void setPressed(bool);

    QFxDrag *drag();

Q_SIGNALS:
    void hoveredChanged();
    void pressedChanged();
    void positionChanged(QFxMouseEvent *mouse);

    void pressed(QFxMouseEvent *mouse);
    void pressAndHold(QFxMouseEvent *mouse);
    void released(QFxMouseEvent *mouse);
    void clicked(QFxMouseEvent *mouse);
    void doubleClicked(QFxMouseEvent *mouse);
    void entered();
    void exited();
    void exitedWhilePressed();
    void reenteredWhilePressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mouseUngrabEvent();
    void timerEvent(QTimerEvent *event);

private:
    void handlePress();
    void handleRelease();

protected:
    QFxMouseRegion(QFxMouseRegionPrivate &dd, QFxItem *parent);

private:
    Q_DISABLE_COPY(QFxMouseRegion)
    Q_DECLARE_PRIVATE(QFxMouseRegion)
};
QML_DECLARE_TYPE(QFxMouseRegion);

QT_END_NAMESPACE

QT_END_HEADER

#endif // QFXMOUSEREGION_H
