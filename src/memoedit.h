#ifndef MEMOEDIT_H
#define MEMOEDIT_H

#include "precompiled.h"

class MemoEdit : public QPlainTextEdit
{
public:
    MemoEdit(QWidget* parent);

    void setMaxLen(int len);
    void setLenDisplayLabel(QLabel* label);
    void setAcceptButton(QPushButton* button);
    void includeReplyTo(QString replyToAddress);
    void updateDisplay();

private:
    int             maxlen           = 512;
    QLabel*         lenDisplayLabel  = nullptr;
    QPushButton*    acceptButton     = nullptr;
};

#endif // MEMOEDIT_H