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

#ifdef Q_OS_WIN // for %APPDATA%
#include "qt_windows.h"
#include "qlibrary.h"
#endif


class QmlSqlDatabaseTransaction : public QObject
{
    Q_OBJECT
public:
    QmlSqlDatabaseTransaction(QSqlDatabase db, QmlEngine *engine) : database(db) {}
    virtual ~QmlSqlDatabaseTransaction(){}
    static QScriptValue executeSql(QScriptContext *context, QScriptEngine *engine)
    {
        QScriptValue tx = context->thisObject();
        QmlSqlDatabaseTransaction *trans = qobject_cast<QmlSqlDatabaseTransaction *>(tx.toQObject());
        QString sql = context->argument(0).toString();
        QScriptValue values = context->argument(1);
        QScriptValue cb = context->argument(2);
        QScriptValue cberr = context->argument(3);
        QSqlQuery query(trans->database);
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
                QScriptValue rows = engine->newArray();
                int i=0;
                for (; query.next(); ++i) {
                    QSqlRecord r = query.record();
                    QScriptValue row = engine->newArray(r.count());
                    for (int j=0; j<r.count(); ++j) {
                        row.setProperty(j, QScriptValue(engine,r.value(j).toString()));
                    }
                    rows.setProperty(i, row);
                }
                QScriptValue rs = engine->newObject();
                rs.setProperty(QLatin1String("rows"), rows);
                cb.call(QScriptValue(), QScriptValueList() << tx << rs);
            } else {
                err = true;
            }
        } else {
            err = true;
        }
        if (err) {
            QScriptValue error = engine->newObject();
            error.setProperty(QLatin1String("message"), query.lastError().text());
            cberr.call(QScriptValue(), QScriptValueList() << tx << error);
        }
        return engine->undefinedValue();
    }

private:
    QSqlDatabase database;
};

class QmlSqlDatabase : public QObject
{
    Q_OBJECT
public:
    QmlSqlDatabase(QmlEngine *engine, QScriptContext *context);
    virtual ~QmlSqlDatabase();

    QScriptValue callback() const;
    void setCallback(const QScriptValue &);

    static QScriptValue transaction(QScriptContext *context, QScriptEngine *engine)
    {
        QmlSqlDatabase *db = qobject_cast<QmlSqlDatabase *>(context->thisObject().toQObject());
        if (!db)
            return context->throwError(QScriptContext::ReferenceError, QLatin1String("Not an SqlDatabase object"));
        if (context->argumentCount() != 1)
            return engine->undefinedValue();
        QScriptValue cb = context->argument(0);
        if (!cb.isFunction())
            return engine->undefinedValue();

        // XXX Call synchronously...
        QScriptValue tx = engine->newQObject(new QmlSqlDatabaseTransaction(db->database, QmlEnginePrivate::getEngine(engine)), QScriptEngine::ScriptOwnership);

        tx.setProperty(QLatin1String("executeSql"), engine->newFunction(QmlSqlDatabaseTransaction::executeSql,4));

        db->database.transaction();
        cb.call(QScriptValue(), QScriptValueList() << tx);
        if (engine->hasUncaughtException()) {
            db->database.rollback();
            QScriptValue cb = context->argument(1);
            if (cb.isFunction())
                cb.call();
        } else {
            db->database.commit();
            QScriptValue cb = context->argument(2);
            if (cb.isFunction())
                cb.call();
        }
        return engine->undefinedValue();
    }

private:
    QSqlDatabase database;
};

// XXX Something like this belongs in Qt.
static QString userLocalDataPath(const QString& app)
{
    QString result;

#ifdef Q_OS_WIN
#ifndef Q_OS_WINCE
    QLibrary library(QLatin1String("shell32"));
#else
    QLibrary library(QLatin1String("coredll"));
#endif // Q_OS_WINCE
    typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
    GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
    if (SHGetSpecialFolderPath) {
        wchar_t path[MAX_PATH];
        SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, FALSE);
        result = QString::fromWCharArray(path);
    }
#endif // Q_OS_WIN

#ifdef Q_OS_MAC
    result = QLatin1String(qgetenv("HOME"));
    result += "/Library/Application Support";
#else
    if (result.isEmpty()) {
        // Fallback: UNIX style
        result = QLatin1String(qgetenv("XDG_DATA_HOME"));
        if (result.isEmpty()) {
            result = QLatin1String(qgetenv("HOME"));
            result += QLatin1String("/.local/share");
        }
    }
#endif

    result += QLatin1Char('/');
    result += app;
    return result;
}

QmlSqlDatabase::QmlSqlDatabase(QmlEngine *engine, QScriptContext *context)
{
    QString dbname = context->argument(0).toString();
    QString dbversion = context->argument(1).toString();
    QString dbdescription = context->argument(2).toString();
    int dbestimatedsize = context->argument(3).toNumber();

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(dbname.utf8());
    md5.addData(dbversion.utf8());
    QString dbid(QLatin1String(md5.result().toHex()));

    QSqlDatabase db;
    if (QSqlDatabase::connectionNames().contains(dbid)) {
        database = QSqlDatabase::database(dbid);
    } else {
        database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), dbid);
    }
    if (!database.isOpen()) {
        QString basename = userLocalDataPath(QLatin1String("Nokia/Qt/QML/Databases/"));
        QDir().mkpath(basename);
        basename += dbid;
        database.setDatabaseName(basename+QLatin1String(".sqllite"));
        QSettings ini(basename+QLatin1String(".ini"),QSettings::IniFormat);
        ini.setValue("Name", dbname);
        ini.setValue("Version", dbversion);
        ini.setValue("Description", dbdescription);
        ini.setValue("EstimatedSize", dbestimatedsize);
        database.open();
    }
}

QmlSqlDatabase::~QmlSqlDatabase()
{
}

static QScriptValue qmlsqldatabase_open(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue proto = engine->newQObject(new QmlSqlDatabase(QmlEnginePrivate::getEngine(engine),context), QScriptEngine::ScriptOwnership);
    proto.setProperty(QLatin1String("transaction"), engine->newFunction(QmlSqlDatabase::transaction,1));
    return proto;
}

void qt_add_qmlsqldatabase(QScriptEngine *engine)
{
    QScriptValue openDatabase = engine->newFunction(qmlsqldatabase_open, 4);
    engine->globalObject().setProperty(QLatin1String("openDatabase"), openDatabase);
}

#include "qmlsqldatabase.moc"
