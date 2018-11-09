#include "fillediconlabel.h"

FilledIconLabel::FilledIconLabel(QWidget* parent) :
    QLabel(parent) {
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}

void FilledIconLabel::setBasePixmap(QPixmap pm) {
    basePm = pm;
}

/**
 * When resized, we re-draw the whole pixmap, resizing it as needed. 
 */ 
void FilledIconLabel::resizeEvent(QResizeEvent*) {    
    QSize sz = size();  
    
    QPixmap scaled = basePm.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap p(sz);
    p.fill(Qt::white);
    QPainter painter(&p);
    painter.drawPixmap((sz.width() - scaled.width()) / 2, (sz.height() - scaled.height()) / 2, scaled);
    
    QLabel::setPixmap(p);
}