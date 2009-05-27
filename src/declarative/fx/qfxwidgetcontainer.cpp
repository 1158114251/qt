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

#include "qfxwidgetcontainer.h"
#include <qsimplecanvas.h>
#include <qgraphicswidget.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass WidgetContainer QFxWidgetContainer
    \brief The WidgetContainer element allows you to add QGraphicsWidgets into Fluid UI elements.
*/

/*!
    \internal
    \class QFxWidgetContainer
    \brief The QFxWidgetContainer class allows you to add QGraphicsWidgets into Fluid UI applications.
*/

QML_DEFINE_TYPE(QFxWidgetContainer, WidgetContainer)

QFxWidgetContainer::QFxWidgetContainer(QFxItem *parent)
: QFxItem(parent), _graphicsWidget(0)
{
}

QFxWidgetContainer::~QFxWidgetContainer()
{
}

QGraphicsWidget *QFxWidgetContainer::graphicsWidget() const
{
    return _graphicsWidget;
}

/*!
    \qmlproperty QGraphicsWidget QFxWidgetContainer::graphicsWidget
    The QGraphicsWidget associated with this element.
*/
void QFxWidgetContainer::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (widget == _graphicsWidget)
        return;

    _graphicsWidget = widget;

    QSimpleCanvas *c = canvas();
    if (!c)
        return;

    if (c->canvasMode() != QSimpleCanvas::GraphicsView) {
        qWarning("QFxWidgetContainer: Cannot add a widget to a non-graphicsview canvas. You might need to set QFX_USE_GRAPHICSVIEW=1");
        return;
    }

    QGraphicsItem *item = (QGraphicsItem *)(*this);
    _graphicsWidget->setParentItem(item);
}

void QFxWidgetContainer::canvasChanged()
{
    if (_graphicsWidget) {
        QGraphicsWidget *w = _graphicsWidget;
        _graphicsWidget = 0;
        setGraphicsWidget(w);
    }
}

QT_END_NAMESPACE
