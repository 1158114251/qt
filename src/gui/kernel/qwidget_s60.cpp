/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_EMBEDDED_LICENSE$
**
****************************************************************************/

#include "qwidget_p.h"
#include "qdesktopwidget.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "private/qbackingstore_p.h"
#include "qevent.h"
#include "qt_s60_p.h"

#include "qbitmap.h"
#include "private/qwindowsurface_s60_p.h"

#include <qinputcontext.h>

QT_BEGIN_NAMESPACE

extern bool qt_nograb();

QWidget *QWidgetPrivate::mouseGrabber = 0;
QWidget *QWidgetPrivate::keyboardGrabber = 0;

void QWidgetPrivate::setWSGeometry(bool dontShow)
{

}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    if ((q->windowType() == Qt::Desktop))
        return;
    if (extra) {                                // any size restrictions?
        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);
    }

    if (q->isWindow())
        topData()->normalGeometry = QRect(0, 0, -1, -1);
    else {
        uint s = data.window_state;
        s &= ~(Qt::WindowMaximized | Qt::WindowFullScreen);
        data.window_state = s;
    }

    QPoint oldPos(q->pos());
    QSize oldSize(q->size());
    QRect oldGeom(data.crect);

    bool isResize = w != oldSize.width() || h != oldSize.height();
    if (!isMove && !isResize)
        return;

    if (isResize)
        data.window_state &= ~Qt::WindowMaximized;

    if(q->isWindow()) {
        if (w == 0 || h == 0) {
            q->setAttribute(Qt::WA_OutsideWSRange, true);
            if (q->isVisible() && q->testAttribute(Qt::WA_Mapped))
                hide_sys();
            data.crect = QRect(x, y, w, h);
            data.window_state &= ~Qt::WindowFullScreen;
        } else if (q->isVisible() && q->testAttribute(Qt::WA_OutsideWSRange)) {
            q->setAttribute(Qt::WA_OutsideWSRange, false);

            // put the window in its place and show it
            q->internalWinId()->SetRect(TRect(TPoint(x, y), TSize(w, h)));
            data.crect.setRect(x, y, w, h);

            show_sys();
        } else {
            QRect r = QRect(x, y, w, h);
            data.crect = r;
            q->internalWinId()->SetRect(TRect(TPoint(x, y), TSize(w, h)));
            topData()->normalGeometry = data.crect;
        }
    } else {
        data.crect.setRect(x, y, w, h);

        QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
        const bool inTopLevelResize = tlwExtra ? tlwExtra->inTopLevelResize : false;

        if (q->isVisible() && (!inTopLevelResize || q->internalWinId())) {
            // Top-level resize optimization does not work for native child widgets;
            // disable it for this particular widget.
            if (inTopLevelResize)
                tlwExtra->inTopLevelResize = false;
            if (!isResize && maybeBackingStore())
                moveRect(QRect(oldPos, oldSize), x - oldPos.x(), y - oldPos.y());
            else
                invalidateBuffer_resizeHelper(oldPos, oldSize);

            if (inTopLevelResize)
                tlwExtra->inTopLevelResize = true;
        }
        if (q->testAttribute(Qt::WA_WState_Created))
            setWSGeometry();
    }

    if (q->isVisible()) {
        if (isMove && q->pos() != oldPos) {
            QMoveEvent e(q->pos(), oldPos);
            QApplication::sendEvent(q, &e);
        }
        if (isResize) {
            bool slowResize = qgetenv("QT_SLOW_TOPLEVEL_RESIZE").toInt();
            const bool setTopLevelResize = !slowResize && q->isWindow() && extra && extra->topextra
                                           && !extra->topextra->inTopLevelResize;
            if (setTopLevelResize)
                extra->topextra->inTopLevelResize = true;
            QResizeEvent e(q->size(), oldSize);
            QApplication::sendEvent(q, &e);
            if (!q->testAttribute(Qt::WA_StaticContents) && q->internalWinId())
                q->internalWinId()->DrawDeferred();
            if (setTopLevelResize)
                extra->topextra->inTopLevelResize = false;
        }
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);

    Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);
    bool tool = (type == Qt::Tool || type == Qt::Drawer);

    WId id = 0;

    if (popup)
        flags |= Qt::WindowStaysOnTopHint; // a popup stays on top

    // always initialize
    if (!window)
        initializeWindow = true;

    TRect clientRect = static_cast<CEikAppUi*>(S60->appUi())->ClientRect();
    int sw = clientRect.Width();
    int sh = clientRect.Height();

    if (desktop) {
        TSize screenSize = S60->screenDevice()->SizeInPixels();
        data.crect.setRect(0, 0, screenSize.iWidth, screenSize.iHeight);
        q->setAttribute(Qt::WA_DontShowOnScreen);
    } else if(topLevel && !q->testAttribute(Qt::WA_Resized)){
        int width = sw;
        int height = sh;
        if (extra) {
            width = qMax(qMin(width, extra->maxw), extra->minw);
            height = qMax(qMin(height, extra->maxh), extra->minh);
        }
        data.crect.setSize(QSize(width, height));
    }

    CCoeControl *destroyw = 0;

    if(window) {
        if (destroyOldWindow)
            destroyw = data.winid;
        id = window;
        setWinId(window);
        TRect tr = window->Rect();
        data.crect.setRect(tr.iTl.iX, tr.iTl.iY, tr.Width(), tr.Height());

    } else if (topLevel) {
        if (!q->testAttribute(Qt::WA_Moved) && !q->testAttribute(Qt::WA_DontShowOnScreen))
            data.crect.moveTopLeft(QPoint(clientRect.iTl.iX, clientRect.iTl.iY));
        QSymbianControl *control= new QSymbianControl(q);
        control->ConstructL(true,desktop);
        if (!desktop) {
            QTLWExtra *topExtra = topData();
            topExtra->rwindow = control->DrawableWindow();
            // Request mouse move events.
            topExtra->rwindow->PointerFilter(EPointerFilterEnterExit
                | EPointerFilterMove | EPointerFilterDrag, 0);
            topExtra->rwindow->EnableVisibilityChangeEvents();

            if (!isOpaque) {
                RWindow *rwindow = static_cast<RWindow*>(topExtra->rwindow);
                TDisplayMode gotDM = (TDisplayMode)rwindow->SetRequiredDisplayMode(EColor16MA);
                if (rwindow->SetTransparencyAlphaChannel() == KErrNone)
                    rwindow->SetBackgroundColor(TRgb(255, 255, 255, 0));
            }
        }


        id = (WId)control;

        setWinId(id);

        q->setAttribute(Qt::WA_WState_Created);

        int x, y, w, h;
        data.crect.getRect(&x, &y, &w, &h);
        control->SetRect(TRect(TPoint(x, y), TSize(w, h)));
    } else if (q->testAttribute(Qt::WA_NativeWindow) || paintOnScreen()) { // create native child widget
        QSymbianControl *control = new QSymbianControl(q);
        control->ConstructL(!parentWidget);
        setWinId(control);
        WId parentw = parentWidget->effectiveWinId();
        control->SetContainerWindowL(*parentw);

        q->setAttribute(Qt::WA_WState_Created);
        int x, y, w, h;
        data.crect.getRect(&x, &y, &w, &h);
        control->SetRect(TRect(TPoint(x, y), TSize(w, h)));
    }

    if (destroyw) {
        destroyw->ControlEnv()->AppUi()->RemoveFromStack(destroyw);
        CBase::Delete(destroyw);
    }
}


void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;

    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    q->setAttribute(Qt::WA_Mapped);

    if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
        invalidateBuffer(q->rect());
        return;
    }

    if (q->isWindow() && q->internalWinId()) {

        WId id = q->internalWinId();
        if (!extra->topextra->activated) {
            id->ActivateL();
            extra->topextra->activated = 1;
        }
        TInt stackingFlags;
        if ((q->windowType() & Qt::Popup) == Qt::Popup) {
            stackingFlags = ECoeStackFlagRefusesAllKeys | ECoeStackFlagRefusesFocus;
        } else {
            stackingFlags = ECoeStackFlagStandard;
        }
        id->ControlEnv()->AppUi()->AddToStackL(id, ECoeStackPriorityDefault, stackingFlags);
        id->MakeVisible(true);

        // Force setting of the icon after window is made visible,
        // this is needed even WA_SetWindowIcon is not set, as in that case we need
        // to reset to the application level window icon
        setWindowIcon_sys(true);
    }

    invalidateBuffer(q->rect());
}

void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
    deactivateWidgetCleanup();
    WId id = q->internalWinId();
    if (q->isWindow() && id) {
        id->SetFocus(false);
        id->MakeVisible(false);
        id->ControlEnv()->AppUi()->RemoveFromStack(id);
        if (QWidgetBackingStore *bs = maybeBackingStore())
            bs->releaseBuffer();
    } else {
        invalidateBuffer(q->rect());
    }

    q->setAttribute(Qt::WA_Mapped, false);
}

void QWidgetPrivate::setFocus_sys()
{
    Q_Q(QWidget);
    if (q->testAttribute(Qt::WA_WState_Created) && q->window()->windowType() != Qt::Popup)
        q->effectiveWinId()->SetFocus(true);
}

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
    QTLWExtra *tlwExtra = maybeTopData();
    if (q->internalWinId() && tlwExtra) {
        tlwExtra->rwindow->SetOrdinalPosition(0);
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
    QTLWExtra *tlwExtra = maybeTopData();
    if (q->internalWinId() && tlwExtra) {
        tlwExtra->rwindow->SetOrdinalPosition(-1);
    }
    if(!q->isWindow())
        invalidateBuffer(q->rect());
}

void QWidgetPrivate::setModal_sys()
{

}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    Q_Q(QWidget);
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
    QTLWExtra *tlwExtra = maybeTopData();
    QTLWExtra *tlwExtraSibling = w->d_func()->maybeTopData();
    if (q->internalWinId() && tlwExtra && w->internalWinId() && tlwExtraSibling)
        tlwExtra->rwindow->SetOrdinalPosition(tlwExtraSibling->rwindow->OrdinalPosition() + 1);
    if(!q->isWindow() || !w->internalWinId())
        invalidateBuffer(q->rect());
}

void QWidgetPrivate::reparentChildren()
{
    Q_Q(QWidget);
    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (!w->testAttribute(Qt::WA_WState_Created))
                continue;
            if (!w->isWindow()) {
                w->d_func()->invalidateBuffer(w->rect());
                WId parent = q->effectiveWinId();
                WId child = w->effectiveWinId();
                if (parent != child)
                    child->SetParent(parent);
                // ### TODO: We probably also need to update the component array here
                w->d_func()->reparentChildren();
            } else {
                bool showIt = w->isVisible();
                QPoint old_pos = w->pos();
                w->setParent(q, w->windowFlags());
                w->move(old_pos);
                if (showIt)
                    w->show();
            }
        }
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WindowFlags f)
{
    Q_Q(QWidget);
    bool wasCreated = q->testAttribute(Qt::WA_WState_Created);

    if (q->isVisible() && q->parentWidget() && parent != q->parentWidget())
        q->parentWidget()->d_func()->invalidateBuffer(q->geometry());

    if (q->testAttribute(Qt::WA_DropSiteRegistered))
        q->setAttribute(Qt::WA_DropSiteRegistered, false);

    WId old_winid = wasCreated ? data.winid : 0;
    if ((q->windowType() == Qt::Desktop))
        old_winid = 0;
    setWinId(0);

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    if (wasCreated && old_winid) {
        old_winid->MakeVisible(false);
        old_winid->ControlEnv()->AppUi()->RemoveFromStack(old_winid);
        old_winid->SetParent(0);
    }

    QObjectPrivate::setParent_helper(parent);
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    data.fstrut_dirty = true;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    adjustFlags(data.window_flags, q);
    // keep compatibility with previous versions, we need to preserve the created state
    // (but we recreate the winId for the widget being reparented, again for compatibility)
    if (wasCreated || (!q->isWindow() && parent->testAttribute(Qt::WA_WState_Created)))
        createWinId();
    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (wasCreated)
        reparentChildren();

    if (old_winid) {
        CBase::Delete(old_winid);
    }

    if (q->testAttribute(Qt::WA_AcceptDrops)
        || (!q->isWindow() && q->parentWidget() && q->parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
        q->setAttribute(Qt::WA_DropSiteRegistered, true);

    invalidateBuffer(q->rect());
}

void QWidgetPrivate::setConstraints_sys()
{

}


void QWidgetPrivate::s60UpdateIsOpaque()
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created) || !q->testAttribute(Qt::WA_TranslucentBackground))
        return;

    if ((data.window_flags & Qt::FramelessWindowHint) == 0)
        return;

    if (!isOpaque) {
        QTLWExtra *topExtra = topData();
        RWindow *rwindow = static_cast<RWindow*>(topExtra->rwindow);
        TDisplayMode gotDM = (TDisplayMode)rwindow->SetRequiredDisplayMode(EColor16MA);
        if (rwindow->SetTransparencyAlphaChannel() == KErrNone)
            rwindow->SetBackgroundColor(TRgb(255, 255, 255, 0));
    } else {
        QTLWExtra *topExtra = topData();
        RWindow *rwindow = static_cast<RWindow*>(topExtra->rwindow);
        rwindow->SetTransparentRegion(TRegionFix<1>());
    }
}

CFbsBitmap* qt_pixmapToNativeBitmapL(QPixmap pixmap, bool invert)
{
    CFbsBitmap* fbsBitmap = new(ELeave)CFbsBitmap;
    TSize size(pixmap.size().width(), pixmap.size().height());
    TDisplayMode mode(EColor16MU);

    bool isNull = pixmap.isNull();
    int depth = pixmap.depth();

    // TODO: dummy assumptions from bit amounts for each color
    // Will fix later on when native pixmap is implemented
    switch(pixmap.depth()) {
    case 1:
		mode = EGray2;
        break;
    case 4:
        mode = EColor16;
        break;
    case 8:
        mode = EColor256;
        break;
    case 12:
        mode = EColor4K;
        break;
    case 16:
        mode = EColor64K;
        break;
    case 24:
        mode = EColor16M;
        break;
    case 32:
        case EColor16MU:
        break;
    default:
        qFatal("Unsupported pixmap depth");
        break;
    }

    User::LeaveIfError(fbsBitmap->Create(size, mode));
    fbsBitmap->LockHeap();
    QImage image = pixmap.toImage();

    if(invert)
    	image.invertPixels();

    int height = pixmap.size().height();
    for(int i=0;i<height;i++ )
        {
        TPtr8 scanline(image.scanLine(i), image.bytesPerLine(), image.bytesPerLine());
        fbsBitmap->SetScanLine( scanline, i );
        }

    fbsBitmap->UnlockHeap();
    return fbsBitmap;
}

void QWidgetPrivate::setWindowIcon_sys(bool forceReset)
{
    Q_Q(QWidget);

    if (!q->testAttribute(Qt::WA_WState_Created) || !q->isWindow() )
        return;

    QTLWExtra* topData = this->topData();
    if (topData->iconPixmap && !forceReset)
        // already been set
        return;

    TRect cPaneRect;
    TBool found = AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EContextPane, cPaneRect );
    CAknContextPane* contextPane = S60->contextPane();
    if( found && contextPane ) { // We have context pane with valid metrics
		QIcon icon = q->windowIcon();
		if (!icon.isNull()) {
			// Valid icon -> set it as an context pane picture
			QSize size = icon.actualSize(QSize(cPaneRect.Size().iWidth, cPaneRect.Size().iHeight));
			QPixmap pm = icon.pixmap(size);
			QBitmap mask = pm.mask();
			if (mask.isNull()) {
				mask = QBitmap(pm.size());
				mask.fill(Qt::color1);
			}

			// Convert to CFbsBitmp
			// TODO: When QPixmap is adapted to use native CFbsBitmap,
			// it could be set directly to context pane
			CFbsBitmap* nBitmap = qt_pixmapToNativeBitmapL(pm, false);
			CFbsBitmap* nMask = qt_pixmapToNativeBitmapL(mask, true);

			contextPane->SetPicture(nBitmap,nMask);
		}  else {
			// Icon set to null -> set context pane picture to default
			contextPane->SetPictureToDefaultL();
		}
    }
}

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    if(q->isWindow()) {
        Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
        CAknTitlePane* titlePane = S60->titlePane();
        if(titlePane)
            titlePane->SetTextL(qt_QString2TPtrC(caption));
    }
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{

}

void QWidgetPrivate::scroll_sys(int dx, int dy)
{
    Q_Q(QWidget);

    scrollChildren(dx, dy);
    if (!paintOnScreen() || !q->internalWinId() || !q->internalWinId()->OwnsWindow()) {
        scrollRect(q->rect(), dx, dy);
    } else {
        Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
        RDrawableWindow* rw = topData()->rwindow;
        rw->Scroll(TPoint(dx, dy));
    }
}

void QWidgetPrivate::scroll_sys(int dx, int dy, const QRect &r)
{
    Q_Q(QWidget);

    if (!paintOnScreen() || !q->internalWinId() || !q->internalWinId()->OwnsWindow()) {
        scrollRect(r, dx, dy);
    } else {
        Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
        RDrawableWindow* rw = topData()->rwindow;
        rw->Scroll(TPoint(dx, dy), qt_QRect2TRect(r));
    }
}

/*!
    For this function to work in the emulator, you must add:
       TRANSPARENCY
    To a line in the wsini.ini file.
*/
void QWidgetPrivate::setWindowOpacity_sys(qreal)
{
    // ### TODO: Implement uniform window transparency
}

void QWidgetPrivate::updateFrameStrut()
{

}

void QWidgetPrivate::updateSystemBackground()
{

}

void QWidgetPrivate::registerDropSite(bool on)
{

}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->backingStore = 0;
    extra->topextra->activated = 0;
    extra->topextra->rwindow = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    delete extra->topextra->backingStore;
    extra->topextra->backingStore = 0;
}

void QWidgetPrivate::createSysExtra()
{

}

void QWidgetPrivate::deleteSysExtra()
{

}

QWindowSurface *QWidgetPrivate::createDefaultWindowSurface_sys()
{
    return new QS60WindowSurface(q_func());
}

void QWidgetPrivate::setMask_sys(const QRegion& region)
{

}

int QWidget::metric(PaintDeviceMetric m) const
{
    Q_D(const QWidget);
    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else {
        CWsScreenDevice *scr = S60->screenDevice();
        switch(m) {
        case PdmDpiX:
        case PdmPhysicalDpiX:
            if (d->extra && d->extra->customDpiX) {
                val = d->extra->customDpiX;
            } else {
                const QWidgetPrivate *p = d;
                while (p->parent) {
                    p = static_cast<const QWidget *>(p->parent)->d_func();
                    if (p->extra && p->extra->customDpiX) {
                        val = p->extra->customDpiX;
                        break;
                    }
                }
                if (p == d || !(p->extra && p->extra->customDpiX))
                    val = S60->defaultDpiX;
            }
            break;
        case PdmDpiY:
        case PdmPhysicalDpiY:
            if (d->extra && d->extra->customDpiY) {
                val = d->extra->customDpiY;
            } else {
                const QWidgetPrivate *p = d;
                while (p->parent) {
                    p = static_cast<const QWidget *>(p->parent)->d_func();
                    if (p->extra && p->extra->customDpiY) {
                        val = p->extra->customDpiY;
                        break;
                    }
                }
                if (p == d || !(p->extra && p->extra->customDpiY))
                    val = S60->defaultDpiY;
            }
            break;
        case PdmWidthMM:
        {
            TInt twips = scr->HorizontalPixelsToTwips(data->crect.width());
            val = (int)(twips * (25.4/KTwipsPerInch));
            break;
        }
        case PdmHeightMM:
        {
            TInt twips = scr->VerticalPixelsToTwips(data->crect.height());
            val = (int)(twips * (25.4/KTwipsPerInch));
            break;
        }
        case PdmNumColors:
            val = TDisplayModeUtils::NumDisplayModeColors(scr->DisplayMode());
            break;
        case PdmDepth:
            val = TDisplayModeUtils::NumDisplayModeBitsPerPixel(scr->DisplayMode());
            break;
        default:
            val = 0;
            qWarning("QWidget::metric: Invalid metric command");
        }
    }
    return val;
}

QPaintEngine *QWidget::paintEngine() const
{
    return 0;
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {

        QPoint p = pos + data->crect.topLeft();
        return (isWindow() || !parentWidget()) ?  p : parentWidget()->mapToGlobal(p);

    } else if ((d->data.window_flags & Qt::Window) && internalWinId()) { //toplevel
        QPoint tp = geometry().topLeft();
        return pos + tp;
    }

    // This is the native window case. Consider using CCoeControl::PositionRelativeToScreen()
    // if we decide to go with CCoeControl
    return QPoint();
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {
        QPoint p = (isWindow() || !parentWidget()) ?  pos : parentWidget()->mapFromGlobal(pos);
        return p - data->crect.topLeft();
    } else if ((d->data.window_flags & Qt::Window) && internalWinId()) { //toplevel
        QPoint tp = geometry().topLeft();
        return pos - tp;
    }

    // ### TODO native window
    return QPoint();
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;

    if (isWindow()) {
        createWinId();
        Q_ASSERT(testAttribute(Qt::WA_WState_Created));
        QTLWExtra *top = d->topData();

        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!testAttribute(Qt::WA_Resized) && !isVisible())
            adjustSize();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if ((newstate & Qt::WindowMaximized)) {
                const QRect normalGeometry = geometry();

                const QRect r = top->normalGeometry;
                setGeometry(qApp->desktop()->availableGeometry(this));
                top->normalGeometry = r;

                if (top->normalGeometry.width() < 0)
                    top->normalGeometry = normalGeometry;
            } else {
                // restore original geometry
                setGeometry(top->normalGeometry);
            }
        }
        if ((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            CEikStatusPane* statusPane = S60->statusPane();
            CEikButtonGroupContainer* buttonGroup = S60->buttonGroupContainer();
            if (newstate & Qt::WindowFullScreen) {
                const QRect normalGeometry = geometry();
                const QRect r = top->normalGeometry;
                setGeometry(qApp->desktop()->screenGeometry(this));
                if (statusPane)
                    statusPane->MakeVisible(false);
                if (buttonGroup)
                    buttonGroup->MakeVisible(false);
                top->normalGeometry = r;
                if (top->normalGeometry.width() < 0)
                    top->normalGeometry = normalGeometry;
            } else {
                if (statusPane)
                    statusPane->MakeVisible(true);
                if (buttonGroup)
                    buttonGroup->MakeVisible(true);
                if (newstate & Qt::WindowMaximized) {
                    const QRect r = top->normalGeometry;
                    setGeometry(qApp->desktop()->availableGeometry(this));
                    top->normalGeometry = r;
                } else {
                    setGeometry(top->normalGeometry);
                }
            }
        }
        if ((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (newstate & Qt::WindowMinimized) {
                if (isVisible()) {
                    WId id = effectiveWinId();
                    id->MakeVisible(false);
                    id->ControlEnv()->AppUi()->RemoveFromStack(id);
                }
            } else {
                if (isVisible()) {
                    WId id = effectiveWinId();
                    id->MakeVisible(true);
                    id->ControlEnv()->AppUi()->AddToStackL(id);
                }
                const QRect normalGeometry = geometry();
                const QRect r = top->normalGeometry;
                top->normalGeometry = r;
                if (top->normalGeometry.width() < 0)
                    top->normalGeometry = normalGeometry;
            }
        }
    }

    data->window_state = newstate;

    if (newstate & Qt::WindowActive)
        activateWindow();

    QWindowStateChangeEvent e(oldstate);
    QApplication::sendEvent(this, &e);
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
    if (!isWindow() && parentWidget())
        parentWidget()->d_func()->invalidateBuffer(geometry());
    d->deactivateWidgetCleanup();
    if (testAttribute(Qt::WA_WState_Created)) {

#ifndef QT_NO_IM
        if (d->ic) {
            delete d->ic;
        } else {
            QInputContext *ic = inputContext();
            if (ic) {
                ic->widgetDestroyed(this);
            }
        }
#endif

        setAttribute(Qt::WA_WState_Created, false);
        QObjectList childList = children();
        for (int i = 0; i < childList.size(); ++i) { // destroy all widget children
            register QObject *obj = childList.at(i);
            if (obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows,
                                                    destroySubWindows);
        }
        if (QWidgetPrivate::mouseGrabber == this)
            releaseMouse();
        if (QWidgetPrivate::keyboardGrabber == this)
            releaseKeyboard();
        if (destroyWindow && !(windowType() == Qt::Desktop) && internalWinId()) {
            WId id = internalWinId();
            id->SetFocus(false);
            id->ControlEnv()->AppUi()->RemoveFromStack(id);
            CBase::Delete(id);

            // Hack to activate window under destroyed one. With this activation
            // the next visible window will get keyboard focus
            WId wid = CEikonEnv::Static()->AppUi()->TopFocusedControl();
            if (wid) {
                QWidget *widget = QWidget::find(wid);
                QApplication::setActiveWindow(widget);
            }

        }

        d->setWinId(0);
    }
}

QWidget *QWidget::mouseGrabber()
{
    return QWidgetPrivate::mouseGrabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return QWidgetPrivate::keyboardGrabber;
}

void QWidget::grabKeyboard()
{
    if (!qt_nograb()) {
        if (QWidgetPrivate::keyboardGrabber && QWidgetPrivate::keyboardGrabber != this)
            QWidgetPrivate::keyboardGrabber->releaseKeyboard();

        // ### TODO: Native keyboard grab

        QWidgetPrivate::keyboardGrabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if (!qt_nograb() && QWidgetPrivate::keyboardGrabber == this) {
        // ### TODO: Native keyboard release
        QWidgetPrivate::keyboardGrabber = 0;
    }
}

void QWidget::grabMouse()
{
    if (!qt_nograb()) {
        if (QWidgetPrivate::mouseGrabber && QWidgetPrivate::mouseGrabber != this)
            QWidgetPrivate::mouseGrabber->releaseMouse();
        Q_ASSERT(testAttribute(Qt::WA_WState_Created));
        WId id = effectiveWinId();
        id->SetPointerCapture(true);
        QWidgetPrivate::mouseGrabber = this;
    }
}

void QWidget::releaseMouse()
{
    if (!qt_nograb() && QWidgetPrivate::mouseGrabber == this) {
        Q_ASSERT(testAttribute(Qt::WA_WState_Created));
        WId id = effectiveWinId();
        id->SetPointerCapture(false);
        QWidgetPrivate::mouseGrabber = 0;
    }
}

void QWidget::activateWindow()
{
    Q_D(QWidget);
    QWidget *tlw = window();
    if (tlw->isVisible()) {
        S60->windowGroup().SetOrdinalPosition(0);
        window()->createWinId();
        RDrawableWindow* rw = tlw->d_func()->topData()->rwindow;
        rw->SetOrdinalPosition(0);
    }
}

QT_END_NAMESPACE
