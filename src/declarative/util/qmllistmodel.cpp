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

#include <QtCore/qdebug.h>
#include <QtCore/qstack.h>
#include <QXmlStreamReader>
#include <private/qmlcustomparser_p.h>
#include "qmlopenmetaobject.h"
#include <qmlcontext.h>
#include <qmlbindablevalue.h>
#include "qmllistmodel.h"

QT_BEGIN_NAMESPACE

#define DATA_ROLE_ID 1
#define DATA_ROLE_NAME "data"

struct ListInstruction
{
    enum { Push, Pop, Value, Set } type;
    int dataIdx;
};

struct ListModelData
{
    int dataOffset;
    int instrCount;
    ListInstruction *instructions() const { return (ListInstruction *)((char *)this + sizeof(ListModelData)); }
};

Q_DECLARE_METATYPE(QListModelInterface *);

/*!
    \qmlclass ListModel 
    \brief The ListModel element defines a free-form list data source.

    The ListModel is a simple XML heirarchy of items containing data roles.
    For example:

    \code
    <ListModel id="FruitModel">
        <Fruit>
            <name>Apple</name>
            <cost>2.45</cost>
        <Fruit>
        <Fruit>
            <name>Orange</name>
            <cost>3.25</cost>
        </Fruit>
        <Fruit>
            <name>Banana</name>
            <cost>1.95</cost>
        </Fruit>
    </ListModel>
    \endcode

    Elements beginning with a capital are items.  Elements beginning
    with lower-case are the data roles.  The above example defines a
    ListModel containing three items, with the roles "name" and "cost".

    The defined model can be used in views such as ListView:
    \code
    <Component id="FruitDelegate">
        <Item width="200" height="50">
            <Text text="{name}"/>
            <Text text="{'$'+cost}" anchors.right="{parent.right}"/>
        </Item>
    </Component>

    <ListView model="{FruitModel}" delegate="{FruitDelegate}" anchors.fill="{parent}"/>
    \endcode
*/

struct ModelNode;
class ListModel : public QListModelInterface
{
    Q_OBJECT
public:
    ListModel(QObject *parent=0);

    virtual QList<int> roles() const;
    virtual QString toString(int role) const;
    Q_PROPERTY(int count READ count);
    virtual int count() const;
    virtual QHash<int,QVariant> data(int index, const QList<int> &roles = (QList<int>())) const;

private:
    QVariant valueForNode(ModelNode *) const;
    mutable QStringList roleStrings;
    friend class ListModelParser;
    friend struct ModelNode;

    void checkRoles() const;
    void addRole(const QString &) const;
    mutable bool _rolesOk;
    ModelNode *_root;
};

class ModelObject : public QObject
{
    Q_OBJECT
public:
    ModelObject(ModelNode *);

    void setValue(const QByteArray &name, const QVariant &val)
    {
        _mo->setValue(name, val);
    }

private:
    ModelNode *_node;
    bool _haveProperties;
    QmlOpenMetaObject *_mo;
};

struct ModelNode
{
    ModelNode();
    ~ModelNode();
    QString className;

    QList<QVariant> values;
    QHash<QString, ModelNode *> properties;

    ListModel *model() {
        if (!modelCache) { 
            modelCache = new ListModel;
            modelCache->_root = this; 
        }
        return modelCache;
    }

    ModelObject *object() {
        if (!objectCache) {
            objectCache = new ModelObject(this);
            QHash<QString, ModelNode *>::iterator it;
            for (it = properties.begin(); it != properties.end(); ++it) {
                if (!(*it)->values.isEmpty())
                    objectCache->setValue(it.key().toLatin1(), (*it)->values.first());
            }
        }
        return objectCache;
    }

    ListModel *modelCache;
    ModelObject *objectCache;
};
Q_DECLARE_METATYPE(ModelNode *);

ModelObject::ModelObject(ModelNode *node)
: _node(node), _haveProperties(false), _mo(new QmlOpenMetaObject(this))
{
}

ListModel::ListModel(QObject *parent)
: QListModelInterface(parent), _rolesOk(false), _root(0)
{
}

void ListModel::checkRoles() const
{
    if (_rolesOk)
        return;

    for (int ii = 0; ii < _root->values.count(); ++ii) {
        ModelNode *node = qvariant_cast<ModelNode *>(_root->values.at(ii));
        if (node) {
            foreach (const QString &role, node->properties.keys())
                addRole(role);
        } 
    }

    _rolesOk = true;
}

void ListModel::addRole(const QString &role) const
{
    if (!roleStrings.contains(role))
        roleStrings << role;
}

QList<int> ListModel::roles() const
{
    checkRoles();
    QList<int> rv;
    for (int ii = 0; ii < roleStrings.count(); ++ii)
        rv << ii;
    return rv;
}

QString ListModel::toString(int role) const
{
    checkRoles();
    if (role < roleStrings.count())
        return roleStrings.at(role);
    else
        return QString();
}

QVariant ListModel::valueForNode(ModelNode *node) const
{
    QObject *rv = 0;

    if (!node->properties.isEmpty()) {
        // Object
        rv = node->object();
    } else if (node->values.count() == 0) {
        // Invalid
        return QVariant();
    } else if (node->values.count() == 1) {
        // Value
        QVariant &var = node->values[0];
        ModelNode *valueNode = qvariant_cast<ModelNode *>(var);
        if (valueNode) {
            if (!valueNode->properties.isEmpty())
                rv = valueNode->object();
            else
                rv = valueNode->model();
        } else {
            return var;
        }
    } else if (node->values.count() > 1) {
        // List
        rv = node->model();
    }

    if (rv)
        return QVariant::fromValue(rv);
    else
        return QVariant();
}

QHash<int,QVariant> ListModel::data(int index, const QList<int> &roles) const
{
    checkRoles();
    QHash<int, QVariant> rv;
    if (index >= count())
        return rv;

    ModelNode *node = qvariant_cast<ModelNode *>(_root->values.at(index));
    if (!node) 
        return rv;

    for (int ii = 0; ii < roles.count(); ++ii) {
        const QString &roleString = roleStrings.at(roles.at(ii));

        QHash<QString, ModelNode *>::ConstIterator iter = 
            node->properties.find(roleString);
        if (iter != node->properties.end()) {
            ModelNode *row = *iter;
            rv.insert(roles.at(ii), valueForNode(row));
        }
    }

    return rv;
}

int ListModel::count() const
{
    if (!_root) return 0;
    return _root->values.count();
}

class ListModelParser : public QmlCustomParser
{
public:
    QByteArray compile(const QList<QmlCustomParserProperty> &, bool *ok);
    bool compileProperty(const QmlCustomParserProperty &prop, QList<ListInstruction> &instr, QByteArray &data);
    void setCustomData(QObject *, const QByteArray &);
};

bool ListModelParser::compileProperty(const QmlCustomParserProperty &prop, QList<ListInstruction> &instr, QByteArray &data)
{
    QList<QVariant> values = prop.assignedValues();
    for(int ii = 0; ii < values.count(); ++ii) {
        const QVariant &value = values.at(ii);

        if(value.userType() == qMetaTypeId<QmlCustomParserNode>()) {
            QmlCustomParserNode node = 
                qvariant_cast<QmlCustomParserNode>(value);

            {
            ListInstruction li;
            li.type = ListInstruction::Push;
            li.dataIdx = -1;
            instr << li;
            }

            QList<QmlCustomParserProperty> props = node.properties();
            for(int jj = 0; jj < props.count(); ++jj) {
                const QmlCustomParserProperty &nodeProp = props.at(jj);
                if(nodeProp.name() == "")
                    return false;

                ListInstruction li;
                int ref = data.count();
                data.append(nodeProp.name());
                data.append('\0');
                li.type = ListInstruction::Set;
                li.dataIdx = ref;
                instr << li;

                if(!compileProperty(nodeProp, instr, data))
                    return false;

                li.type = ListInstruction::Pop;
                li.dataIdx = -1;
                instr << li;
            }

            {
            ListInstruction li;
            li.type = ListInstruction::Pop;
            li.dataIdx = -1;
            instr << li;
            }

        } else {

            int ref = data.count();
            QByteArray d = value.toString().toLatin1();
            d.append('\0');
            data.append(d);

            ListInstruction li;
            li.type = ListInstruction::Value;
            li.dataIdx = ref;
            instr << li;

        }
    }

    return true;
}

QByteArray ListModelParser::compile(const QList<QmlCustomParserProperty> &customProps, bool *ok)
{
    *ok = true;
    QList<ListInstruction> instr;
    QByteArray data;

    for(int ii = 0; ii < customProps.count(); ++ii) {
        const QmlCustomParserProperty &prop = customProps.at(ii);
        if(prop.name() != "") { // isn't default property
            *ok = false;
            return QByteArray();
        }

        if(!compileProperty(prop, instr, data)) {
            *ok = false;
            return QByteArray();
        }
    }

    int size = sizeof(ListModelData) + 
               instr.count() * sizeof(ListInstruction) + 
               data.count();

    QByteArray rv;
    rv.resize(size);

    ListModelData *lmd = (ListModelData *)rv.data();
    lmd->dataOffset = sizeof(ListModelData) + 
                     instr.count() * sizeof(ListInstruction);
    lmd->instrCount = instr.count();
    for (int ii = 0; ii < instr.count(); ++ii)
        lmd->instructions()[ii] = instr.at(ii);
    ::memcpy(rv.data() + lmd->dataOffset, data.constData(), data.count());

    return rv;
}

void ListModelParser::setCustomData(QObject *obj, const QByteArray &d)
{
    ListModel *rv = static_cast<ListModel *>(obj);

    ModelNode *root = new ModelNode;
    rv->_root = root;
    QStack<ModelNode *> nodes;
    nodes << root;

    const ListModelData *lmd = (const ListModelData *)d.constData();
    const char *data = ((const char *)lmd) + lmd->dataOffset;

    for (int ii = 0; ii < lmd->instrCount; ++ii) {
        const ListInstruction &instr = lmd->instructions()[ii];

        switch(instr.type) {
        case ListInstruction::Push:
            {
                ModelNode *n = nodes.top();
                ModelNode *n2 = new ModelNode;
                n->values << qVariantFromValue(n2);
                nodes.push(n2);
            }
            break;

        case ListInstruction::Pop:
            nodes.pop();
            break;

        case ListInstruction::Value:
            {
                ModelNode *n = nodes.top();
                n->values.append(QByteArray(data + instr.dataIdx));
            }
            break;

        case ListInstruction::Set:
            {
                ModelNode *n = nodes.top();
                ModelNode *n2 = new ModelNode;
                n->properties.insert(QLatin1String(data + instr.dataIdx), n2);
                nodes.push(n2);
            }
            break;
        }
    }
}

QML_DECLARE_TYPE(ListModel);
QML_DEFINE_CUSTOM_TYPE(ListModel, ListModel, ListModelParser);

// ### FIXME
class ListElement : public QObject
{
Q_OBJECT
};
QML_DECLARE_TYPE(ListElement);
QML_DEFINE_TYPE(ListElement,ListElement);

static void dump(ModelNode *node, int ind)
{
    QByteArray indentBa(ind * 4, ' ');
    const char *indent = indentBa.constData();

    for (int ii = 0; ii < node->values.count(); ++ii) {
        ModelNode *subNode = qvariant_cast<ModelNode *>(node->values.at(ii));
        if (subNode) {
            qWarning().nospace() << indent << "Sub-node " << ii << ": class " << subNode->className;
            dump(subNode, ind + 1);
        } else {
            qWarning().nospace() << indent << "Sub-node " << ii << ": " << node->values.at(ii).toString();
        }
    }

    for (QHash<QString, ModelNode *>::ConstIterator iter = node->properties.begin(); iter != node->properties.end(); ++iter) {
        qWarning().nospace() << indent << "Property " << iter.key() << ":";
        dump(iter.value(), ind + 1);
    }
}

ModelNode::ModelNode()
: modelCache(0), objectCache(0)
{
}

ModelNode::~ModelNode()
{
    qDeleteAll(properties);
    for (int ii = 0; ii < values.count(); ++ii) {
        ModelNode *node = qvariant_cast<ModelNode *>(values.at(ii));
        if (node) { delete node; node = 0; }
    }
    if (modelCache) { delete modelCache; modelCache = 0; }
}

QT_END_NAMESPACE
#include "qmllistmodel.moc"
