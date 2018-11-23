#include "addresscombo.h"
#include "addressbook.h"
#include "settings.h"

AddressCombo::AddressCombo(QWidget* parent) : 
    QComboBox(parent) {
}

QString AddressCombo::itemText(int i) {
    QString txt = QComboBox::itemText(i);
    return AddressBook::addressFromAddressLabel(txt.split("(")[0].trimmed());
}

QString AddressCombo::currentText() {
    QString txt = QComboBox::currentText();
    return AddressBook::addressFromAddressLabel(txt.split("(")[0].trimmed());
}

void AddressCombo::setCurrentText(const QString& text) {
    for (int i=0; i < count(); i++) {
        if (itemText(i) == text) {
            QComboBox::setCurrentIndex(i);
        }
    }
} 

void AddressCombo::addItem(const QString& text, double bal) {
    QString txt = AddressBook::addLabelToAddress(text);
    if (bal > 0)
        txt = txt % "(" % Settings::getZECDisplayFormat(bal) % ")";
        
    QComboBox::addItem(txt);
}

void AddressCombo::insertItem(int index, const QString& text, double bal) {
    QString txt = AddressBook::addLabelToAddress(text) % 
                    "(" % Settings::getZECDisplayFormat(bal) % ")";
    QComboBox::insertItem(index, txt);
}