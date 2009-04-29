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

#include "qfxcontentwrapper.h"
#include "qfxcontentwrapper_p.h"


QT_BEGIN_NAMESPACE
QML_DEFINE_TYPE(QFxContentWrapper,ContentWrapper);

QFxContentWrapper::QFxContentWrapper(QFxItem *parent)
: QFxItem(*(new QFxContentWrapperPrivate), parent)
{
}

QFxContentWrapper::QFxContentWrapper(QFxContentWrapperPrivate &dd, QFxItem *parent)
  : QFxItem(dd, parent)
{
}

QList<QFxItem *> *QFxContentWrapper::content()
{
    Q_D(QFxContentWrapper);
    return &(d->_content);
}

void QFxContentWrapper::componentComplete()
{
    QFxItem::componentComplete();
    if (content()->size() < 1)
        return;

    QList<QSimpleCanvasItem *> nodes;
    nodes.append(this);
    QFxItem *target = findContent(nodes);
    if (!target)
        return;
    target = target->itemParent();

    QList<QFxItem*> myContent(*content());
    for (int ii = 0; ii < myContent.count();  ++ii) 
        myContent.at(ii)->setParent(target);
}

QFxItem *QFxContentWrapper::findContent(QList<QSimpleCanvasItem *> &nodes)
{
    QSimpleCanvasItem *item = nodes.takeFirst();
    if (qobject_cast<QFxContent*>(item))
        return static_cast<QFxItem *>(item);
    nodes << item->children();
    if (nodes.isEmpty())
        return 0;
    return findContent(nodes);
}

QML_DEFINE_TYPE(QFxContent,Content);

/*!
    \qmlclass Content QFxContent
    \ingroup group_utility
    \brief Content is used as a placeholder for the content of a component.
    \inherits Item

    In some cases the content of a component is not defined by the component itself.
    For example, the items placed in a group box need to be specified external to
    the where the group box component itself is defined.
    In cases like these Content can be used to specify at what location in the component
    the content should be placed. It is used in conjuntion with the content property of 
    the component instance: any items listed as content will be placed in the location 
    specified by Content.

    Example:
    \code
    <!--GroupBox component definition-->
    <Rect width="{parent.width}" color="white" pen.width="2" pen.color="#adaeb0" radius="10" clip="false" height="{contents.height}">
        <VerticalLayout id="layout" width="{parent.width}">
            <Content/>
        </VerticalLayout>
    </Rect>

    <!--component use-->
    <GroupBox>
        <content>
            <Text text="First Item"/>
            ...
        </content>
    </GroupBox>
    \endcode
*/

QT_END_NAMESPACE
