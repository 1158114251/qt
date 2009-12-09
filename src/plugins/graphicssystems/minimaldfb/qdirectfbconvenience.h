#ifndef QDIRECTFBCONVENIENCE_H
#define QDIRECTFBCONVENIENCE_H

#include <QtGui/qimage.h>
#include <QtCore/QHash>
#include <QtCore/QEvent>

#include <directfb/directfb.h>

class QDirectFbKeyMap: public QHash<DFBInputDeviceKeySymbol, Qt::Key>
{
public:
    QDirectFbKeyMap();
};


class QDirectFbConvenience
{
public:
    static QImage::Format imageFormatFromSurface(IDirectFBSurface *surface);

    //This is set by the graphicssystem constructor
    static IDirectFB *dfbInterface() { return dfb; }
    static Qt::MouseButton mouseButton(DFBInputDeviceButtonIdentifier identifier);
    static Qt::MouseButtons mouseButtons(DFBInputDeviceButtonMask mask);
    static QEvent::Type eventType(DFBWindowEventType type);

    static QDirectFbKeyMap *keyMap();

private:
    static void setDfbInterface(IDirectFB *dfbInterface) {dfb = dfbInterface;}
    static IDirectFB *dfb;
    static QDirectFbKeyMap *dfbKeymap;
    friend class QDirectFbGraphicsSystem;
};

#endif // QDIRECTFBCONVENIENCE_H
