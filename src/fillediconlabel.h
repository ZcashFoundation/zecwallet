#ifndef FILLEDICONLABEL_H
#define FILLEDICONLABEL_H

#include "precompiled.h"

class FilledIconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit        FilledIconLabel(QWidget *parent = 0);
    void            setBasePixmap(QPixmap pm);

public slots:
    void resizeEvent(QResizeEvent *);

private:
    QPixmap basePm;
};


#endif // FILLEDICONLABEL_H