/********************************************************************************
** Form generated from reading UI file 'confirm.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIRM_H
#define UI_CONFIRM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_confirm
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QLabel *sendFrom;
    QGroupBox *sendToAddrs;
    QGridLayout *gridLayout;
    QLabel *Addr1;
    QLabel *Amt1;
    QSpacerItem *verticalSpacer;
    QFrame *line;
    QLabel *label;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *confirm)
    {
        if (confirm->objectName().isEmpty())
            confirm->setObjectName(QStringLiteral("confirm"));
        confirm->resize(429, 371);
        verticalLayout = new QVBoxLayout(confirm);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(confirm);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        sendFrom = new QLabel(groupBox);
        sendFrom->setObjectName(QStringLiteral("sendFrom"));
        sendFrom->setWordWrap(true);

        verticalLayout_2->addWidget(sendFrom);


        verticalLayout->addWidget(groupBox);

        sendToAddrs = new QGroupBox(confirm);
        sendToAddrs->setObjectName(QStringLiteral("sendToAddrs"));
        gridLayout = new QGridLayout(sendToAddrs);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        Addr1 = new QLabel(sendToAddrs);
        Addr1->setObjectName(QStringLiteral("Addr1"));
        Addr1->setWordWrap(true);

        gridLayout->addWidget(Addr1, 0, 0, 1, 1);

        Amt1 = new QLabel(sendToAddrs);
        Amt1->setObjectName(QStringLiteral("Amt1"));
        Amt1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(Amt1, 0, 1, 1, 1);


        verticalLayout->addWidget(sendToAddrs);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        line = new QFrame(confirm);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        label = new QLabel(confirm);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);

        buttonBox = new QDialogButtonBox(confirm);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(confirm);
        QObject::connect(buttonBox, SIGNAL(accepted()), confirm, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), confirm, SLOT(reject()));

        QMetaObject::connectSlotsByName(confirm);
    } // setupUi

    void retranslateUi(QDialog *confirm)
    {
        confirm->setWindowTitle(QApplication::translate("confirm", "Confirm Transaction", nullptr));
        groupBox->setTitle(QApplication::translate("confirm", "From", nullptr));
        sendFrom->setText(QString());
        sendToAddrs->setTitle(QApplication::translate("confirm", "To", nullptr));
        Addr1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        Amt1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        label->setText(QApplication::translate("confirm", "Fees: 0.0001 ZEC", nullptr));
    } // retranslateUi

};

namespace Ui {
    class confirm: public Ui_confirm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIRM_H
