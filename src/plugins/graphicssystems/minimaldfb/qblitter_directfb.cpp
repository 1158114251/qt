#include "qblitter_directfb.h"
#include "qgraphicssystem_minimaldfb.h"

#include <QDebug>

#include <directfb.h>

QDirectFbBlitter::QDirectFbBlitter(const QRect &rect, IDirectFBSurface *surface)
        : QBlittable(rect, QBlittable::Capabilities(QBlittable::SolidRectCapability
                                                          |QBlittable::SourcePixmapCapability
                                                          |QBlittable::SourceOverPixmapCapability
                                                          |QBlittable::SourceOverScaledPixmapCapability))
//                0))
{
    DFBSurfaceDescription surfaceDesc;
    surfaceDesc.width = rect.width();
    surfaceDesc.height = rect.height();
    surfaceDesc.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT);

    if (surface) {
        m_surface = surface;
    } else {
        IDirectFB *dfb = QDirectFbGraphicsSystem::dfbInterface();
        dfb->CreateSurface(dfb,&surfaceDesc, &m_surface);
    }
}

void QDirectFbBlitter::fillRect(const QRectF &rect, const QColor &color)
{
    m_surface->SetColor(m_surface, color.red(), color.green(), color.blue(), color.alpha());
    m_surface->SetDrawingFlags(m_surface, DSDRAW_NOFX);
    m_surface->FillRectangle(m_surface, rect.x(), rect.y(),
                              rect.width(), rect.height());
}

void QDirectFbBlitter::drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &srcRect)
{
    quint32 blittingFlags = pixmap.hasAlphaChannel() ? DSBLIT_BLEND_ALPHACHANNEL : DSBLIT_NOFX;

    m_surface->SetBlittingFlags(m_surface, DFBSurfaceBlittingFlags(blittingFlags));
    m_surface->SetColor(m_surface, 0xff, 0xff, 0xff, 255);

//    QPixmapData *data = pixmap.pixmapData();
//    Q_ASSERT(data->classId() == QPixmapData::DirectFBClass);
//    QDirectFBPixmapData *dfbData = static_cast<QDirectFBPixmapData*>(data);
//    IDirectFBSurface *s = dfbData->directFBSurface();
//    const DFBRectangle sRect = { srcRect.x(), srcRect.y(), rect.width(), rect.height() };
//
//    DFBResult result;
//    if (rect.width() == srcRect.width() && rect.height() == srcRect.height())
//        result = surface->Blit(surface, s, &sRect, rect.x(), rect.y());
//    else {
//        const DFBRectangle dRect = { rect.x(), rect.y(), rect.width(), rect.height() };
//        result = surface->StretchBlit(surface, s, &sRect, &dRect);
//    }
//    if (result != DFB_OK)
//        DirectFBError("QDirectFBBlitter::drawPixmap()", result);

}

QImage *QDirectFbBlitter::lock()
{
    if (m_image.isNull()) {
    if(!m_surface)
        qDebug() << "Trying to lock null surface";
    if (!this->rect().isValid()) {
        qDebug() << "No valid blitter rect";
        return 0;
    }



        void *mem;
        int bpl;
        const DFBResult result = m_surface->Lock(m_surface, DFBSurfaceLockFlags(DSLF_WRITE|DSLF_READ), static_cast<void**>(&mem), &bpl);
        if (result == DFB_OK) {
            QImage::Format format = QDirectFbGraphicsSystem::imageFormatFromSurface(m_surface);
            int w, h;
            m_surface->GetSize(m_surface,&w,&h);
            m_image = QImage(static_cast<uchar *>(mem),w,h,format);
        } else {
            DirectFBError("Failed to lock image", result);
        }

    }

    return &m_image;
}

void QDirectFbBlitter::unlock()
{
    m_surface->Unlock(m_surface);
    m_image = QImage();
}
