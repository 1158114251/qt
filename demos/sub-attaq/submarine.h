/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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
** contact the sales department at http://qt.nokia.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef __SUBMARINE__H__
#define __SUBMARINE__H__

//Qt
#include <QtCore/QVariantAnimation>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsTransform>

class PixmapItem;

class Torpedo;

class SubMarine : public QGraphicsWidget
{
Q_OBJECT
public:
    enum Movement {
       None = 0,
       Left,
       Right
    };
    enum { Type = UserType + 1 };
    SubMarine(int type, const QString &name, int points, QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);

    int points();

    void setCurrentDirection(Movement direction);
    enum Movement currentDirection() const;

    void setCurrentSpeed(int speed);
    int currentSpeed() const;

    void launchTorpedo(int speed);
    void destroy();

    virtual int type() const;

    QGraphicsRotation *rotation() const { return graphicsRotation; }

signals:
    void subMarineDestroyed();
    void subMarineExecutionFinished();
    void subMarineStateChanged();

private:
    int subType;
    QString subName;
    int subPoints;
    int speed;
    Movement direction;
    PixmapItem *pixmapItem;
    QGraphicsRotation *graphicsRotation;
};

#endif //__SUBMARINE__H__
