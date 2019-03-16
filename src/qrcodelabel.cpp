#include "qrcodelabel.h"

QRCodeLabel::QRCodeLabel(QWidget *parent) :
    QLabel(parent)
{
    this->setMinimumSize(100, 100);
    setScaledContents(false);
}

QSize QRCodeLabel::sizeHint() const
{
    int w = this->width();
    return QSize(w, w);   // 1:1 
}

void QRCodeLabel::resizeEvent(QResizeEvent*)
{
    if(!str.isEmpty())
        QLabel::setPixmap(scaledPixmap());
}

QPixmap QRCodeLabel::scaledPixmap() const {
    QPixmap pm(size());
    pm.fill(Qt::white);
    QPainter painter(&pm);
    
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(str.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    const int    s      = qr.getSize()>0?qr.getSize():1;
    const double w      = pm.width();
    const double h      = pm.height();
    const double aspect = w/h;
    const double size   = ((aspect>1.0)?h:w);
    const double scale  = size/(s+2);
    const double woff   = (w - size) > 0 ? (w - size) / 2 : 0;
    const double hoff   = (h - size) > 0 ? (h - size) / 2 : 0;
    
    // NOTE: For performance reasons my implementation only draws the foreground parts    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(Qt::black));
    for(int y=0; y<s; y++) {
        for(int x=0; x<s; x++) {
            const int color=qr.getModule(x, y);  // 0 for white, 1 for black
            if(0!=color) {
                const double rx1=(x+1)*scale + woff, ry1=(y+1)*scale + hoff;
                QRectF r(rx1, ry1, scale, scale);
                painter.drawRects(&r,1);
            }
        }
    }
    
    return pm;
}

void QRCodeLabel::setQrcodeString(QString stra) {
    str = stra;
    QLabel::setPixmap(scaledPixmap());
}