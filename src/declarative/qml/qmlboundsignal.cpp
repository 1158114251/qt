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

#include "qmlboundsignal_p.h"
#include "private/qmetaobjectbuilder_p.h"
#include "private/qmlengine_p.h"
#include "private/qmlexpression_p.h"
#include "private/qmlcontext_p.h"
#include <qfxglobal.h>
#include <qmlmetatype.h>
#include <qml.h>
#include <qmlcontext.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

class QmlBoundSignalParameters : public QObject
{
Q_OBJECT
public:
    QmlBoundSignalParameters(const QMetaMethod &, QObject * = 0);
    ~QmlBoundSignalParameters();

    void setValues(void **);
    void clearValues();

private:
    friend class MetaObject;
    int metaCall(QMetaObject::Call, int _id, void **);
    struct MetaObject : public QAbstractDynamicMetaObject {
        MetaObject(QmlBoundSignalParameters *b)
            : parent(b) {}

        int metaCall(QMetaObject::Call c, int id, void **a) { 
            return parent->metaCall(c, id, a);
        }
        QmlBoundSignalParameters *parent;
    };

    int *types;
    void **values;
    QMetaObject *myMetaObject;
};

static int evaluateIdx = -1;

QmlAbstractBoundSignal::QmlAbstractBoundSignal(QObject *parent)
: QObject(parent)
{
}

QmlAbstractBoundSignal::~QmlAbstractBoundSignal()
{
}

QmlBoundSignal::QmlBoundSignal(QObject *scope, const QMetaMethod &signal, 
                               QObject *parent)
: m_expression(0), m_idx(signal.methodIndex()), m_params(0)
{
    // A cached evaluation of the QmlExpression::value() slot index.
    //
    // This is thread safe.  Although it may be updated by two threads, they
    // will both set it to the same value - so the worst thing that can happen
    // is that they both do the work to figure it out.  Boo hoo.
    if (evaluateIdx == -1) evaluateIdx = metaObject()->methodCount();

    QFx_setParent_noEvent(this, parent);
    QMetaObject::connect(scope, m_idx, this, evaluateIdx);

    if (!signal.parameterTypes().isEmpty())
        m_params = new QmlBoundSignalParameters(signal, this);
}

QmlBoundSignal::QmlBoundSignal(QmlContext *ctxt, const QString &val, 
                               QObject *scope, const QMetaMethod &signal,
                               QObject *parent)
: m_expression(0), m_idx(signal.methodIndex()), m_params(0)
{
    // A cached evaluation of the QmlExpression::value() slot index.
    //
    // This is thread safe.  Although it may be updated by two threads, they
    // will both set it to the same value - so the worst thing that can happen
    // is that they both do the work to figure it out.  Boo hoo.
    if (evaluateIdx == -1) evaluateIdx = metaObject()->methodCount();

    QFx_setParent_noEvent(this, parent);
    QMetaObject::connect(scope, m_idx, this, evaluateIdx);

    m_expression = new QmlExpression(ctxt, val, scope);
    m_expression->setTrackChange(false);

    if (!signal.parameterTypes().isEmpty())
        m_params = new QmlBoundSignalParameters(signal, this);
}

QmlBoundSignal::~QmlBoundSignal()
{
    delete m_expression;
    m_expression = 0;
}

int QmlBoundSignal::index() const 
{ 
    return m_idx; 
}

/*!
    Returns the signal expression.
*/
QmlExpression *QmlBoundSignal::expression() const
{
    return m_expression;
}

/*!
    Sets the signal expression to \a e.  Returns the current signal expression,
    or null if there is no signal expression.

    The QmlBoundSignal instance takes ownership of \a e.  The caller is 
    assumes ownership of the returned QmlExpression.
*/
QmlExpression *QmlBoundSignal::setExpression(QmlExpression *e)
{
    QmlExpression *rv = m_expression;
    m_expression = e;
    if (m_expression) m_expression->setTrackChange(false);
    return rv;
}

QmlBoundSignal *QmlBoundSignal::cast(QObject *o)
{
    QmlAbstractBoundSignal *s = qobject_cast<QmlAbstractBoundSignal*>(o);
    return static_cast<QmlBoundSignal *>(s);
}

int QmlBoundSignal::qt_metacall(QMetaObject::Call c, int id, void **a)
{
    if (c == QMetaObject::InvokeMetaMethod && id == evaluateIdx) {
        if (m_params) m_params->setValues(a);
        if (m_expression) {
            QmlExpressionPrivate::get(m_expression)->value(m_params);
            if (m_expression->hasError())
                qWarning().nospace() << qPrintable(m_expression->error().toString());
        }
        if (m_params) m_params->clearValues();
        return -1;
    } else {
        return QObject::qt_metacall(c, id, a);
    }
}

QmlBoundSignalParameters::QmlBoundSignalParameters(const QMetaMethod &method, 
                                                   QObject *parent)
: QObject(parent), types(0), values(0)
{
    MetaObject *mo = new MetaObject(this);

    // ### Optimize!
    // ### Ensure only supported types are allowed, otherwise it might crash
    QMetaObjectBuilder mob;
    mob.setSuperClass(&QmlBoundSignalParameters::staticMetaObject);
    mob.setClassName("QmlBoundSignalParameters");

    QList<QByteArray> paramTypes = method.parameterTypes();
    QList<QByteArray> paramNames = method.parameterNames();
    types = new int[paramTypes.count()];
    for (int ii = 0; ii < paramTypes.count(); ++ii) {
        const QByteArray &type = paramTypes.at(ii);
        const QByteArray &name = paramNames.at(ii);

        if (name.isEmpty() || type.isEmpty()) {
            types[ii] = 0;
            continue;
        }

        QVariant::Type t = (QVariant::Type)QMetaType::type(type.constData());
        if (QmlMetaType::isObject(t)) {
            types[ii] = QMetaType::QObjectStar;
            QMetaPropertyBuilder prop = mob.addProperty(name, "QObject*");
            prop.setWritable(false);
        } else {
            types[ii] = t;
            QMetaPropertyBuilder prop = mob.addProperty(name, type);
            prop.setWritable(false);
        }
    }
    myMetaObject = mob.toMetaObject();
    *static_cast<QMetaObject *>(mo) = *myMetaObject;

    d_ptr->metaObject = mo;
}

QmlBoundSignalParameters::~QmlBoundSignalParameters()
{
    delete [] types;
    qFree(myMetaObject);
}

void QmlBoundSignalParameters::setValues(void **v)
{
    values = v;
}

void QmlBoundSignalParameters::clearValues()
{
    values = 0;
}

int QmlBoundSignalParameters::metaCall(QMetaObject::Call c, int id, void **a)
{
    if (!values)
        return -1;

    if (c == QMetaObject::ReadProperty && id >= 1) {
        QmlMetaType::copy(types[id - 1], a[0], values[id]);
        return -1;
    } else {
        return qt_metacall(c, id, a);
    }
}

QT_END_NAMESPACE

#include "qmlboundsignal.moc"
