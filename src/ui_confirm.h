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
    QLabel *devFee;
    QLabel *Amt1;
    QLabel *labelDevFee;
    QLabel *minerFee;
    QLabel *Addr1;
    QLabel *labelMinerFee;
    QLabel *AmtUSD1;
    QLabel *Memo1;
    QLabel *minerFeeUSD;
    QLabel *devFeeUSD;
    QSpacerItem *verticalSpacer;
    QFrame *line;
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
        devFee = new QLabel(sendToAddrs);
        devFee->setObjectName(QStringLiteral("devFee"));
        devFee->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(devFee, 3, 1, 1, 1);

        Amt1 = new QLabel(sendToAddrs);
        Amt1->setObjectName(QStringLiteral("Amt1"));
        Amt1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(Amt1, 0, 1, 1, 1);

        labelDevFee = new QLabel(sendToAddrs);
        labelDevFee->setObjectName(QStringLiteral("labelDevFee"));

        gridLayout->addWidget(labelDevFee, 3, 0, 1, 1);

        minerFee = new QLabel(sendToAddrs);
        minerFee->setObjectName(QStringLiteral("minerFee"));
        minerFee->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(minerFee, 2, 1, 1, 1);

        Addr1 = new QLabel(sendToAddrs);
        Addr1->setObjectName(QStringLiteral("Addr1"));
        Addr1->setWordWrap(true);

        gridLayout->addWidget(Addr1, 0, 0, 1, 1);

        labelMinerFee = new QLabel(sendToAddrs);
        labelMinerFee->setObjectName(QStringLiteral("labelMinerFee"));

        gridLayout->addWidget(labelMinerFee, 2, 0, 1, 1);

        AmtUSD1 = new QLabel(sendToAddrs);
        AmtUSD1->setObjectName(QStringLiteral("AmtUSD1"));
        AmtUSD1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(AmtUSD1, 0, 2, 1, 1);

        Memo1 = new QLabel(sendToAddrs);
        Memo1->setObjectName(QStringLiteral("Memo1"));

        gridLayout->addWidget(Memo1, 1, 0, 1, 3);

        minerFeeUSD = new QLabel(sendToAddrs);
        minerFeeUSD->setObjectName(QStringLiteral("minerFeeUSD"));
        minerFeeUSD->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(minerFeeUSD, 2, 2, 1, 1);

        devFeeUSD = new QLabel(sendToAddrs);
        devFeeUSD->setObjectName(QStringLiteral("devFeeUSD"));
        devFeeUSD->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(devFeeUSD, 3, 2, 1, 1);


        verticalLayout->addWidget(sendToAddrs);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        line = new QFrame(confirm);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

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
        devFee->setText(QApplication::translate("confirm", "Dev Fee Amount", nullptr));
        Amt1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        labelDevFee->setText(QApplication::translate("confirm", "Dev Fee", nullptr));
        minerFee->setText(QApplication::translate("confirm", "Miner Amount", nullptr));
        Addr1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        labelMinerFee->setText(QApplication::translate("confirm", "Miner Fee", nullptr));
        AmtUSD1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        Memo1->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        minerFeeUSD->setText(QApplication::translate("confirm", "TextLabel", nullptr));
        devFeeUSD->setText(QApplication::translate("confirm", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class confirm: public Ui_confirm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIRM_H
