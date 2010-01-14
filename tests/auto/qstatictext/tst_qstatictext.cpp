/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QStaticText>
#include <QtGui/QImage>

#include <private/qstatictext_p.h>

// #define DEBUG_SAVE_IMAGE

class tst_QStaticText: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void constructionAndDestruction();
    void drawToPoint();
    void drawToRect();
    void setFont();
    void setMaximumSize();

    void translatedPainter();
    void rotatedPainter();
    void scaledPainter();
    void rotatedScaledAndTranslatedPainter();
    void transformationChanged();
    void translationDoesntCauseRelayout();
};

void tst_QStaticText::init()
{
}

void tst_QStaticText::cleanup()
{
}

void tst_QStaticText::constructionAndDestruction()
{
    QStaticText text("My text");
}

void tst_QStaticText::drawToPoint()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::drawToRect()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.drawText(QRectF(11, 12, 10, 500), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.", QFont(), QSizeF(10, 500));
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::setFont()
{
    QFont font = QApplication::font();
    font.setBold(true);
    font.setPointSize(28);

    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.setFont(font);
        p.drawText(11, 120, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        QStaticText text;
        text.setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        text.setFont(font);
        p.drawStaticText(11, 120, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::setMaximumSize()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.drawText(QRectF(11, 12, 10, 500), "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        text.setMaximumSize(QSizeF(10, 500));
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::translatedPainter()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.translate(100, 200);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.translate(100, 200);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::rotatedPainter()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.rotate(30.0);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.rotate(30.0);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);
    }

#if defined(DEBUG_SAVE_IMAGE)
    imageDrawText.save("rotatedPainter_imageDrawText.png");
    imageDrawStaticText.save("rotatedPainter_imageDrawStaticText.png");
#endif

    QCOMPARE(imageDrawStaticText, imageDrawText);

}

void tst_QStaticText::scaledPainter()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.scale(2.0, 0.2);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.scale(2.0, 0.2);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::rotatedScaledAndTranslatedPainter()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.rotate(45.0);
        p.scale(0.2, 2.0);
        p.translate(100, 200);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.rotate(45.0);
        p.scale(0.2, 2.0);
        p.translate(100, 200);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::transformationChanged()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.rotate(33.0);
        p.scale(0.5, 0.7);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

        p.rotate(77.0);
        p.scale(0.7, 0.5);
        p.drawText(100, 200, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.rotate(33.0);
        p.scale(0.5, 0.7);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        p.drawStaticText(11, 12, text);

        p.rotate(77.0);
        p.scale(0.7, 0.5);
        p.drawStaticText(100, 200, text);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);
}

void tst_QStaticText::translationDoesntCauseRelayout()
{
    QImage imageDrawText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawText);
        p.rotate(33.0);
        p.scale(0.5, 0.7);

        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

        p.translate(100, 200);
        p.drawText(11, 12, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }

    QImage imageDrawStaticText(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&imageDrawStaticText);
        p.rotate(33.0);
        p.scale(0.5, 0.7);

        QStaticText text("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

        QStaticTextPrivate *textd = QStaticTextPrivate::get(&text);
        glyph_t *glyphPool = textd->glyphPool;
        QFixedPoint *positionPool = textd->positionPool;
        QStaticTextItem *items = textd->items;

        p.drawStaticText(11, 12, text);

        p.translate(100, 200);
        p.drawStaticText(11, 12, text);

        // ###
        // If the layout is recalculated, the data is reallocated. Not a entirely deterministic
        // test, since the memory can be reused.
        QCOMPARE(textd->glyphPool, glyphPool);
        QCOMPARE(textd->positionPool, positionPool);
        QCOMPARE(textd->items, items);
    }

    QCOMPARE(imageDrawStaticText, imageDrawText);

}


QTEST_MAIN(tst_QStaticText)
#include "tst_qstatictext.moc"
