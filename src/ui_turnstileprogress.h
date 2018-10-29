/********************************************************************************
** Form generated from reading UI file 'turnstileprogress.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TURNSTILEPROGRESS_H
#define UI_TURNSTILEPROGRESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_TurnstileProgress
{
public:
    QGridLayout *gridLayout;
    QProgressBar *progressBar;
    QLabel *label_4;
    QFrame *line;
    QLabel *nextTx;
    QLabel *progressTxt;
    QLabel *label_2;
    QLabel *msgIcon;
    QDialogButtonBox *buttonBox;
    QSpacerItem *verticalSpacer;
    QFrame *line_2;

    void setupUi(QDialog *TurnstileProgress)
    {
        if (TurnstileProgress->objectName().isEmpty())
            TurnstileProgress->setObjectName(QStringLiteral("TurnstileProgress"));
        TurnstileProgress->resize(400, 300);
        gridLayout = new QGridLayout(TurnstileProgress);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        progressBar = new QProgressBar(TurnstileProgress);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(33);

        gridLayout->addWidget(progressBar, 3, 0, 1, 3);

        label_4 = new QLabel(TurnstileProgress);
        label_4->setObjectName(QStringLiteral("label_4"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);
        label_4->setWordWrap(true);

        gridLayout->addWidget(label_4, 7, 1, 1, 2);

        line = new QFrame(TurnstileProgress);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 4, 0, 1, 3);

        nextTx = new QLabel(TurnstileProgress);
        nextTx->setObjectName(QStringLiteral("nextTx"));

        gridLayout->addWidget(nextTx, 5, 0, 1, 3);

        progressTxt = new QLabel(TurnstileProgress);
        progressTxt->setObjectName(QStringLiteral("progressTxt"));
        progressTxt->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(progressTxt, 2, 2, 1, 1);

        label_2 = new QLabel(TurnstileProgress);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 2);

        msgIcon = new QLabel(TurnstileProgress);
        msgIcon->setObjectName(QStringLiteral("msgIcon"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(msgIcon->sizePolicy().hasHeightForWidth());
        msgIcon->setSizePolicy(sizePolicy1);
        msgIcon->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(msgIcon, 7, 0, 1, 1);

        buttonBox = new QDialogButtonBox(TurnstileProgress);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Discard|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(false);

        gridLayout->addWidget(buttonBox, 9, 0, 1, 3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 6, 0, 1, 3);

        line_2 = new QFrame(TurnstileProgress);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 8, 0, 1, 3);


        retranslateUi(TurnstileProgress);
        QObject::connect(buttonBox, SIGNAL(accepted()), TurnstileProgress, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), TurnstileProgress, SLOT(reject()));

        QMetaObject::connectSlotsByName(TurnstileProgress);
    } // setupUi

    void retranslateUi(QDialog *TurnstileProgress)
    {
        TurnstileProgress->setWindowTitle(QApplication::translate("TurnstileProgress", "Turnstile Migration Progress", nullptr));
        label_4->setText(QApplication::translate("TurnstileProgress", "Please ensure you have your wallet.dat backed up!", nullptr));
        nextTx->setText(QApplication::translate("TurnstileProgress", "Next Transaction in 4 hours", nullptr));
        progressTxt->setText(QApplication::translate("TurnstileProgress", "4 / 12", nullptr));
        label_2->setText(QApplication::translate("TurnstileProgress", "Migration Progress", nullptr));
        msgIcon->setText(QApplication::translate("TurnstileProgress", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TurnstileProgress: public Ui_TurnstileProgress {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TURNSTILEPROGRESS_H
