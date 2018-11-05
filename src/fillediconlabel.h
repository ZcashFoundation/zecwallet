#ifndef FILLEDICONLABEL_H
#define FILLEDICONLABEL_H

#include "precompiled.h"

class FilledIconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit        FilledIconLabel(QWidget *parent = 0);

public slots:
    void resizeEvent(QResizeEvent *);

};


#endif // FILLEDICONLABEL_H