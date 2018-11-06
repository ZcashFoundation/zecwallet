#include "fillediconlabel.h"

FilledIconLabel::FilledIconLabel(QWidget* parent) :
    QLabel(parent) {
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}

void FilledIconLabel::setBasePixmap(QPixmap pm) {
    basePm = pm;
}

void FilledIconLabel::resizeEvent(QResizeEvent*) {
    // Top pixmap    
    QSize sz = size();  
    
    QPixmap scaled = basePm.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap p(sz);
    p.fill(Qt::white);
    QPainter painter(&p);
    painter.drawPixmap((sz.width() - scaled.width()) / 2, (sz.height() - scaled.height()) / 2, scaled);
    
    QLabel::setPixmap(p);
}