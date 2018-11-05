#include "fillediconlabel.h"

FilledIconLabel::FilledIconLabel(QWidget* parent) :
    QLabel(parent) {
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}


void FilledIconLabel::resizeEvent(QResizeEvent*) {
    // Top pixmap
    QIcon icon(":/icons/res/icon.ico");
    QSize sz = size();
    qDebug() << sz;
    QPixmap p(sz);
    p.fill(Qt::white);
    QPainter painter(&p);
    painter.drawPixmap((sz.width() - sz.height()) / 2, 0, 
                        icon.pixmap(sz.height(), sz.height()));
    
    QLabel::setPixmap(p);
}