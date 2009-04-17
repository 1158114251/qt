/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QITEMANIMATION_H
#define QITEMANIMATION_H

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qvariantanimation.h"
#else
# include <QtCore/qvariantanimation.h>
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_ANIMATION

class QGraphicsItem;

class QItemAnimationPrivate;
class Q_GUI_EXPORT QItemAnimation : public QVariantAnimation
{
public:
    enum PropertyName
    {
        None, //default
        Position,
        Opacity,
        RotationX,
        RotationY,
        RotationZ,
        ScaleFactorX,
        ScaleFactorY
    };

    Q_OBJECT
    Q_PROPERTY(PropertyName propertyName READ propertyName WRITE setPropertyName)
    Q_PROPERTY(QGraphicsItem* targetItem READ targetItem WRITE setTargetItem)  /*NOTIFY targetItemChanged*/

public:
    QItemAnimation(QObject *parent = 0);
    QItemAnimation(QGraphicsItem *target, PropertyName p = None, QObject *parent = 0);
    ~QItemAnimation();

    QGraphicsItem *targetItem() const;
    void setTargetItem(QGraphicsItem *item);

    PropertyName propertyName() const;
    void setPropertyName(PropertyName);

    static QList<QItemAnimation*> runningAnimations(QGraphicsItem *item = 0);
    static QItemAnimation* runningAnimation(QGraphicsItem *item, PropertyName prop);

protected:
    bool event(QEvent *event);
    void updateCurrentValue(const QVariant &value);
    void updateState(QAbstractAnimation::State oldState, QAbstractAnimation::State newState);

private:
    Q_DISABLE_COPY(QItemAnimation)
    Q_DECLARE_PRIVATE(QItemAnimation)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

QT_END_HEADER

#endif //QITEMANIMATION_H
