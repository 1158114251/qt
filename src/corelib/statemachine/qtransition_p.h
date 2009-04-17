/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#ifndef QTRANSITION_P_H
#define QTRANSITION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qabstracttransition_p.h"

#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QStateAction;

class QTransition;
class Q_CORE_EXPORT QTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QTransition)
public:
    QTransitionPrivate();
    ~QTransitionPrivate();

    static QTransitionPrivate *get(QTransition *q);
    static const QTransitionPrivate *get(const QTransition *q);

    QList<QStateAction*> actions() const;
};

QT_END_NAMESPACE

#endif
