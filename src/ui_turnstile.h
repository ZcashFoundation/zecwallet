/********************************************************************************
** Form generated from reading UI file 'turnstile.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TURNSTILE_H
#define UI_TURNSTILE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Turnstile
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLabel *msgIcon;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label;
    QLabel *label_5;
    QSpacerItem *verticalSpacer;
    QLabel *label_6;
    QLabel *label_7;
    QComboBox *migrateZaddList;
    QComboBox *privLevel;
    QLabel *label_8;
    QLabel *label_9;
    QComboBox *migrateTo;
    QFrame *line;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Turnstile)
    {
        if (Turnstile->objectName().isEmpty())
            Turnstile->setObjectName(QStringLiteral("Turnstile"));
        Turnstile->resize(565, 416);
        verticalLayout = new QVBoxLayout(Turnstile);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(Turnstile);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        msgIcon = new QLabel(groupBox);
        msgIcon->setObjectName(QStringLiteral("msgIcon"));
        msgIcon->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(msgIcon, 0, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout->addWidget(label_3, 5, 0, 2, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 4, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);

        gridLayout->addWidget(label_4, 5, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 2, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);

        gridLayout->addWidget(label_5, 6, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 7, 0, 1, 2);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 5, 2, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 6, 2, 1, 1);

        migrateZaddList = new QComboBox(groupBox);
        migrateZaddList->setObjectName(QStringLiteral("migrateZaddList"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(migrateZaddList->sizePolicy().hasHeightForWidth());
        migrateZaddList->setSizePolicy(sizePolicy2);
        migrateZaddList->setEditable(false);

        gridLayout->addWidget(migrateZaddList, 2, 1, 1, 2);

        privLevel = new QComboBox(groupBox);
        privLevel->setObjectName(QStringLiteral("privLevel"));
        sizePolicy2.setHeightForWidth(privLevel->sizePolicy().hasHeightForWidth());
        privLevel->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(privLevel, 4, 1, 1, 2);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setWordWrap(true);

        gridLayout->addWidget(label_8, 0, 1, 1, 2);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));
        sizePolicy1.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_9, 3, 0, 1, 1);

        migrateTo = new QComboBox(groupBox);
        migrateTo->setObjectName(QStringLiteral("migrateTo"));

        gridLayout->addWidget(migrateTo, 3, 1, 1, 2);

        line = new QFrame(groupBox);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 1, 0, 1, 3);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(Turnstile);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(Turnstile);
        QObject::connect(buttonBox, SIGNAL(accepted()), Turnstile, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Turnstile, SLOT(reject()));

        QMetaObject::connectSlotsByName(Turnstile);
    } // setupUi

    void retranslateUi(QDialog *Turnstile)
    {
        Turnstile->setWindowTitle(QApplication::translate("Turnstile", "Turnstile Migration", nullptr));
        groupBox->setTitle(QApplication::translate("Turnstile", "Turnstile Migration", nullptr));
        msgIcon->setText(QString());
        label_3->setText(QApplication::translate("Turnstile", "Total Fees", nullptr));
        label_2->setText(QApplication::translate("Turnstile", "Privacy Level", nullptr));
        label_4->setText(QApplication::translate("Turnstile", "Miner Fee:", nullptr));
        label->setText(QApplication::translate("Turnstile", "From", nullptr));
        label_5->setText(QApplication::translate("Turnstile", "Dev Fee:    ", nullptr));
        label_6->setText(QApplication::translate("Turnstile", "0.0004 ZEC $0.04", nullptr));
        label_7->setText(QApplication::translate("Turnstile", "0.0004 ZEC $0.04", nullptr));
        migrateZaddList->setCurrentText(QString());
        label_8->setText(QApplication::translate("Turnstile", "<html><head/><body><p>Funds from Sprout z-Addresses (which start with &quot;zc&quot;) need to be moved to the upgraded Sapling z-Addresses (which start with &quot;zs&quot;). The funds cannot be moved directly, but need to be sent through intermediate &quot;transparent&quot; addresses in privacy-preserving way.</p><p>This migration can be done automatically for you.</p></body></html>", nullptr));
        label_9->setText(QApplication::translate("Turnstile", "To", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Turnstile: public Ui_Turnstile {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TURNSTILE_H
