/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionSettings;
    QAction *actionDonate;
    QAction *actionImport_Private_Keys;
    QAction *actionCheck_for_Updates;
    QAction *actionTurnstile_Migration;
    QWidget *centralWidget;
    QGridLayout *gridLayout_3;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_5;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *balSheilded;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLabel *balTransparent;
    QFrame *line;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLabel *balTotal;
    QSpacerItem *verticalSpacer;
    QLabel *unconfirmedWarning;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QTableView *balancesTable;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_8;
    QComboBox *inputsCombo;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_5;
    QLineEdit *sendAddressBalance;
    QLabel *sendAddressBalanceUSD;
    QSpacerItem *horizontalSpacer_6;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QScrollArea *sendToScrollArea;
    QWidget *sendToWidgets;
    QVBoxLayout *sendToLayout;
    QGroupBox *verticalGroupBox;
    QVBoxLayout *sendAddressLayout;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_4;
    QLineEdit *Address1;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_6;
    QLineEdit *Amount1;
    QLabel *AmtUSD1;
    QCheckBox *Max1;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *MemoBtn1;
    QLabel *MemoTxt1;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *addAddressButton;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_7;
    QLabel *lblMinerFee;
    QLabel *lblMinerFeeUSD;
    QLabel *lblDevFeeTxt;
    QLabel *lblDevFee;
    QLabel *lblDevFeeUSD;
    QSpacerItem *horizontalSpacer;
    QPushButton *sendTransactionButton;
    QPushButton *cancelSendButton;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_8;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_9;
    QHBoxLayout *horizontalLayout_9;
    QRadioButton *rdioZSAddr;
    QRadioButton *rdioZAddr;
    QRadioButton *rdioTAddr;
    QHBoxLayout *horizontalLayout_10;
    QComboBox *listRecieveAddresses;
    QPushButton *btnRecieveNewAddr;
    QHBoxLayout *horizontalLayout_11;
    QPlainTextEdit *txtRecieve;
    QLabel *qrcodeDisplay;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_2;
    QTableView *transactionsTable;
    QMenuBar *menuBar;
    QMenu *menuBalance;
    QMenu *menuHelp;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(889, 603);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/res/icon.ico"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QStringLiteral("actionSettings"));
        actionDonate = new QAction(MainWindow);
        actionDonate->setObjectName(QStringLiteral("actionDonate"));
        actionImport_Private_Keys = new QAction(MainWindow);
        actionImport_Private_Keys->setObjectName(QStringLiteral("actionImport_Private_Keys"));
        actionImport_Private_Keys->setVisible(false);
        actionCheck_for_Updates = new QAction(MainWindow);
        actionCheck_for_Updates->setObjectName(QStringLiteral("actionCheck_for_Updates"));
        actionTurnstile_Migration = new QAction(MainWindow);
        actionTurnstile_Migration->setObjectName(QStringLiteral("actionTurnstile_Migration"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout_3 = new QGridLayout(centralWidget);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setMinimumSize(QSize(300, 0));
        groupBox->setFlat(false);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        balSheilded = new QLabel(groupBox);
        balSheilded->setObjectName(QStringLiteral("balSheilded"));
        balSheilded->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(balSheilded);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        balTransparent = new QLabel(groupBox);
        balTransparent->setObjectName(QStringLiteral("balTransparent"));
        balTransparent->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(balTransparent);


        verticalLayout->addLayout(horizontalLayout_2);

        line = new QFrame(groupBox);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_3->setFont(font);

        horizontalLayout_3->addWidget(label_3);

        balTotal = new QLabel(groupBox);
        balTotal->setObjectName(QStringLiteral("balTotal"));
        balTotal->setFont(font);
        balTotal->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(balTotal);


        verticalLayout->addLayout(horizontalLayout_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        unconfirmedWarning = new QLabel(groupBox);
        unconfirmedWarning->setObjectName(QStringLiteral("unconfirmedWarning"));
        unconfirmedWarning->setStyleSheet(QStringLiteral("color: red;"));

        verticalLayout->addWidget(unconfirmedWarning);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        horizontalLayout_5->addWidget(groupBox);

        groupBox_2 = new QGroupBox(tab);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        balancesTable = new QTableView(groupBox_2);
        balancesTable->setObjectName(QStringLiteral("balancesTable"));
        balancesTable->setSelectionMode(QAbstractItemView::SingleSelection);
        balancesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        balancesTable->setCornerButtonEnabled(false);
        balancesTable->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        balancesTable->horizontalHeader()->setStretchLastSection(true);
        balancesTable->verticalHeader()->setVisible(false);

        horizontalLayout_4->addWidget(balancesTable);


        horizontalLayout_5->addWidget(groupBox_2);


        gridLayout_2->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout_4 = new QVBoxLayout(tab_2);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        groupBox_4 = new QGroupBox(tab_2);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setAutoFillBackground(false);
        groupBox_4->setFlat(false);
        verticalLayout_6 = new QVBoxLayout(groupBox_4);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        inputsCombo = new QComboBox(groupBox_4);
        inputsCombo->setObjectName(QStringLiteral("inputsCombo"));

        horizontalLayout_8->addWidget(inputsCombo);


        verticalLayout_6->addLayout(horizontalLayout_8);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setSpacing(6);
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label_5 = new QLabel(groupBox_4);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_15->addWidget(label_5);

        sendAddressBalance = new QLineEdit(groupBox_4);
        sendAddressBalance->setObjectName(QStringLiteral("sendAddressBalance"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(sendAddressBalance->sizePolicy().hasHeightForWidth());
        sendAddressBalance->setSizePolicy(sizePolicy);
        sendAddressBalance->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        sendAddressBalance->setReadOnly(true);

        horizontalLayout_15->addWidget(sendAddressBalance);

        sendAddressBalanceUSD = new QLabel(groupBox_4);
        sendAddressBalanceUSD->setObjectName(QStringLiteral("sendAddressBalanceUSD"));

        horizontalLayout_15->addWidget(sendAddressBalanceUSD);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer_6);


        verticalLayout_6->addLayout(horizontalLayout_15);


        verticalLayout_4->addWidget(groupBox_4);

        groupBox_3 = new QGroupBox(tab_2);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setFlat(false);
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        sendToScrollArea = new QScrollArea(groupBox_3);
        sendToScrollArea->setObjectName(QStringLiteral("sendToScrollArea"));
        sendToScrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        sendToScrollArea->setWidgetResizable(true);
        sendToWidgets = new QWidget();
        sendToWidgets->setObjectName(QStringLiteral("sendToWidgets"));
        sendToWidgets->setGeometry(QRect(0, 0, 841, 321));
        sendToLayout = new QVBoxLayout(sendToWidgets);
        sendToLayout->setSpacing(6);
        sendToLayout->setContentsMargins(11, 11, 11, 11);
        sendToLayout->setObjectName(QStringLiteral("sendToLayout"));
        verticalGroupBox = new QGroupBox(sendToWidgets);
        verticalGroupBox->setObjectName(QStringLiteral("verticalGroupBox"));
        sendAddressLayout = new QVBoxLayout(verticalGroupBox);
        sendAddressLayout->setSpacing(6);
        sendAddressLayout->setContentsMargins(11, 11, 11, 11);
        sendAddressLayout->setObjectName(QStringLiteral("sendAddressLayout"));
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setSpacing(6);
        horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
        label_4 = new QLabel(verticalGroupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout_12->addWidget(label_4);

        Address1 = new QLineEdit(verticalGroupBox);
        Address1->setObjectName(QStringLiteral("Address1"));

        horizontalLayout_12->addWidget(Address1);


        sendAddressLayout->addLayout(horizontalLayout_12);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(6);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        label_6 = new QLabel(verticalGroupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_13->addWidget(label_6);

        Amount1 = new QLineEdit(verticalGroupBox);
        Amount1->setObjectName(QStringLiteral("Amount1"));
        Amount1->setBaseSize(QSize(200, 0));
        Amount1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(Amount1);

        AmtUSD1 = new QLabel(verticalGroupBox);
        AmtUSD1->setObjectName(QStringLiteral("AmtUSD1"));

        horizontalLayout_13->addWidget(AmtUSD1);

        Max1 = new QCheckBox(verticalGroupBox);
        Max1->setObjectName(QStringLiteral("Max1"));

        horizontalLayout_13->addWidget(Max1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_4);

        MemoBtn1 = new QPushButton(verticalGroupBox);
        MemoBtn1->setObjectName(QStringLiteral("MemoBtn1"));
        MemoBtn1->setEnabled(true);

        horizontalLayout_13->addWidget(MemoBtn1);


        sendAddressLayout->addLayout(horizontalLayout_13);

        MemoTxt1 = new QLabel(verticalGroupBox);
        MemoTxt1->setObjectName(QStringLiteral("MemoTxt1"));
        QFont font1;
        font1.setPointSize(10);
        MemoTxt1->setFont(font1);

        sendAddressLayout->addWidget(MemoTxt1);


        sendToLayout->addWidget(verticalGroupBox);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_2);

        addAddressButton = new QPushButton(sendToWidgets);
        addAddressButton->setObjectName(QStringLiteral("addAddressButton"));
        addAddressButton->setBaseSize(QSize(200, 0));

        horizontalLayout_7->addWidget(addAddressButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_3);


        sendToLayout->addLayout(horizontalLayout_7);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        sendToLayout->addItem(verticalSpacer_2);

        sendToScrollArea->setWidget(sendToWidgets);

        verticalLayout_3->addWidget(sendToScrollArea);


        verticalLayout_4->addWidget(groupBox_3);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_7 = new QLabel(tab_2);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_6->addWidget(label_7);

        lblMinerFee = new QLabel(tab_2);
        lblMinerFee->setObjectName(QStringLiteral("lblMinerFee"));

        horizontalLayout_6->addWidget(lblMinerFee);

        lblMinerFeeUSD = new QLabel(tab_2);
        lblMinerFeeUSD->setObjectName(QStringLiteral("lblMinerFeeUSD"));

        horizontalLayout_6->addWidget(lblMinerFeeUSD);

        lblDevFeeTxt = new QLabel(tab_2);
        lblDevFeeTxt->setObjectName(QStringLiteral("lblDevFeeTxt"));

        horizontalLayout_6->addWidget(lblDevFeeTxt);

        lblDevFee = new QLabel(tab_2);
        lblDevFee->setObjectName(QStringLiteral("lblDevFee"));

        horizontalLayout_6->addWidget(lblDevFee);

        lblDevFeeUSD = new QLabel(tab_2);
        lblDevFeeUSD->setObjectName(QStringLiteral("lblDevFeeUSD"));

        horizontalLayout_6->addWidget(lblDevFeeUSD);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        sendTransactionButton = new QPushButton(tab_2);
        sendTransactionButton->setObjectName(QStringLiteral("sendTransactionButton"));
        sendTransactionButton->setBaseSize(QSize(100, 0));
        sendTransactionButton->setFlat(false);

        horizontalLayout_6->addWidget(sendTransactionButton);

        cancelSendButton = new QPushButton(tab_2);
        cancelSendButton->setObjectName(QStringLiteral("cancelSendButton"));
        cancelSendButton->setBaseSize(QSize(100, 0));

        horizontalLayout_6->addWidget(cancelSendButton);


        verticalLayout_4->addLayout(horizontalLayout_6);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_5 = new QVBoxLayout(tab_3);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        groupBox_6 = new QGroupBox(tab_3);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setFlat(false);
        groupBox_6->setCheckable(false);
        verticalLayout_9 = new QVBoxLayout(groupBox_6);
        verticalLayout_9->setSpacing(6);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        rdioZSAddr = new QRadioButton(groupBox_6);
        rdioZSAddr->setObjectName(QStringLiteral("rdioZSAddr"));

        horizontalLayout_9->addWidget(rdioZSAddr);

        rdioZAddr = new QRadioButton(groupBox_6);
        rdioZAddr->setObjectName(QStringLiteral("rdioZAddr"));

        horizontalLayout_9->addWidget(rdioZAddr);

        rdioTAddr = new QRadioButton(groupBox_6);
        rdioTAddr->setObjectName(QStringLiteral("rdioTAddr"));
        sizePolicy.setHeightForWidth(rdioTAddr->sizePolicy().hasHeightForWidth());
        rdioTAddr->setSizePolicy(sizePolicy);

        horizontalLayout_9->addWidget(rdioTAddr);


        verticalLayout_9->addLayout(horizontalLayout_9);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        listRecieveAddresses = new QComboBox(groupBox_6);
        listRecieveAddresses->setObjectName(QStringLiteral("listRecieveAddresses"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(listRecieveAddresses->sizePolicy().hasHeightForWidth());
        listRecieveAddresses->setSizePolicy(sizePolicy1);
        listRecieveAddresses->setDuplicatesEnabled(false);

        horizontalLayout_10->addWidget(listRecieveAddresses);

        btnRecieveNewAddr = new QPushButton(groupBox_6);
        btnRecieveNewAddr->setObjectName(QStringLiteral("btnRecieveNewAddr"));

        horizontalLayout_10->addWidget(btnRecieveNewAddr);


        verticalLayout_9->addLayout(horizontalLayout_10);


        verticalLayout_8->addWidget(groupBox_6);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        txtRecieve = new QPlainTextEdit(tab_3);
        txtRecieve->setObjectName(QStringLiteral("txtRecieve"));
        txtRecieve->setReadOnly(true);

        horizontalLayout_11->addWidget(txtRecieve);

        qrcodeDisplay = new QLabel(tab_3);
        qrcodeDisplay->setObjectName(QStringLiteral("qrcodeDisplay"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(qrcodeDisplay->sizePolicy().hasHeightForWidth());
        qrcodeDisplay->setSizePolicy(sizePolicy2);
        qrcodeDisplay->setStyleSheet(QStringLiteral("background-color: #fff"));

        horizontalLayout_11->addWidget(qrcodeDisplay);


        verticalLayout_8->addLayout(horizontalLayout_11);


        verticalLayout_5->addLayout(verticalLayout_8);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        verticalLayout_2 = new QVBoxLayout(tab_4);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        transactionsTable = new QTableView(tab_4);
        transactionsTable->setObjectName(QStringLiteral("transactionsTable"));
        transactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
        transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayout_2->addWidget(transactionsTable);

        tabWidget->addTab(tab_4, QString());

        gridLayout_3->addWidget(tabWidget, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 889, 22));
        menuBalance = new QMenu(menuBar);
        menuBalance->setObjectName(QStringLiteral("menuBalance"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        QWidget::setTabOrder(tabWidget, inputsCombo);
        QWidget::setTabOrder(inputsCombo, sendAddressBalance);
        QWidget::setTabOrder(sendAddressBalance, sendToScrollArea);
        QWidget::setTabOrder(sendToScrollArea, Address1);
        QWidget::setTabOrder(Address1, Amount1);
        QWidget::setTabOrder(Amount1, Max1);
        QWidget::setTabOrder(Max1, addAddressButton);
        QWidget::setTabOrder(addAddressButton, sendTransactionButton);
        QWidget::setTabOrder(sendTransactionButton, cancelSendButton);
        QWidget::setTabOrder(cancelSendButton, balancesTable);
        QWidget::setTabOrder(balancesTable, rdioZAddr);
        QWidget::setTabOrder(rdioZAddr, rdioTAddr);
        QWidget::setTabOrder(rdioTAddr, listRecieveAddresses);
        QWidget::setTabOrder(listRecieveAddresses, btnRecieveNewAddr);
        QWidget::setTabOrder(btnRecieveNewAddr, txtRecieve);
        QWidget::setTabOrder(txtRecieve, transactionsTable);

        menuBar->addAction(menuBalance->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuBalance->addAction(actionImport_Private_Keys);
        menuBalance->addAction(actionTurnstile_Migration);
        menuBalance->addAction(actionSettings);
        menuBalance->addSeparator();
        menuBalance->addAction(actionExit);
        menuHelp->addAction(actionDonate);
        menuHelp->addAction(actionCheck_for_Updates);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "zec-qt-wallet", nullptr));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", nullptr));
        actionAbout->setText(QApplication::translate("MainWindow", "About", nullptr));
        actionSettings->setText(QApplication::translate("MainWindow", "Settings", nullptr));
        actionDonate->setText(QApplication::translate("MainWindow", "Donate", nullptr));
        actionImport_Private_Keys->setText(QApplication::translate("MainWindow", "Import Private Keys", nullptr));
        actionCheck_for_Updates->setText(QApplication::translate("MainWindow", "Check github.com for Updates", nullptr));
        actionTurnstile_Migration->setText(QApplication::translate("MainWindow", "Turnstile Migration", nullptr));
        groupBox->setTitle(QApplication::translate("MainWindow", "Summary", nullptr));
        label->setText(QApplication::translate("MainWindow", "Shielded", nullptr));
        balSheilded->setText(QString());
        label_2->setText(QApplication::translate("MainWindow", "Transparent", nullptr));
        balTransparent->setText(QString());
        label_3->setText(QApplication::translate("MainWindow", "Total", nullptr));
        balTotal->setText(QString());
        unconfirmedWarning->setText(QApplication::translate("MainWindow", "Some transactions are not yet confirmed", nullptr));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "Address Balances", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Balance", nullptr));
        groupBox_4->setTitle(QApplication::translate("MainWindow", "Pay From", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "Address Balance", nullptr));
        sendAddressBalanceUSD->setText(QString());
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Send To", nullptr));
        verticalGroupBox->setTitle(QApplication::translate("MainWindow", "Recipient", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "Address", nullptr));
        Address1->setPlaceholderText(QApplication::translate("MainWindow", "Address", nullptr));
        label_6->setText(QApplication::translate("MainWindow", "Amount", nullptr));
        Amount1->setPlaceholderText(QApplication::translate("MainWindow", "Amount", nullptr));
        AmtUSD1->setText(QString());
        Max1->setText(QApplication::translate("MainWindow", "Max Available", nullptr));
#ifndef QT_NO_TOOLTIP
        MemoBtn1->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        MemoBtn1->setText(QApplication::translate("MainWindow", "Memo", nullptr));
        MemoTxt1->setText(QString());
        addAddressButton->setText(QApplication::translate("MainWindow", "Add Address", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "Miner Fee:", nullptr));
        lblMinerFee->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        lblMinerFeeUSD->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        lblDevFeeTxt->setText(QApplication::translate("MainWindow", "Dev Fee:", nullptr));
        lblDevFee->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        lblDevFeeUSD->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        sendTransactionButton->setText(QApplication::translate("MainWindow", "Send", nullptr));
        cancelSendButton->setText(QApplication::translate("MainWindow", "Cancel", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "Send", nullptr));
        groupBox_6->setTitle(QApplication::translate("MainWindow", "Address Type", nullptr));
        rdioZSAddr->setText(QApplication::translate("MainWindow", "z-Addr(Sapling)", nullptr));
        rdioZAddr->setText(QApplication::translate("MainWindow", "z-Addr(Sprout)", nullptr));
        rdioTAddr->setText(QApplication::translate("MainWindow", "t-Addr", nullptr));
        btnRecieveNewAddr->setText(QApplication::translate("MainWindow", "New Address", nullptr));
        qrcodeDisplay->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("MainWindow", "Receive", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("MainWindow", "Transactions", nullptr));
        menuBalance->setTitle(QApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
