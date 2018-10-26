/********************************************************************************
** Form generated from reading UI file 'memodialog.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEMODIALOG_H
#define UI_MEMODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_MemoDialog
{
public:
    QGridLayout *gridLayout;
    QLineEdit *memoTxt;
    QLabel *label;
    QLabel *memoSize;
    QDialogButtonBox *buttonBox;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *MemoDialog)
    {
        if (MemoDialog->objectName().isEmpty())
            MemoDialog->setObjectName(QStringLiteral("MemoDialog"));
        MemoDialog->resize(618, 115);
        gridLayout = new QGridLayout(MemoDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        memoTxt = new QLineEdit(MemoDialog);
        memoTxt->setObjectName(QStringLiteral("memoTxt"));

        gridLayout->addWidget(memoTxt, 1, 0, 1, 2);

        label = new QLabel(MemoDialog);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        memoSize = new QLabel(MemoDialog);
        memoSize->setObjectName(QStringLiteral("memoSize"));
        memoSize->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(memoSize, 0, 1, 1, 1);

        buttonBox = new QDialogButtonBox(MemoDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 3, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 2, 0, 1, 2);


        retranslateUi(MemoDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), MemoDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MemoDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(MemoDialog);
    } // setupUi

    void retranslateUi(QDialog *MemoDialog)
    {
        MemoDialog->setWindowTitle(QApplication::translate("MemoDialog", "Dialog", nullptr));
        label->setText(QApplication::translate("MemoDialog", "Memo", nullptr));
        memoSize->setText(QApplication::translate("MemoDialog", "6 / 512", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MemoDialog: public Ui_MemoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEMODIALOG_H
