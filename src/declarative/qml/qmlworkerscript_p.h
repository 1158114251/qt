/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QMLWORKERSCRIPT_P_H
#define QMLWORKERSCRIPT_P_H

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

#include <QtCore/qthread.h>
#include <QtDeclarative/qml.h>
#include <QtDeclarative/qmlparserstatus.h>
#include <QtScript/qscriptvalue.h>
#include <QtCore/qurl.h>

class QmlWorkerScriptEnginePrivate;
class QmlWorkerScriptEngine : public QThread
{
Q_OBJECT
public:
    QmlWorkerScriptEngine(QObject *parent = 0);
    virtual ~QmlWorkerScriptEngine();

    class Data;
    class WorkerScript {
    public:
        void sendMessage(Data *);
        void executeUrl(const QUrl &);

    private:
        WorkerScript();
        friend class QmlWorkerScriptEngine;
        QmlWorkerScriptEngine *engine;
        int id;
    };
    WorkerScript *createWorkerScript();

    struct Data {
        QVariant var;
    };

    void executeUrl(WorkerScript *, const QUrl &);
    void sendMessage(WorkerScript *, Data *);

protected:
    virtual void run();

private:
    QmlWorkerScriptEnginePrivate *d;
};

class QmlWorkerScript : public QObject, public QmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged);

public:
    QmlWorkerScript(QObject *parent = 0);
    virtual ~QmlWorkerScript();

    QUrl source() const;
    void setSource(const QUrl &);

public slots:
    void sendMessage(const QScriptValue &);

signals:
    void sourceChanged();

protected:
    virtual void componentComplete();

private:
    QmlWorkerScriptEngine::WorkerScript *m_script;
    QUrl m_source;
};
QML_DECLARE_TYPE(QmlWorkerScript);

#endif // QMLWORKERSCRIPT_P_H
