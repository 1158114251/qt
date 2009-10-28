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
#include <private/qmlparser_p.h>
#include "qmlopenmetaobject.h"
#include <qmlcontext.h>
#include "qmllistmodel.h"
#include <QtScript/qscriptvalueiterator.h>

Q_DECLARE_METATYPE(QListModelInterface *)

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

static void dump(ModelNode *node, int ind);

/*!
    \qmlclass ListModel 
    \brief The ListModel element defines a free-form list data source.

    The ListModel is a simple hierarchy of elements containing data roles. The contents can
    be defined dynamically, or explicitly in QML:

    For example:

    \code
    ListModel {
        id: fruitModel
        ListElement {
            name: "Apple"
            cost: 2.45
        }
        ListElement {
            name: "Orange"
            cost: 3.25
        }
        ListElement {
            name: "Banana"
            cost: 1.95
        }
    }
    \endcode

    Roles (properties) must begin with a lower-case letter.  The above example defines a
    ListModel containing three elements, with the roles "name" and "cost".

    The defined model can be used in views such as ListView:
    \code
    Component {
        id: fruitDelegate
        Item {
            width: 200; height: 50
            Text { text: name }
            Text { text: '$'+cost; anchors.right: parent.right }
        }
    }

    ListView {
        model: fruitModel
        delegate: fruitDelegate
        anchors.fill: parent
    }
    \endcode

    It is possible for roles to contain list data.  In the example below we create a list of fruit attributes:

    \code
    ListModel {
        id: fruitModel
        ListElement {
            name: "Apple"
            cost: 2.45
            attributes: [
                ListElement { description: "Core" },
                ListElement { description: "Deciduous" }
            ]
        }
        ListElement {
            name: "Orange"
            cost: 3.25
            attributes: [
                ListElement { description: "Citrus" }
            ]
        }
        ListElement {
            name: "Banana"
            cost: 1.95
            attributes: [
                ListElement { description: "Tropical" },
                ListElement { description: "Seedless" }
            ]
        }
    }
    \endcode

    The delegate below will list all the fruit attributes:
    \code
    Component {
        id: fruitDelegate
        Item {
            width: 200; height: 50
            Text { id: Name; text: name }
            Text { text: '$'+cost; anchors.right: parent.right }
            Row {
                anchors.top: Name.bottom
                spacing: 5
                Text { text: "Attributes:" }
                Repeater {
                    dataSource: attributes
                    Component { Text { text: description } }
                }
            }
        }
    }
    \endcode

    The content of a ListModel may be created and modified using the clear(),
    append(), and set() methods.  For example:

    \code
    Component {
        id: fruitDelegate
        Item {
            width: 200; height: 50
            Text { text: name }
            Text { text: '$'+cost; anchors.right: parent.right }

            // Double the price when clicked.
            MouseRegion {
                anchors.fill: parent
                onClicked: fruitModel.set(index, "cost", cost*2)
            }
        }
    }
    \endcode

    When creating content dynamically, note that the set of available properties cannot be changed
    except by first clearing the model - whatever properties are first added are then the
    only permitted properties in the model.
*/

class ModelObject : public QObject
{
    Q_OBJECT
public:
    ModelObject();

    void setValue(const QByteArray &name, const QVariant &val)
    {
        _mo->setValue(name, val);
    }

private:
    QmlOpenMetaObject *_mo;
};

struct ModelNode
{
    ModelNode();
    ~ModelNode();

    QList<QVariant> values;
    QHash<QString, ModelNode *> properties;

    QmlListModel *model() {
        if (!modelCache) { 
            modelCache = new QmlListModel;
            modelCache->_root = this; 
        }
        return modelCache;
    }

    ModelObject *object() {
        if (!objectCache) {
            objectCache = new ModelObject();
            QHash<QString, ModelNode *>::iterator it;
            for (it = properties.begin(); it != properties.end(); ++it) {
                if (!(*it)->values.isEmpty())
                    objectCache->setValue(it.key().toUtf8(), (*it)->values.first());
            }
        }
        return objectCache;
    }

    void setObjectValue(const QScriptValue& valuemap);
    void setListValue(const QScriptValue& valuelist);

    void setProperty(const QString& prop, const QVariant& val) {
        QHash<QString, ModelNode *>::const_iterator it = properties.find(prop);
        if (it != properties.end()) {
            (*it)->values[0] = val;
        } else {
            ModelNode *n = new ModelNode;
            n->values << val;
            properties.insert(prop,n);
        }
        if (objectCache)
            objectCache->setValue(prop.toUtf8(), val);
    }

    QmlListModel *modelCache;
    ModelObject *objectCache;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(ModelNode *)

QT_BEGIN_NAMESPACE
void ModelNode::setObjectValue(const QScriptValue& valuemap) {
    QScriptValueIterator it(valuemap);
    while (it.hasNext()) {
        it.next();
        ModelNode *value = new ModelNode;
        QScriptValue v = it.value();
        if (v.isArray()) {
            value->setListValue(v);
        } else if (v.isObject()) {
            value->setObjectValue(v);
        } else {
            value->values << v.toVariant();
        }
        properties.insert(it.name(),value);
    }
}

void ModelNode::setListValue(const QScriptValue& valuelist) {
    QScriptValueIterator it(valuelist);
    values.clear();
    while (it.hasNext()) {
        it.next();
        ModelNode *value = new ModelNode;
        QScriptValue v = it.value();
        if (v.isArray()) {
            value->setListValue(v);
        } else if (v.isObject()) {
            value->setObjectValue(v);
        } else {
            value->values << v.toVariant();
        }
        values.append(qVariantFromValue(value));

    }
}


ModelObject::ModelObject()
: _mo(new QmlOpenMetaObject(this))
{
}

QmlListModel::QmlListModel(QObject *parent)
: QListModelInterface(parent), _rolesOk(false), _root(0)
{
}

QmlListModel::~QmlListModel()
{
    delete _root;
}

void QmlListModel::checkRoles() const
{
    if (_rolesOk || !_root)
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

void QmlListModel::addRole(const QString &role) const
{
    if (!roleStrings.contains(role))
        roleStrings << role;
}

QList<int> QmlListModel::roles() const
{
    checkRoles();
    QList<int> rv;
    for (int ii = 0; ii < roleStrings.count(); ++ii)
        rv << ii;
    return rv;
}

QString QmlListModel::toString(int role) const
{
    checkRoles();
    if (role < roleStrings.count())
        return roleStrings.at(role);
    else
        return QString();
}

QVariant QmlListModel::valueForNode(ModelNode *node) const
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

QHash<int,QVariant> QmlListModel::data(int index, const QList<int> &roles) const
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

/*!
    \qmlproperty int ListModel::count
    The number of data entries in the model.
*/
int QmlListModel::count() const
{
    if (!_root) return 0;
    return _root->values.count();
}

/*!
    \qmlmethod ListModel::clear()

    Deletes all content from the model. The properties are cleared such that
    different properties may be set on subsequent additions.

    \sa append() remove()
*/
void QmlListModel::clear()
{
    int cleared = count();
    _rolesOk = false;
    delete _root;
    _root = 0;
    roleStrings.clear();
    emit itemsRemoved(0,cleared);
}

/*!
    \qmlmethod ListModel::remove(int index)

    Deletes the content at \a index from the model.

    \sa clear()
*/
void QmlListModel::remove(int index)
{
    if (_root) {
        ModelNode *node = qvariant_cast<ModelNode *>(_root->values.at(index));
        _root->values.removeAt(index);
        if (node)
            delete node;
        emit itemsRemoved(index,1);
    }
}

/*!
    \qmlmethod ListModel::insert(index,dict)

    Adds a new item to the list model at position \a index, with the
    values in \a dict.

    \code
        FruitModel.insert(2, {"cost": 5.95, "name":"Pizza"})
    \endcode

    The \a index must be to an existing item in the list, or one past
    the end of the list (equivalent to append).

    \sa set() append()
*/
void QmlListModel::insert(int index, const QScriptValue& valuemap)
{
    if (!_root)
        _root = new ModelNode;
    if (index >= _root->values.count()) {
        if (index == _root->values.count())
            append(valuemap);
        return;
    }
    ModelNode *mn = new ModelNode;
    mn->setObjectValue(valuemap);
    _root->values.insert(index,qVariantFromValue(mn));
    emit itemsInserted(index,1);
}

/*!
    \qmlmethod ListModel::move(from,to,n)

    Moves \a n items \a from one position \a to another.

    The from and to ranges must exist; for example, to move the first 3 items
    to the end of the list:

    \code
        FruitModel.move(0,FruitModel.count-3,3)
    \endcode

    \sa append()
*/
void QmlListModel::move(int from, int to, int n)
{
    if (from+n > count() || to+n > count() || n==0 || from==to || from < 0 || to < 0)
        return;
    int origfrom=from; // preserve actual move, so any animations are correct
    int origto=to;
    int orign=n;
    if (from > to) {
        // Only move forwards - flip if backwards moving
        int tfrom = from;
        int tto = to;
        from = tto;
        to = tto+n;
        n = tfrom-tto;
    }
    if (n==1) {
        _root->values.move(from,to);
    } else {
        QList<QVariant> replaced;
        int i=0;
        QVariantList::const_iterator it=_root->values.begin(); it += from+n;
        for (; i<to-from; ++i,++it)
            replaced.append(*it);
        i=0;
        it=_root->values.begin(); it += from;
        for (; i<n; ++i,++it)
            replaced.append(*it);
        QVariantList::const_iterator f=replaced.begin();
        QVariantList::iterator t=_root->values.begin(); t += from;
        for (; f != replaced.end(); ++f, ++t)
            *t = *f;
    }
    emit itemsMoved(origfrom,origto,orign);
}

/*!
    \qmlmethod ListModel::append(dict)

    Adds a new item to the end of the list model, with the
    values in \a dict.

    \code
        FruitModel.append({"cost": 5.95, "name":"Pizza"})
    \endcode

    \sa set() remove()
*/
void QmlListModel::append(const QScriptValue& valuemap)
{
    if (!valuemap.isObject()) {
        qWarning("ListModel::append: value is not an object");
        return;
    }
    if (!_root)
        _root = new ModelNode;
    ModelNode *mn = new ModelNode;
    mn->setObjectValue(valuemap);
    _root->values << qVariantFromValue(mn);
    emit itemsInserted(count()-1,1);
}

/*!
    \qmlmethod ListModel::set(index,dict)

    Changes the item at \a index in the list model to the
    values in \a dict.

    \code
        FruitModel.set(3, {"cost": 5.95, "name":"Pizza"})
    \endcode

    The \a index must be an element in the list.

    \sa append()
*/
void QmlListModel::set(int index, const QScriptValue& valuemap)
{
    if (!_root)
        _root = new ModelNode;
    if ( index >= _root->values.count()) {
        qWarning() << "ListModel::set index out of range:" << index;
        return;
    }
    if (index == _root->values.count())
        append(valuemap);
    else {
        ModelNode *node = qvariant_cast<ModelNode *>(_root->values.at(index));
        QList<int> roles;
        node->setObjectValue(valuemap);
        QScriptValueIterator it(valuemap);
        while (it.hasNext()) {
            it.next();
            int r = roleStrings.indexOf(it.name());
            if (r<0) {
                r = roleStrings.count();
                roleStrings << it.name();
            }
            roles.append(r);
        }
        emit itemsChanged(index,1,roles);
    }
}

/*!
    \qmlmethod ListModel::set(index,property,value)

    Changes the \a property of the item at \a index in the list model to \a value.

    \code
        FruitModel.set(3, "cost", 5.95)
    \endcode

    The \a index must be an element in the list.

    \sa append()
*/
void QmlListModel::set(int index, const QString& property, const QVariant& value)
{
    if (!_root)
        _root = new ModelNode;
    if ( index >= _root->values.count()) {
        qWarning() << "ListModel::set index out of range:" << index;
        return;
    }
    ModelNode *node = qvariant_cast<ModelNode *>(_root->values.at(index));
    int r = roleStrings.indexOf(property);
    if (r<0) {
        r = roleStrings.count();
        roleStrings << property;
    }
    QList<int> roles;
    roles.append(r);

    if (node)
        node->setProperty(property,value);
    emit itemsChanged(index,1,roles);
}


class QmlListModelParser : public QmlCustomParser
{
public:
    QByteArray compile(const QList<QmlCustomParserProperty> &);
    bool compileProperty(const QmlCustomParserProperty &prop, QList<ListInstruction> &instr, QByteArray &data);
    void setCustomData(QObject *, const QByteArray &);
};

bool QmlListModelParser::compileProperty(const QmlCustomParserProperty &prop, QList<ListInstruction> &instr, QByteArray &data)
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
                if (nodeProp.name() == "") {
                    error(nodeProp, QLatin1String("Cannot use default property in ListModel"));
                    return false;
                }
                if (nodeProp.name() == "id") {
                    error(nodeProp, QLatin1String("Cannot use reserved \"id\" property in ListModel"));
                    return false;
                }

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

            QmlParser::Variant variant = 
                qvariant_cast<QmlParser::Variant>(value);

            int ref = data.count();
            QByteArray d = variant.asScript().toUtf8();
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

QByteArray QmlListModelParser::compile(const QList<QmlCustomParserProperty> &customProps)
{
    QList<ListInstruction> instr;
    QByteArray data;

    for(int ii = 0; ii < customProps.count(); ++ii) {
        const QmlCustomParserProperty &prop = customProps.at(ii);
        if(prop.name() != "") { // isn't default property
            error(prop, QLatin1String("Cannot use default property"));
            return QByteArray();
        }

        if(!compileProperty(prop, instr, data)) {
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

void QmlListModelParser::setCustomData(QObject *obj, const QByteArray &d)
{
    QmlListModel *rv = static_cast<QmlListModel *>(obj);

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
                n->properties.insert(QString::fromUtf8(data + instr.dataIdx), n2);
                nodes.push(n2);
            }
            break;
        }
    }
}

QML_DEFINE_CUSTOM_TYPE(Qt, 4,6, (QT_VERSION&0x00ff00)>>8, ListModel, QmlListModel, QmlListModelParser)

/*!
    \qmlclass ListElement
    \brief The ListElement element defines a data item in a ListModel.

    \sa ListModel
*/
// ### FIXME
class QmlListElement : public QObject
{
Q_OBJECT
};
QML_DEFINE_TYPE(Qt,4,6,(QT_VERSION&0x00ff00)>>8,ListElement,QmlListElement)

static void dump(ModelNode *node, int ind)
{
    QByteArray indentBa(ind * 4, ' ');
    const char *indent = indentBa.constData();

    for (int ii = 0; ii < node->values.count(); ++ii) {
        ModelNode *subNode = qvariant_cast<ModelNode *>(node->values.at(ii));
        if (subNode) {
            qWarning().nospace() << indent << "Sub-node " << ii;
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
    if (modelCache) { modelCache->_root = 0/* ==this */; delete modelCache; modelCache = 0; }
    if (objectCache) { delete objectCache; }
}

QT_END_NAMESPACE

QML_DECLARE_TYPE(QmlListElement)

#include "qmllistmodel.moc"
