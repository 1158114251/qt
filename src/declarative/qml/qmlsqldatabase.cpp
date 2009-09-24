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

#include <QtCore/qobject.h>
#include <QtDeclarative/qmlengine.h>
#include <private/qmlengine_p.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptvalueiterator.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptclasspropertyiterator.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlrecord.h>
#include <private/qmlrefcount_p.h>
#include <private/qmlengine_p.h>
#include <QtCore/qstack.h>
#include <QtCore/qcryptographichash.h>
#include "qmlsqldatabase_p.h"
#include <QtCore/qsettings.h>
#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>

Q_DECLARE_METATYPE(QSqlDatabase)
Q_DECLARE_METATYPE(QSqlQuery)

class QmlSqlQueryScriptClassPropertyIterator;

class QmlSqlQueryScriptClass: public QScriptClass {
public:
    QmlSqlQueryScriptClass(QScriptEngine *engine) : QScriptClass(engine)
    {
        str_length = engine->toStringHandle(QLatin1String("length"));
        str_forwardOnly = engine->toStringHandle(QLatin1String("forwardOnly")); // not in HTML5 (an optimization)
    }

    QueryFlags queryProperty(const QScriptValue &object,
                             const QScriptString &name,
                             QueryFlags flags, uint *id)
    {
        if (flags & HandlesReadAccess) {
            if (name == str_length) {
                return HandlesReadAccess;
            } else if (name == str_forwardOnly) {
                return flags;
            } else {
                bool ok;
                qint32 pos = name.toString().toInt(&ok);
                if (pos < 0 || !ok)
                    return 0;
                QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object.data());
                *id = pos;
                if (*id < (uint)query.size())
                    return HandlesReadAccess;
            }
        }
        if (flags & HandlesWriteAccess)
            if (name == str_forwardOnly)
                return flags;
        return 0;
    }

    QScriptValue property(const QScriptValue &object,
                          const QScriptString &name, uint id)
    {
        QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object.data());
        if (name == str_length) {
            int s = query.size();
            if (s<0) {
                // Inefficient.
                query.last();
                return query.at()+1;
            } else {
                return s;
            }
        } else if (name == str_forwardOnly) {
            return query.isForwardOnly();
        } else {
            if ((uint)query.at() == id || query.seek(id)) { // Qt 4.6 doesn't optimize seek(at())
                QSqlRecord r = query.record();
                QScriptValue row = engine()->newObject();
                for (int j=0; j<r.count(); ++j) {
                    row.setProperty(r.fieldName(j), QScriptValue(engine(),r.value(j).toString())); // XXX only strings
                }
                return row;
            }
        }
        return engine()->undefinedValue();
    }

    void setProperty(QScriptValue &object,
                      const QScriptString &name, uint, const QScriptValue & value)
    {
        if (name == str_forwardOnly) {
            QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object.data());
            query.setForwardOnly(value.toBool());
        }
    }

    QScriptValue::PropertyFlags propertyFlags(const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
    {
        if (name == str_length) {
            return QScriptValue::Undeletable
                | QScriptValue::SkipInEnumeration;
        }
        return QScriptValue::Undeletable;
    }

    QScriptClassPropertyIterator *newIterator(const QScriptValue &object);

private:
    QScriptString str_length;
    QScriptString str_forwardOnly;
};

class QmlSqlQueryScriptClassPropertyIterator : public QScriptClassPropertyIterator
{
public:
    QmlSqlQueryScriptClassPropertyIterator(const QScriptValue &object)
        : QScriptClassPropertyIterator(object)
    {
        toFront();
    }

    ~QmlSqlQueryScriptClassPropertyIterator()
    {
    }

    bool hasNext() const
    {
        QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object().data());
        return query.at() == m_index || query.seek(m_index); // Qt 4.6 doesn't optimize seek(at())
    }

    void next()
    {
        m_last = m_index;
        ++m_index;
    }

    bool hasPrevious() const
    {
        return (m_index > 0);
    }

    void previous()
    {
        --m_index;
        m_last = m_index;
    }

    void toFront()
    {
        m_index = 0;
        m_last = -1;
    }

    void toBack()
    {
        QSqlQuery query = qscriptvalue_cast<QSqlQuery>(object().data());
        m_index = query.size();
        m_last = -1;
    }

    QScriptString name() const
    {
        return object().engine()->toStringHandle(QString::number(m_last));
    }

    uint id() const
    {
        return m_last;
    }

private:
    int m_index;
    int m_last;
};

QScriptClassPropertyIterator *QmlSqlQueryScriptClass::newIterator(const QScriptValue &object)
{
    return new QmlSqlQueryScriptClassPropertyIterator(object);
}



static QScriptValue qmlsqldatabase_item(QScriptContext *context, QScriptEngine *engine)
{
    QSqlQuery query = qscriptvalue_cast<QSqlQuery>(context->thisObject().data());
    int i = context->argument(0).toNumber();
    if (query.at() == i || query.seek(i)) { // Qt 4.6 doesn't optimize seek(at())
        QSqlRecord r = query.record();
        QScriptValue row = engine->newObject();
        for (int j=0; j<r.count(); ++j) {
            row.setProperty(r.fieldName(j), QScriptValue(engine,r.value(j).toString()));
        }
        return row;
    }
    return engine->undefinedValue();
}

static QScriptValue qmlsqldatabase_executeSql(QScriptContext *context, QScriptEngine *engine)
{
    QSqlDatabase db = qscriptvalue_cast<QSqlDatabase>(context->thisObject());
    QString sql = context->argument(0).toString();
    QScriptValue values = context->argument(1);
    QScriptValue cb = context->argument(2);
    QScriptValue cberr = context->argument(3);
    QSqlQuery query(db);
    bool err = false;
    if (query.prepare(sql)) {
        if (values.isArray()) {
            for (QScriptValueIterator it(values); it.hasNext();) {
                it.next();
                query.addBindValue(it.value().toVariant());
            }
        } else {
            query.bindValue(0,values.toVariant());
        }
        if (query.exec()) {
            QScriptValue rs = engine->newObject();
            if (!QmlEnginePrivate::get(engine)->sqlQueryClass)
                QmlEnginePrivate::get(engine)->sqlQueryClass= new QmlSqlQueryScriptClass(engine);
            QScriptValue rows = engine->newObject(QmlEnginePrivate::get(engine)->sqlQueryClass);
            rows.setData(engine->newVariant(qVariantFromValue(query)));
            rows.setProperty(QLatin1String("item"), engine->newFunction(qmlsqldatabase_item,1), QScriptValue::SkipInEnumeration);
            rs.setProperty(QLatin1String("rows"),rows);
            rs.setProperty(QLatin1String("rowsAffected"),query.numRowsAffected());
            rs.setProperty(QLatin1String("insertId"),query.lastInsertId().toString()); // XXX only string
            cb.call(QScriptValue(), QScriptValueList() << context->thisObject() << rs);
        } else {
            err = true;
        }
    } else {
        err = true;
    }
    if (err) {
        QScriptValue error = engine->newObject();
        error.setProperty(QLatin1String("message"), query.lastError().text());
        cberr.call(QScriptValue(), QScriptValueList() << context->thisObject() << error);
    }
    return engine->undefinedValue();
}

static QScriptValue qmlsqldatabase_transaction(QScriptContext *context, QScriptEngine *engine)
{
    QSqlDatabase db = qscriptvalue_cast<QSqlDatabase>(context->thisObject());
    if (context->argumentCount() < 1)
        return engine->undefinedValue();
    QScriptValue cb = context->argument(0);
    if (!cb.isFunction())
        return engine->undefinedValue();

    // Call synchronously...  - XXX could do asynch with threads
    QScriptValue instance = engine->newObject();
    instance.setProperty(QLatin1String("executeSql"), engine->newFunction(qmlsqldatabase_executeSql,4));
    QScriptValue tx = engine->newVariant(instance,qVariantFromValue(db));

    db.transaction();
    cb.call(QScriptValue(), QScriptValueList() << tx);
    if (engine->hasUncaughtException()) {
        db.rollback();
        QScriptValue cb = context->argument(1);
        if (cb.isFunction())
            cb.call();
    } else {
        db.commit();
        QScriptValue cb = context->argument(2);
        if (cb.isFunction())
            cb.call();
    }
    return engine->undefinedValue();
}


static QScriptValue qmlsqldatabase_open(QScriptContext *context, QScriptEngine *engine)
{
    QSqlDatabase database;

    QString dbname = context->argument(0).toString();
    QString dbversion = context->argument(1).toString();
    QString dbdescription = context->argument(2).toString();
    int dbestimatedsize = context->argument(3).toNumber();

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(dbname.toUtf8());
    md5.addData(dbversion.toUtf8());
    QString dbid(QLatin1String(md5.result().toHex()));

    // Uses SQLLITE (like HTML5), but any could be used.

    if (QSqlDatabase::connectionNames().contains(dbid)) {
        database = QSqlDatabase::database(dbid);
    } else {
        database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), dbid);
    }
    if (!database.isOpen()) {
        QString basename = QmlEnginePrivate::get(engine)->offlineStoragePath + QLatin1String("/Databases/");
        QDir().mkpath(basename);
        basename += dbid;
        database.setDatabaseName(basename+QLatin1String(".sqllite"));
        QSettings ini(basename+QLatin1String(".ini"),QSettings::IniFormat);
        ini.setValue(QLatin1String("Name"), dbname);
        ini.setValue(QLatin1String("Version"), dbversion);
        ini.setValue(QLatin1String("Description"), dbdescription);
        ini.setValue(QLatin1String("EstimatedSize"), dbestimatedsize);
        database.open();
    }

    QScriptValue instance = engine->newObject();
    instance.setProperty(QLatin1String("transaction"), engine->newFunction(qmlsqldatabase_transaction,3));
    return engine->newVariant(instance,qVariantFromValue(database));
}

void qt_add_qmlsqldatabase(QScriptEngine *engine)
{
    QScriptValue openDatabase = engine->newFunction(qmlsqldatabase_open, 4);
    engine->globalObject().setProperty(QLatin1String("openDatabase"), openDatabase);
}

/*
HTML5 "spec" says "rs.rows[n]", but WebKit only impelments "rs.rows.item(n)". We do both (and property iterator).
We add a "forwardOnly" property that stops Qt caching results (code promises to only go forward
through the data.
*/

