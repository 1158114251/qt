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

#ifndef QMLCOMPILER_P_H
#define QMLCOMPILER_P_H

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

#include <QtCore/qbytearray.h>
#include <QtCore/qset.h>
#include <qml.h>
#include <qmlerror.h>
#include <private/qmlinstruction_p.h>
#include <private/qmlcompositetypemanager_p.h>
#include <private/qmlparser_p.h>
#include <private/qmlengine_p.h>
#include <private/qbitfield_p.h>
#include <private/qmlpropertycache_p.h>
#include <private/qmlintegercache_p.h>
#include <private/qmltypenamecache_p.h>

QT_BEGIN_NAMESPACE

class QmlEngine;
class QmlComponent;
class QmlContext;

class QScriptProgram;
class Q_AUTOTEST_EXPORT QmlCompiledData : public QmlRefCount
{
public:
    QmlCompiledData();
    virtual ~QmlCompiledData();

    QByteArray name;
    QUrl url;
    QmlEnginePrivate::Imports imports;
    QmlTypeNameCache *importCache;

    struct TypeReference 
    {
        TypeReference()
        : type(0), component(0), ref(0) {}

        QByteArray className;
        QmlType *type;
        QmlComponent *component;

        QmlRefCount *ref;
        QObject *createInstance(QmlContext *, const QBitField &) const;
        const QMetaObject *metaObject() const;
    };
    QList<TypeReference> types;
    struct CustomTypeData
    {
        int index;
        int type;
    };

    const QMetaObject *root;
    QAbstractDynamicMetaObject rootData;
    QList<QString> primitives;
    QList<float> floatData;
    QList<int> intData;
    QList<CustomTypeData> customTypeData;
    QList<QByteArray> datas;
    QList<QmlParser::Location> locations;
    QList<QmlInstruction> bytecode;
    QList<QScriptProgram *> cachedPrograms;
    QList<QScriptValue *> cachedClosures;
    QList<QmlPropertyCache *> propertyCaches;
    QList<QmlIntegerCache *> contextCaches;
    QList<QmlParser::Object::ScriptBlock> scripts;

    void dumpInstructions();
private:
    void dump(QmlInstruction *, int idx = -1);
    QmlCompiledData(const QmlCompiledData &other);
    QmlCompiledData &operator=(const QmlCompiledData &other);
    QByteArray packData;
    friend class QmlCompiler;
    int pack(const char *, size_t);

    int indexForString(const QString &);
    int indexForByteArray(const QByteArray &);
    int indexForFloat(float *, int);
    int indexForInt(int *, int);
    int indexForLocation(const QmlParser::Location &);
    int indexForLocation(const QmlParser::LocationSpan &);
};

class QMetaObjectBuilder;
class Q_DECLARATIVE_EXPORT QmlCompiler
{
public:
    QmlCompiler();

    bool compile(QmlEngine *, QmlCompositeTypeData *, QmlCompiledData *);

    bool isError() const;
    QList<QmlError> errors() const;

    static bool isValidId(const QString &);
    static bool isAttachedPropertyName(const QByteArray &);
    static bool isSignalPropertyName(const QByteArray &);

private:
    static void reset(QmlCompiledData *);

    struct BindingContext {
        BindingContext()
            : stack(0), owner(0), object(0) {}
        BindingContext(QmlParser::Object *o)
            : stack(0), owner(0), object(o) {}
        BindingContext incr() const {
            BindingContext rv(object);
            rv.stack = stack + 1;
            return rv;
        }
        bool isSubContext() const { return stack != 0; }
        int stack;
        int owner;
        QmlParser::Object *object;
    };

    void compileTree(QmlParser::Object *tree);


    bool buildObject(QmlParser::Object *obj, const BindingContext &);
    bool buildScript(QmlParser::Object *obj, QmlParser::Object *script);
    bool buildComponent(QmlParser::Object *obj, const BindingContext &);
    bool buildSubObject(QmlParser::Object *obj, const BindingContext &);
    bool buildSignal(QmlParser::Property *prop, QmlParser::Object *obj, 
                     const BindingContext &);
    bool buildProperty(QmlParser::Property *prop, QmlParser::Object *obj, 
                       const BindingContext &);
    bool buildPropertyInNamespace(QmlEnginePrivate::ImportedNamespace *ns,
                                  QmlParser::Property *prop, 
                                  QmlParser::Object *obj, 
                                  const BindingContext &);
    bool buildIdProperty(QmlParser::Property *prop, QmlParser::Object *obj);
    bool buildAttachedProperty(QmlParser::Property *prop, 
                               QmlParser::Object *obj,
                               const BindingContext &ctxt);
    bool buildGroupedProperty(QmlParser::Property *prop,
                              QmlParser::Object *obj,
                              const BindingContext &ctxt);
    bool buildValueTypeProperty(QObject *type, 
                                QmlParser::Object *obj, 
                                QmlParser::Object *baseObj,
                                const BindingContext &ctxt);
    bool buildListProperty(QmlParser::Property *prop,
                           QmlParser::Object *obj,
                           const BindingContext &ctxt);
    bool buildScriptStringProperty(QmlParser::Property *prop,
                                   QmlParser::Object *obj,
                                   const BindingContext &ctxt);
    bool buildPropertyAssignment(QmlParser::Property *prop,
                                 QmlParser::Object *obj,
                                 const BindingContext &ctxt);
    bool buildPropertyObjectAssignment(QmlParser::Property *prop,
                                       QmlParser::Object *obj,
                                       QmlParser::Value *value,
                                       const BindingContext &ctxt);
    bool buildPropertyLiteralAssignment(QmlParser::Property *prop,
                                        QmlParser::Object *obj,
                                        QmlParser::Value *value,
                                        const BindingContext &ctxt);
    bool doesPropertyExist(QmlParser::Property *prop, QmlParser::Object *obj);
    bool testLiteralAssignment(const QMetaProperty &prop, 
                               QmlParser::Value *value);
    enum DynamicMetaMode { IgnoreAliases, ResolveAliases, ForceCreation };
    bool mergeDynamicMetaProperties(QmlParser::Object *obj);
    bool buildDynamicMeta(QmlParser::Object *obj, DynamicMetaMode mode);
    bool checkDynamicMeta(QmlParser::Object *obj);
    bool buildBinding(QmlParser::Value *, QmlParser::Property *prop,
                      const BindingContext &ctxt);
    bool buildComponentFromRoot(QmlParser::Object *obj, const BindingContext &);
    bool compileAlias(QMetaObjectBuilder &, 
                      QByteArray &data,
                      QmlParser::Object *obj, 
                      const QmlParser::Object::DynamicProperty &);
    bool completeComponentBuild();


    void genObject(QmlParser::Object *obj);
    void genObjectBody(QmlParser::Object *obj);
    void genComponent(QmlParser::Object *obj);
    void genValueProperty(QmlParser::Property *prop, QmlParser::Object *obj);
    void genListProperty(QmlParser::Property *prop, QmlParser::Object *obj);
    void genPropertyAssignment(QmlParser::Property *prop, 
                               QmlParser::Object *obj,
                               QmlParser::Property *valueTypeProperty = 0);
    void genLiteralAssignment(const QMetaProperty &prop, 
                              QmlParser::Value *value);
    void genBindingAssignment(QmlParser::Value *binding, 
                              QmlParser::Property *prop, 
                              QmlParser::Object *obj,
                              QmlParser::Property *valueTypeProperty = 0);
    int genContextCache();

    int genValueTypeData(QmlParser::Property *prop, QmlParser::Property *valueTypeProp);
    int genPropertyData(QmlParser::Property *prop);

    int componentTypeRef();

    static int findSignalByName(const QMetaObject *, const QByteArray &name);
    static QmlType *toQmlType(QmlParser::Object *from);
    bool canCoerce(int to, QmlParser::Object *from);
    bool canCoerce(int to, int from);

    QStringList deferredProperties(QmlParser::Object *);

    void addId(const QString &, QmlParser::Object *);

    void dumpStats();

    struct BindingReference {
        QmlParser::Variant expression;
        QmlParser::Property *property;
        QmlParser::Value *value;
        bool isBasicScript;
        QByteArray compiledData;
        BindingContext bindingContext;
    };
    void addBindingReference(const BindingReference &);

    struct ComponentCompileState
    {
        ComponentCompileState() 
            : parserStatusCount(0), pushedProperties(0), root(0) {}
        QHash<QString, QmlParser::Object *> ids;
        QHash<int, QmlParser::Object *> idIndexes;
        int parserStatusCount;
        int pushedProperties;

        QHash<QmlParser::Value *, BindingReference> bindings;
        QList<QmlParser::Object *> aliasingObjects;
        QmlParser::Object *root;
    };
    ComponentCompileState compileState;

    struct ComponentStat
    {
        ComponentStat() 
            : ids(0), scriptBindings(0), optimizedBindings(0), objects(0) {}

        int lineNumber;

        int ids;
        int scriptBindings;
        int optimizedBindings;
        int objects;
    };
    ComponentStat componentStat;

    void saveComponentState();

    ComponentCompileState componentState(QmlParser::Object *);
    QHash<QmlParser::Object *, ComponentCompileState> savedCompileStates;
    QList<ComponentStat> savedComponentStats;

    QList<QmlError> exceptions;
    QmlCompiledData *output;
    QmlEngine *engine;
    QmlCompositeTypeData *unit;
};
QT_END_NAMESPACE

#endif // QMLCOMPILER_P_H
