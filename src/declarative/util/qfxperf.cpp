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

#include "qfxperf.h"


QT_BEGIN_NAMESPACE
Q_DEFINE_PERFORMANCE_LOG(QFxPerf, "QFx") {
    Q_DEFINE_PERFORMANCE_METRIC(QmlParsing, "QML Parsing");
    Q_DEFINE_PERFORMANCE_METRIC(Compile, "QML Compilation");
    Q_DEFINE_PERFORMANCE_METRIC(CompileRun, "QML Compilation Run");
    Q_DEFINE_PERFORMANCE_METRIC(CreateComponent, "Component creation");
    Q_DEFINE_PERFORMANCE_METRIC(BindInit, "BindValue Initialization");
    Q_DEFINE_PERFORMANCE_METRIC(BindCompile, "BindValue compile");
    Q_DEFINE_PERFORMANCE_METRIC(BindValue, "BindValue execution");
    Q_DEFINE_PERFORMANCE_METRIC(BindValueSSE, "BindValue execution SSE");
    Q_DEFINE_PERFORMANCE_METRIC(BindValueQt, "BindValue execution QtScript");
    Q_DEFINE_PERFORMANCE_METRIC(ContextQuery, "QtScript: Query Context");
    Q_DEFINE_PERFORMANCE_METRIC(ContextProperty, "QtScript: Context Property");
    Q_DEFINE_PERFORMANCE_METRIC(ObjectQuery, "QtScript: Query Object");
    Q_DEFINE_PERFORMANCE_METRIC(ObjectProperty, "QtScript: Object Property");
    Q_DEFINE_PERFORMANCE_METRIC(ObjectSetProperty, "QtScript: Set Object Property");
    Q_DEFINE_PERFORMANCE_METRIC(BindableValueUpdate, "QmlBindableValue::update");
    Q_DEFINE_PERFORMANCE_METRIC(PixmapLoad, "Pixmap loading");
    Q_DEFINE_PERFORMANCE_METRIC(MetaProperty, "Meta property resolution");
    Q_DEFINE_PERFORMANCE_METRIC(PathCache, "Path cache");
    Q_DEFINE_PERFORMANCE_METRIC(CreateParticle, "Particle creation");
    Q_DEFINE_PERFORMANCE_METRIC(FontDatabase, "Font database creation");
    Q_DEFINE_PERFORMANCE_METRIC(ItemComponentComplete, "QFxItem::componentComplete");
    Q_DEFINE_PERFORMANCE_METRIC(ImageComponentComplete, "QFxImage::componentComplete");
    Q_DEFINE_PERFORMANCE_METRIC(ComponentInstanceComponentComplete, "QFxComponentInstance::componentComplete");
    Q_DEFINE_PERFORMANCE_METRIC(BaseLayoutComponentComplete, "QFxBaseLayout::componentComplete");
    Q_DEFINE_PERFORMANCE_METRIC(TextComponentComplete, "QFxText::componentComplete");
    Q_DEFINE_PERFORMANCE_METRIC(QFxText_setText, "QFxText::setText");
    Q_DEFINE_PERFORMANCE_METRIC(AddScript, "QmlScript::addScriptToEngine");
}
QT_END_NAMESPACE
