/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qapplication_p.h"
#include "qcolormap.h"
#include "qpixmapcache.h"
#if !defined(QT_NO_GLIB)
#include "private/qeventdispatcher_glib_p.h"
#endif
#include "private/qeventdispatcher_unix_p.h"
#ifndef QT_NO_CURSOR
#include "private/qcursor_p.h"
#endif

#include "private/qwidget_p.h"

#include "qgenericpluginfactory_lite.h"
#include <qdesktopwidget.h>

#include <qinputcontext.h>

#include <qdebug.h>


QT_BEGIN_NAMESPACE

static QString appName;
static const char *appFont = 0;                  // application font

QWidget *qt_button_down = 0;                     // widget got last button-down

static bool app_do_modal = false;
extern QWidgetList *qt_modal_stack;              // stack of modal widgets

int qt_last_x = 0;
int qt_last_y = 0;

QString QApplicationPrivate::appName() const
{
    return QT_PREPEND_NAMESPACE(appName);
}

void QApplicationPrivate::createEventDispatcher()
{
    Q_Q(QApplication);
#if !defined(QT_NO_GLIB)
    if (qgetenv("QT_NO_GLIB").isEmpty() && QEventDispatcherGlib::versionSupported())
        eventDispatcher = new QEventDispatcherGlib(q);
    else
#endif
    eventDispatcher = new QEventDispatcherUNIX(q);
}

static bool qt_try_modal(QWidget *widget, const QEvent *event)
{
    QWidget * top = 0;

    if (QApplicationPrivate::tryModalHelper(widget, &top))
        return true;

    bool block_event  = false;
    bool paint_event = false;

    switch (event->type()) {
#if 0
    case QEvent::Focus:
        if (!static_cast<QWSFocusEvent*>(event)->simpleData.get_focus)
            break;
        // drop through
#endif
    case QEvent::MouseButtonPress:                        // disallow mouse/key events
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        block_event         = true;
        break;
    default:
        break;
    }

    if ((block_event || paint_event) && top->parentWidget() == 0)
        top->raise();

    return !block_event;
}



void QApplicationPrivate::enterModal_sys(QWidget *widget)
{
    if (!qt_modal_stack)
        qt_modal_stack = new QWidgetList;
    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
}

void QApplicationPrivate::leaveModal_sys(QWidget *widget )
{
    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
        }
    }
    app_do_modal = qt_modal_stack != 0;
}

bool QApplicationPrivate::modalState()
{
    return app_do_modal;
}

void QApplicationPrivate::closePopup(QWidget *popup)
{
    Q_Q(QApplication);
    if (!popupWidgets)
        return;
    popupWidgets->removeAll(popup);

//###
//     if (popup == qt_popup_down) {
//         qt_button_down = 0;
//         qt_popup_down = 0;
//     }

    if (QApplicationPrivate::popupWidgets->count() == 0) {                // this was the last popup
        delete QApplicationPrivate::popupWidgets;
        QApplicationPrivate::popupWidgets = 0;

        //### replay mouse event?

        //### transfer/release mouse grab

        //### transfer/release keyboard grab

        //give back focus

        if (active_window) {
            if (QWidget *fw = active_window->focusWidget()) {
                if (fw != QApplication::focusWidget()) {
                    fw->setFocus(Qt::PopupFocusReason);
                } else {
                    QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
                    q->sendEvent(fw, &e);
                }
            }
        }

    } else {
        // A popup was closed, so the previous popup gets the focus.

        QWidget* aw = QApplicationPrivate::popupWidgets->last();
        if (QWidget *fw = aw->focusWidget())
            fw->setFocus(Qt::PopupFocusReason);

        //### regrab the keyboard and mouse in case 'popup' lost the grab


    }

}

static int openPopupCount = 0;
void QApplicationPrivate::openPopup(QWidget *popup)
{
    openPopupCount++;
    if (!popupWidgets) {                        // create list
        popupWidgets = new QWidgetList;

        /* only grab if you are the first/parent popup */
        //####   ->grabMouse(popup,true);
        //####   ->grabKeyboard(popup,true);
        //### popupGrabOk = true;
    }
    popupWidgets->append(popup);                // add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if (popup->focusWidget()) {
        popup->focusWidget()->setFocus(Qt::PopupFocusReason);
    } else if (popupWidgets->count() == 1) { // this was the first popup
        if (QWidget *fw = QApplication::focusWidget()) {
            QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
            QApplication::sendEvent(fw, &e);
        }
    }
}

void QApplicationPrivate::initializeMultitouch_sys()
{
}

void QApplicationPrivate::cleanupMultitouch_sys()
{
}

void QApplicationPrivate::initializeWidgetPaletteHash()
{
}

void QApplication::setCursorFlashTime(int msecs)
{
    QApplicationPrivate::cursor_flash_time = msecs;
}

int QApplication::cursorFlashTime()
{
    return QApplicationPrivate::cursor_flash_time;
}

void QApplication::setDoubleClickInterval(int ms)
{
    QApplicationPrivate::mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return QApplicationPrivate::mouse_double_click_time;
}

void QApplication::setKeyboardInputInterval(int ms)
{
    QApplicationPrivate::keyboard_input_time = ms;
}

int QApplication::keyboardInputInterval()
{
    return QApplicationPrivate::keyboard_input_time;
}

#ifndef QT_NO_WHEELEVENT
void QApplication::setWheelScrollLines(int lines)
{
    QApplicationPrivate::wheel_scroll_lines = lines;
}

int QApplication::wheelScrollLines()
{
    return QApplicationPrivate::wheel_scroll_lines;
}
#endif

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    switch (effect) {
    case Qt::UI_AnimateMenu:
        QApplicationPrivate::animate_menu = enable;
        break;
    case Qt::UI_FadeMenu:
        if (enable)
            QApplicationPrivate::animate_menu = true;
        QApplicationPrivate::fade_menu = enable;
        break;
    case Qt::UI_AnimateCombo:
        QApplicationPrivate::animate_combo = enable;
        break;
    case Qt::UI_AnimateTooltip:
        QApplicationPrivate::animate_tooltip = enable;
        break;
    case Qt::UI_FadeTooltip:
        if (enable)
            QApplicationPrivate::animate_tooltip = true;
        QApplicationPrivate::fade_tooltip = enable;
        break;
    case Qt::UI_AnimateToolBox:
        QApplicationPrivate::animate_toolbox = enable;
        break;
    default:
        QApplicationPrivate::animate_ui = enable;
        break;
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if (QColormap::instance().depth() < 16 || !QApplicationPrivate::animate_ui)
        return false;

    switch(effect) {
    case Qt::UI_AnimateMenu:
        return QApplicationPrivate::animate_menu;
    case Qt::UI_FadeMenu:
        return QApplicationPrivate::fade_menu;
    case Qt::UI_AnimateCombo:
        return QApplicationPrivate::animate_combo;
    case Qt::UI_AnimateTooltip:
        return QApplicationPrivate::animate_tooltip;
    case Qt::UI_FadeTooltip:
        return QApplicationPrivate::fade_tooltip;
    case Qt::UI_AnimateToolBox:
        return QApplicationPrivate::animate_toolbox;
    default:
        return QApplicationPrivate::animate_ui;
    }
}

#ifndef QT_NO_CURSOR
void QApplication::setOverrideCursor(const QCursor &)
{
    // XXX
}

void QApplication::restoreOverrideCursor()
{
    // XXX
}

#endif// QT_NO_CURSOR

QWidget *QApplication::topLevelAt(const QPoint &pos)
{
//### We have to implement a windowsystem-aware way to do this

    //fallback implementation assuming widgets are in stacking order

    QWidgetList list = topLevelWidgets();
    for (int i = list.size()-1; i >= 0; --i) {
        QWidget *w = list[i];
        //### mask is ignored
        if (w != QApplication::desktop() && w->isVisible() && w->geometry().contains(pos))
            return w;
    }

    return 0;
}

void QApplication::beep()
{
}

void QApplication::alert(QWidget *, int)
{
}

static void init_plugins(const QList<QByteArray> pluginList)
{
    for (int i = 0; i < pluginList.count(); ++i) {
        QByteArray pluginSpec = pluginList.at(i);
        qDebug() << "init_plugins" << i << pluginSpec;
        int colonPos = pluginSpec.indexOf(':');
        QObject *plugin;
        if (colonPos < 0)
            plugin = QGenericPluginFactory::create(QLatin1String(pluginSpec), QString());
        else
            plugin = QGenericPluginFactory::create(QLatin1String(pluginSpec.mid(0, colonPos)),
                                                   QLatin1String(pluginSpec.mid(colonPos+1)));
        qDebug() << "	created" << plugin;
    }
}

class QDummyInputContext : public QInputContext
{
public:
    explicit QDummyInputContext(QObject* parent = 0) : QInputContext(parent) {}
    ~QDummyInputContext() {}
    QString identifierName() { return QString(); }
    QString language() { return QString(); }

    void reset() {}
    bool isComposing() const { return false; }

};

void qt_init(QApplicationPrivate *priv, int type)
{
    Q_UNUSED(type);

    char *p;
    char **argv = priv->argv;
    int argc = priv->argc;

    if (argv && *argv) { //apparently, we allow people to pass 0 on the other platforms
        p = strrchr(argv[0], '/');
        appName = QString::fromLocal8Bit(p ? p + 1 : argv[0]);
    }

    QList<QByteArray> pluginList;

    // Get command line params

    int j = argc ? 1 : 0;
    for (int i=1; i<argc; i++) {
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        QByteArray arg = argv[i];
        if (arg == "-fn" || arg == "-font") {
            if (++i < argc)
                appFont = argv[i];
        } else if (arg == "-plugin") {
            if (++i < argc)
                pluginList << argv[i];
        } else {
            argv[j++] = argv[i];
        }
    }

    if (j < priv->argc) {
        priv->argv[j] = 0;
        priv->argc = j;
    }




#if 0
    QByteArray pluginEnv = qgetenv("QT_LITE_PLUGINS");
    if (!pluginEnv.isEmpty()) {
        pluginList.append(pluginEnv.split(';'));
    }
#endif


    qDebug() << pluginList;

    init_plugins(pluginList);

    QColormap::initialize();
    QFont::initialize();
#ifndef QT_NO_CURSOR
//    QCursorData::initialize();
#endif

    qApp->setObjectName(appName);

#ifndef QT_NO_QWS_INPUTMETHODS
        qApp->setInputContext(new QDummyInputContext(qApp));
#endif
}

void qt_cleanup()
{
    QPixmapCache::clear();
#ifndef QT_NO_CURSOR
    QCursorData::cleanup();
#endif
    QFont::cleanup();
    QColormap::cleanup();
    delete QApplicationPrivate::inputContext;
    QApplicationPrivate::inputContext = 0;

    QApplicationPrivate::active_window = 0; //### this should not be necessary
}


#ifdef QT3_SUPPORT
void QApplication::setMainWidget(QWidget *mainWidget)
{
    QApplicationPrivate::main_widget = mainWidget;
    if (QApplicationPrivate::main_widget && windowIcon().isNull()
        && QApplicationPrivate::main_widget->testAttribute(Qt::WA_SetWindowIcon))
        setWindowIcon(QApplicationPrivate::main_widget->windowIcon());
}
#endif


//------------------------------------------------------------
//
// Callback functions for plugins:
//

/*!

\a tlw == 0 means that \a ev is in global coords only


*/

void QApplicationPrivate::handleMouseEvent(QWidget *tlw, const QMouseEvent &ev)
{
//    qDebug() << "handleMouseEvent" << tlw << ev.pos() << hex << ev.buttons();

    static QWidget *implicit_mouse_grabber=0;

    QPoint localPoint = ev.pos();
    QPoint globalPoint = ev.globalPos();
    bool trustLocalPoint = !!tlw; //is there something the local point can be local to?
    QWidget *mouseWidget = tlw;

    qt_last_x = globalPoint.x();
    qt_last_y = globalPoint.y();

    if (self->inPopupMode()) {
        //popup mouse handling is magical...
        mouseWidget = qApp->activePopupWidget();
        trustLocalPoint = (mouseWidget == tlw);

        //### how should popup mode and implicit mouse grab interact?

    } else if (tlw && app_do_modal && !qt_try_modal(tlw, &ev) ) {
        //even if we're blocked by modality, we should deliver the mouse release event..
        //### this code is not completely correct: multiple buttons can be pressed simultaneously
        if (!(implicit_mouse_grabber && ev.buttons() == Qt::NoButton)) {
            //qDebug() << "modal blocked mouse event to" << tlw;
            return;
        }
    } else {
        QWidget *mouseWindow = tlw;

        // find the tlw if we didn't get it from the plugin
        if (!mouseWindow) {
            mouseWindow = QApplication::topLevelAt(globalPoint);
        }

        if (!mouseWindow && !implicit_mouse_grabber)
            return; //nowhere to send it

        // which child should have it?
        mouseWidget = mouseWindow;
        if (mouseWindow) {
            QWidget *w =  mouseWindow->childAt(ev.pos());
            if (w) {
                mouseWidget = w;
            }
        }

        //handle implicit mouse grab
        if (ev.type() == QEvent::MouseButtonPress && !implicit_mouse_grabber) {
            implicit_mouse_grabber = mouseWidget;

            Q_ASSERT(mouseWindow);
            mouseWindow->activateWindow(); //focus
        } else if (implicit_mouse_grabber) {
            mouseWidget = implicit_mouse_grabber;
            mouseWindow = mouseWidget->window();
            trustLocalPoint = (mouseWindow == tlw);
        }
    }
    Q_ASSERT(mouseWidget);

    if (trustLocalPoint) {
        // we have a sensible localPoint, so we prefer that, since the
        // window system may know more than we do
        localPoint = mouseWidget->mapFrom(tlw, localPoint);
    } else {
        // we don't want to map a local point from a different toplevel
        // and we definitely don't want to map from the null pointer
        localPoint = mouseWidget->mapFromGlobal(globalPoint);
    }

    if (ev.buttons() == Qt::NoButton) {
        //qDebug() << "resetting mouse grabber";
        implicit_mouse_grabber = 0;
    }

    // Remember, we might enter a modal event loop when sending the event,
    // so think carefully before adding code below this point.

    // qDebug() << "sending mouse ev." << ev.type() << localPoint << globalPoint << ev.button() << ev.buttons() << mouseWidget << "mouse grabber" << implicit_mouse_grabber;

    QMouseEvent e(ev.type(), localPoint, globalPoint, ev.button(), ev.buttons(), ev.modifiers());
    QApplication::sendSpontaneousEvent(mouseWidget, &e);

}


//### there's a lot of duplicated logic here -- refactoring required!

void QApplicationPrivate::handleWheelEvent(QWidget *tlw, QWheelEvent &ev)
{
//    QPoint localPoint = ev.pos();
    QPoint globalPoint = ev.globalPos();
//    bool trustLocalPoint = !!tlw; //is there something the local point can be local to?
    QWidget *mouseWidget = tlw;

    qt_last_x = globalPoint.x();
    qt_last_y = globalPoint.y();

     QWidget *mouseWindow = tlw;

     // find the tlw if we didn't get it from the plugin
     if (!mouseWindow) {
         mouseWindow = QApplication::topLevelAt(globalPoint);
     }

     if (!mouseWindow)
         return;

     if (app_do_modal && !qt_try_modal(mouseWindow, &ev) ) {
         qDebug() << "modal blocked wheel event" << mouseWindow;
         return;
     }
     QPoint p = mouseWindow->mapFromGlobal(globalPoint);
     QWidget *w = mouseWindow->childAt(p);
     if (w) {
         mouseWidget = w;
         p = mouseWidget->mapFromGlobal(globalPoint);
     }

     QWheelEvent e(p, globalPoint, ev.delta(), ev.buttons(), ev.modifiers(),
                   ev.orientation());
     QApplication::sendSpontaneousEvent(mouseWidget, &e);
}



// Remember, Qt convention is:  keyboard state is state *before*

void QApplicationPrivate::handleKeyEvent(QWidget *tlw, QKeyEvent *e)
{
    QWidget *focusW = 0;
    if (self->inPopupMode()) {
        QWidget *popupW = qApp->activePopupWidget();
        focusW = popupW->focusWidget() ? popupW->focusWidget() : popupW;
    }
    if (!focusW)
        focusW = QApplication::focusWidget();
    if (!focusW)
        focusW = tlw;
    if (!focusW)
        focusW = QApplication::activeWindow();

    //qDebug() << "handleKeyEvent" << hex << e->key() << e->modifiers() << e->text() << "widget" << focusW;

    if (!focusW)
        return;
    if (app_do_modal && !qt_try_modal(focusW, e))
        return;

    QApplication::sendSpontaneousEvent(focusW, e);
}


void QApplicationPrivate::handleGeometryChange(QWidget *tlw, const QRect &newRect)
{
    QRect cr(tlw->geometry());

    bool isResize = cr.size() != newRect.size();
    bool isMove = cr.topLeft() != newRect.topLeft();
    tlw->data->crect = newRect;
    if (isResize) {
        QResizeEvent e(tlw->data->crect.size(), cr.size());
        QApplication::sendSpontaneousEvent(tlw, &e);
    }

    if (isMove) {
        //### frame geometry
        QMoveEvent e(tlw->data->crect.topLeft(), cr.topLeft());
        QApplication::sendSpontaneousEvent(tlw, &e);
    }
}


void QApplicationPrivate::handleCloseEvent(QWidget *tlw)
{
    tlw->d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
}


QT_END_NAMESPACE
