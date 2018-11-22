#ifndef QRCODELABEL_H
#define QRCODELABEL_H

#include "precompiled.h"

class QRCodeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit        QRCodeLabel(QWidget *parent = nullptr);
    virtual QSize   sizeHint() const;
    
    void            setAddress(QString address);
    QPixmap         scaledPixmap() const;
public slots:    
    void resizeEvent(QResizeEvent *);

private:
    QString address;
};


#endif // QRCODELABEL_H
