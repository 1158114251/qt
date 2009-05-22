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

#ifndef QFXTEXTEDIT_P_H
#define QFXTEXTEDIT_P_H

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

#include "qfxitem.h"
#include "qfxpainteditem_p.h"
#include "qml.h"


QT_BEGIN_NAMESPACE
class QTextLayout;
class QTextDocument;
class QTextControl;
class QFxTextEditPrivate : public QFxPaintedItemPrivate
{
    Q_DECLARE_PUBLIC(QFxTextEdit)

public:
    QFxTextEditPrivate()
      : font(0), color("black"), imgDirty(true), hAlign(QFxTextEdit::AlignLeft), vAlign(QFxTextEdit::AlignTop),
      dirty(false), wrap(false), richText(false), cursorVisible(false), focusOnPress(false), preserveSelection(true),
      format(QFxTextEdit::AutoText), document(0)
    {
    }

    void init();

    void updateDefaultTextOption();
    void relayoutDocument();

    QString text;
    QmlFont font;
    QColor  color;
    QColor  highlightColor;
    QString style;
    QColor  styleColor;
    bool imgDirty;
#if defined(QFX_RENDER_OPENGL)
    GLTexture texture;
#endif
    QSimpleCanvasConfig::Image imgCache;
    QImage imgStyleCache;
    QFxTextEdit::HAlignment hAlign;
    QFxTextEdit::VAlignment vAlign;
    bool dirty;
    bool wrap;
    bool richText;
    bool cursorVisible;
    bool focusOnPress;
    bool preserveSelection;
    QFxTextEdit::TextFormat format;
    QTextDocument *document;
    QTextControl *control;
};

QT_END_NAMESPACE
#endif
