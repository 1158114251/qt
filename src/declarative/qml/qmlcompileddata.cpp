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

#include "qmlcompiler_p.h"
#include "qmlengine.h"
#include "qmlcomponent.h"
#include "qmlcomponent_p.h"
#include "qmlcontext.h"
#include "qmlcontext_p.h"
#include <private/qobject_p.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

int QmlCompiledData::pack(const char *data, size_t size)
{
    const char *p = packData.constData();
    unsigned int ps = packData.size();

    for (unsigned int ii = 0; (ii + size) <= ps; ii += sizeof(int)) {
        if (0 == ::memcmp(p + ii, data, size))
            return ii;
    }

    int rv = packData.size();
    packData.append(data, size);
    return rv;
}

int QmlCompiledData::indexForString(const QString &data)
{
    int idx = primitives.indexOf(data);
    if (idx == -1) {
        idx = primitives.count();
        primitives << data;
    }
    return idx;
}

int QmlCompiledData::indexForByteArray(const QByteArray &data)
{
    int idx = datas.indexOf(data);
    if (idx == -1) {
        idx = datas.count();
        datas << data;
    }
    return idx;
}

int QmlCompiledData::indexForFloat(float *data, int count)
{
    Q_ASSERT(count > 0);

    for (int ii = 0; ii <= floatData.count() - count; ++ii) {
        bool found = true;
        for (int jj = 0; jj < count; ++jj) {
            if (floatData.at(ii + jj) != data[jj]) {
                found = false;
                break;
            }
        }

        if (found)
            return ii;
    }

    int idx = floatData.count();
    for (int ii = 0; ii < count; ++ii)
        floatData << data[ii];

    return idx;
}

int QmlCompiledData::indexForInt(int *data, int count)
{
    Q_ASSERT(count > 0);

    for (int ii = 0; ii <= intData.count() - count; ++ii) {
        bool found = true;
        for (int jj = 0; jj < count; ++jj) {
            if (intData.at(ii + jj) != data[jj]) {
                found = false;
                break;
            }
        }

        if (found)
            return ii;
    }

    int idx = intData.count();
    for (int ii = 0; ii < count; ++ii)
        intData << data[ii];

    return idx;
}

int QmlCompiledData::indexForLocation(const QmlParser::Location &l)
{
    // ### FIXME
    int rv = locations.count();
    locations << l;
    return rv;
}

int QmlCompiledData::indexForLocation(const QmlParser::LocationSpan &l)
{
    // ### FIXME
    int rv = locations.count();
    locations << l.start << l.end;
    return rv;
}

QmlCompiledData::QmlCompiledData()
{
}

QmlCompiledData::~QmlCompiledData()
{
    for (int ii = 0; ii < types.count(); ++ii) {
        if (types.at(ii).ref)
            types.at(ii).ref->release();
    }
}

QObject *QmlCompiledData::TypeReference::createInstance(QmlContext *ctxt) const
{
    if (type) {
        QObject *rv = type->create();
        if (rv)
            QmlEngine::setContextForObject(rv, ctxt);
        return rv;
    } else {
        Q_ASSERT(component);
        return component->create(ctxt);
    } 
}

const QMetaObject *QmlCompiledData::TypeReference::metaObject() const
{
    if (type) {
        return type->metaObject();
    } else {
        Q_ASSERT(component);
        return &static_cast<QmlComponentPrivate *>(QObjectPrivate::get(component))->cc->root;
    }
}

void QmlCompiledData::dumpInstructions()
{
    if (!name.isEmpty())
        qWarning() << name;
    qWarning() << "Index\tLine\tOperation\t\tData1\tData2\t\tComments";
    qWarning() << "-------------------------------------------------------------------------------";
    for (int ii = 0; ii < bytecode.count(); ++ii) {
        dump(&bytecode[ii], ii);
    }
    qWarning() << "-------------------------------------------------------------------------------";
}


QT_END_NAMESPACE
