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

#ifndef QMLCOMPILER_P_H
#define QMLCOMPILER_P_H

#include <QtCore/qbytearray.h>
#include <QtCore/qset.h>
#include <qml.h>
#include <private/qmlinstruction_p.h>
#include <private/qmlcompositetypemanager_p.h>
class QStringList;

QT_BEGIN_NAMESPACE
class QmlXmlParser;
class QmlEngine;
class QmlComponent;
class QmlCompiledComponent;

namespace QmlParser {
    class Object;
    class Property;
    class Value;
};

class QmlCompiledData 
{
public:
    QmlCompiledData();
    QmlCompiledData(const QmlCompiledData &other);
    QmlCompiledData &operator=(const QmlCompiledData &other);
    virtual ~QmlCompiledData();

    QByteArray name;
    QUrl url;

    struct TypeReference 
    {
        TypeReference()
        : type(0), component(0), parser(0), ref(0) {}

        QByteArray className;
        QmlType *type;
        QmlComponent *component;
        QmlCustomParser *parser;

        QmlRefCount *ref;
        QObject *createInstance() const;
    };
    QList<TypeReference> types;
    struct CustomTypeData
    {
        int index;
        int type;
    };
    QList<QString> primitives;
    QList<float> floatData;
    QList<int> intData;
    QList<CustomTypeData> customTypeData;
    QList<QByteArray> datas;
    QList<QMetaObject *> mos;
    QList<QmlInstruction> bytecode;

private:
    friend class QmlCompiler;
    int indexForString(const QString &);
    int indexForByteArray(const QByteArray &);
    int indexForFloat(float *, int);
    int indexForInt(int *, int);
};

class Q_DECLARATIVE_EXPORT QmlCompiler 
{
public:
    QmlCompiler();

    bool compile(QmlEngine *, QmlCompositeTypeData *, QmlCompiledComponent *);

    bool isError() const;
    qint64 errorLine() const;
    QString errorDescription() const;

    static bool isValidId(const QString &);
    static bool isBinding(const QString &);
    static bool isAttachedProperty(const QByteArray &);

    enum StoreInstructionResult { Ok, UnknownType, InvalidData, ReadOnly };
    static StoreInstructionResult 
        generateStoreInstruction(QmlCompiledData &data,
                                 QmlInstruction &instr, 
                                 const QMetaProperty &prop, 
                                 int index, 
                                 int primitive, 
                                 const QString *string);
private:
    void reset(QmlCompiledComponent *, bool);

    void compileTree(QmlParser::Object *tree);
    bool compileObject(QmlParser::Object *obj, int);
    bool compileComponent(QmlParser::Object *obj, int);
    bool compileComponentFromRoot(QmlParser::Object *obj, int);
    bool compileFetchedObject(QmlParser::Object *obj, int);
    bool compileSignal(QmlParser::Property *prop, QmlParser::Object *obj);
    bool compileProperty(QmlParser::Property *prop, QmlParser::Object *obj, int);
    bool compileIdProperty(QmlParser::Property *prop, 
                           QmlParser::Object *obj);
    bool compileAttachedProperty(QmlParser::Property *prop, 
                                 QmlParser::Object *obj,
                                 int ctxt);
    bool compileNestedProperty(QmlParser::Property *prop,
                               int ctxt);
    bool compileListProperty(QmlParser::Property *prop,
                             QmlParser::Object *obj,
                             int ctxt);
    bool compilePropertyAssignment(QmlParser::Property *prop,
                                   QmlParser::Object *obj,
                                   int ctxt);
    bool compilePropertyObjectAssignment(QmlParser::Property *prop,
                                         QmlParser::Object *obj,
                                         QmlParser::Value *value,
                                         int ctxt);
    bool compilePropertyLiteralAssignment(QmlParser::Property *prop,
                                          QmlParser::Object *obj,
                                          QmlParser::Value *value,
                                          int ctxt);

    bool findDynamicProperties(QmlParser::Property *prop,
                               QmlParser::Object *obj);
    bool findDynamicSignals(QmlParser::Property *sigs,
                            QmlParser::Object *obj);

    bool compileDynamicPropertiesAndSignals(QmlParser::Object *obj);
    void compileBinding(const QString &, QmlParser::Property *prop,
                        int ctxt, const QMetaObject *, qint64);

    int optimizeExpressions(int start, int end, int patch = -1);

    QSet<QString> ids;
    qint64 exceptionLine;
    QString exceptionDescription;
    QmlCompiledData *output;
};

QT_END_NAMESPACE
#endif // QMLCOMPILER_P_H
