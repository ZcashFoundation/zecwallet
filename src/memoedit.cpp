#include "memoedit.h"

MemoEdit::MemoEdit(QWidget* parent) : QPlainTextEdit(parent) {
    QObject::connect(this, &QPlainTextEdit::textChanged, this, &MemoEdit::updateDisplay);
}

void MemoEdit::updateDisplay() {
    QString txt = this->toPlainText();
    if (lenDisplayLabel)
        lenDisplayLabel->setText(QString::number(txt.toUtf8().size()) + "/" + QString::number(maxlen));

    if (txt.toUtf8().size() <= maxlen) {
        // Everything is fine
        if (acceptButton)
            acceptButton->setEnabled(true);

        if (lenDisplayLabel)
            lenDisplayLabel->setStyleSheet("");
    }
    else {
        // Overweight
        if (acceptButton)
            acceptButton->setEnabled(false);

        if (lenDisplayLabel)
            lenDisplayLabel->setStyleSheet("color: red;");
    }
}

void MemoEdit::setMaxLen(int len) {
    this->maxlen = len;
    updateDisplay();
}

void MemoEdit::setLenDisplayLabel(QLabel* label) {
    this->lenDisplayLabel = label;
}

void MemoEdit::setAcceptButton(QPushButton* button) {
    this->acceptButton = button;
}

void MemoEdit::includeReplyTo(QString addr) {
    if (addr.isEmpty())
        return;
    
    auto curText = this->toPlainText();
    if (curText.endsWith(addr))
        return;

    this->setPlainText(curText + "\n" + tr("Reply to") + ":\n" + addr);
}