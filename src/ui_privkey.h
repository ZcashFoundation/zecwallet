/********************************************************************************
** Form generated from reading UI file 'privkey.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRIVKEY_H
#define UI_PRIVKEY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>

QT_BEGIN_NAMESPACE

class Ui_PrivKey
{
public:
    QGridLayout *gridLayout;
    QPlainTextEdit *privKeyTxt;
    QDialogButtonBox *buttonBox;
    QLabel *helpLbl;

    void setupUi(QDialog *PrivKey)
    {
        if (PrivKey->objectName().isEmpty())
            PrivKey->setObjectName(QStringLiteral("PrivKey"));
        PrivKey->resize(461, 389);
        gridLayout = new QGridLayout(PrivKey);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        privKeyTxt = new QPlainTextEdit(PrivKey);
        privKeyTxt->setObjectName(QStringLiteral("privKeyTxt"));

        gridLayout->addWidget(privKeyTxt, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(PrivKey);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 1);

        helpLbl = new QLabel(PrivKey);
        helpLbl->setObjectName(QStringLiteral("helpLbl"));

        gridLayout->addWidget(helpLbl, 0, 0, 1, 1);


        retranslateUi(PrivKey);
        QObject::connect(buttonBox, SIGNAL(accepted()), PrivKey, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), PrivKey, SLOT(reject()));

        QMetaObject::connectSlotsByName(PrivKey);
    } // setupUi

    void retranslateUi(QDialog *PrivKey)
    {
        PrivKey->setWindowTitle(QApplication::translate("PrivKey", "Private Key", nullptr));
        privKeyTxt->setPlainText(QString());
        helpLbl->setText(QApplication::translate("PrivKey", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PrivKey: public Ui_PrivKey {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRIVKEY_H
