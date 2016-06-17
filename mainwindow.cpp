#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tinyxml2.h"
#include "QFileDialog"
#include "iostream"
#include "QDirIterator"
#include "QDebug"
#include "QGraphicsScene"
#include "QGraphicsView"
#include "QScrollBar"
#include "QVector2D"
#include "QLabel"
#include "iostream"
#include "QListWidgetItem"
#include "QTreeWidget"
#include "QMouseEvent"
#include "QMessageBox"
#include <QGraphicsEllipseItem>
#include <QFont>
#include <QFontMetrics>
#include <QInputDialog>
#include <QHoverEvent>

using namespace std;
using namespace tinyxml2;

const int cClassLineLength = 2500;
const int cSceneSizeIncrement = 500;
int cHorizontalSpacing = 150; //steps
int cVerticalSpacing = 30; //steps
const int cTaskflowHorizontalMidpoint = 200;
int cTaskflowVerticalSpacing = 100;
const QString cIcdLocation = "/home/jryan/simulation/dev/common/icd/";
const QString cSTDLocation = "/home/jryan/simulation/dev/common/icd/STDs/";
const char* aModifiedStatus[] ={"eNotModified","eRemoved", "eAdded","eModified"};
const char* aAcceptedStatus[] ={"eToBeAssessed", "eAccepted","eRejected"};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    icdScene = new QGraphicsScene();
    functionScene = new QGraphicsScene();
    taskflowScene = new QGraphicsScene();
    icdBrowser = ui->messageBrowser;

//    onRevertToStockIcds();
    ParseStdIcds();
    SetupFileDirectories();
    SetupMessageBrowser();
    SetupIcdMenu();
    SetupDrawingArea();
    SetupContextMenu();
    SetupTaskflowScene();

    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
                this, SLOT(onListIcdClicked(QListWidgetItem *)));
    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(onScrollEvent(int)));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(onScrollEvent(int)));
    connect(ui->addDataExchange, SIGNAL(clicked()),
                this, SLOT(onAddDataExchangeClicked()));
    connect(ui->actionHome, SIGNAL(triggered()),
                this, SLOT(onHomeTriggered()));
    connect(ui->addPilotInteraction, SIGNAL(clicked()),
                this, SLOT(onAddPilotInteraction()));
    connect(ui->functionsGraphicsView, SIGNAL(customContextMenuRequested(const QPoint)),this,
              SLOT(functionContextMenuRequested(const QPoint&)));
    connect(ui->taskflowGraphicsView, SIGNAL(customContextMenuRequested(const QPoint)),this,
              SLOT(taskflowContextMenuRequested(const QPoint&)));
    connect(functionDropdownMenu, SIGNAL(triggered(QAction*)),this,
              SLOT(onFunctionDropdownMenuClicked(QAction*)));
    connect(taskflowDropdownMenu, SIGNAL(triggered(QAction*)),this,
              SLOT(onTaskflowDropdownMenuClicked(QAction*)));
    connect(ui->actionSave_Function, SIGNAL(triggered()),this,
              SLOT(onSaveFunction()));
    connect(ui->actionLoad_Function, SIGNAL(triggered()),this,
              SLOT(onLoadFunction()));
    connect(ui->actionRename, SIGNAL(triggered()),this,
              SLOT(onRenameFunction()));
    connect(ui->actionHome_All, SIGNAL(triggered()),this,
              SLOT(onHomeAllTriggered()));
    connect(ui->addFunction, SIGNAL(clicked()),this,
              SLOT(onAddFunctionToTask()));
    connect(ui->actionNew, SIGNAL(triggered()),this,
              SLOT(onNewTaskflowScene()));
    connect(ui->actionSave_As, SIGNAL(triggered()),this,
              SLOT(onSaveTaskflowScene()));
    connect(ui->actionOpen, SIGNAL(triggered()),this,
              SLOT(onLoadTaskflowScene()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)),this,
              SLOT(onTabChange(int)));
    connect(ui->actionReload_STD_XMLs, SIGNAL(triggered()),this,
              SLOT(onParseStdXmls()));
    connect(ui->actionReload_Stock_XMLs, SIGNAL(triggered()),this,
              SLOT(onParseStockXmls()));
    connect(ui->actionSave_XML_Changes, SIGNAL(triggered()),this,
              SLOT(onSaveXmlChanges()));
    connect(ui->actionSave_to_JPG, SIGNAL(triggered()),this,
              SLOT(onSaveToJPG()));
    connect(ui->actionCheck_for_ICD_Changes, SIGNAL(triggered()),this,
              SLOT(onCheckforChanges()));
    connect(ui->actionRevert_to_stock_ICDs, SIGNAL(triggered()),this,
              SLOT(onRevertToStockIcds()));
    connect(ui->actionToggle_Function_Style, SIGNAL(triggered()),this,
              SLOT(onToggleFunctionStyle()));
    connect(ui->actionToggle_Task_Style, SIGNAL(triggered()),this,
              SLOT(onToggleTaskStyle()));
    connect(ui->actionHorizontal_Spacing, SIGNAL(triggered()),this,
              SLOT(onHorizontalSpacing()));
    connect(ui->actionVertical_Spacing, SIGNAL(triggered()),this,
              SLOT(onVerticalSpacing()));
    connect(ui->actionVertical_Spacing_2, SIGNAL(triggered()),this,
              SLOT(onTaskVerticalSpacing()));
    connect(ui->actionFunctionComments, SIGNAL(triggered()),this,
              SLOT(onFunctionComments()));
    connect(ui->actionTaskComments, SIGNAL(triggered()),this,
              SLOT(onTaskComments()));
    connect(ui->actionAccept_all_modifications, SIGNAL(triggered()),this,
              SLOT(onAcceptAllMods()));
    connect(ui->actionReject_all_modification, SIGNAL(triggered()),this,
              SLOT(onRejectAllMods()));

    ui->statusBar->showMessage(QString::number(ui->graphicsView->verticalScrollBar()->value()) + " " + QString::number(ui->graphicsView->horizontalScrollBar()->value()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ParseStockIcds()
{
    bool yesToAll;
    v_ICDs.clear();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open ICD Directory"),
                                                    cIcdLocation,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::NoIteratorFlags);

    if(!it.hasNext())
        exit(0);

    while (it.hasNext())
    {

            std::string filePath;
            filePath = it.next().toStdString();
            if(it.fileName().toStdString() != "DatabaseH145.xml" &&
                    it.fileName().toStdString() != "IcdModelQTGHost.xml" &&
                    it.fileName().toStdString() != "IcdModelTemplate.xml" &&
                    it.fileName().toStdString() != "IcdModelQTGExecutor.xml")
            {
                string Rmsf;
                string Name;
                XMLDocument doc;
                doc.LoadFile( filePath.c_str() );
                XMLElement* root = doc.FirstChildElement("RmsfInterface");
                Rmsf = root->Attribute("namespace");
                Name = root->Attribute("name");
                QDateTime recentIcdDate = it.fileInfo().lastModified();
                string strName(Name);

                string textNode = root->Attribute("name");
                qDebug() << textNode.c_str();

                ICD newICD;

                for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
                {
                    qDebug() << element->Name();

                    if(strcmp(element->Name(), "Subscribe") == 0)
                    {

                        for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
                        {
                            string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                            string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                            qDebug() << SubscribeCheckIcd.c_str();
                            qDebug() << SubscribeCheckMessage.c_str();

                            SubMessage* subMessage = new SubMessage(
                                        SubscribeCheckIcd, SubscribeCheckMessage, aModifiedStatus[eNotModified], ""); //Smells like memory leaks.
                            newICD.v_pSubscribedMessages.append(subMessage);
                        }


                    }

                    if(strcmp(element->Name(), "Message") == 0)
                    {
                        string MessageCheckName = element->Attribute("name");
                        const char* MessageCheckNetwork = element->Attribute("network");
                        const char* MessageCheckComment = element->Attribute("comment");
                        qDebug() << MessageCheckName.c_str();
                        qDebug() << MessageCheckNetwork;
                        qDebug() << MessageCheckComment;

                        QVector<MessageParameter*> msgPrms;


                        for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementPublish->Attribute("name");
                            const char* ParameterCheckType = elementPublish->Attribute("type");
                            const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                            const char* ParameterCheckDefault = elementPublish->Attribute("default");
                            const char* ParameterCheckMin = elementPublish->Attribute("min");
                            const char* ParameterCheckMax = elementPublish->Attribute("max");
                            const char* ParameterCheckComment = elementPublish->Attribute("comment");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == NULL)
                                ParameterCheckUnit = "";
                            if(ParameterCheckMin == NULL)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == NULL)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == NULL)
                                ParameterCheckComment = "";
                            if(ParameterCheckDefault == NULL)
                                ParameterCheckDefault = "";


                            MessageParameter* messageParameter = new MessageParameter(
                                        ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, "", "", -1); //Smells like memory leaks.

                            msgPrms.append(messageParameter);

                        }

                        if(MessageCheckNetwork == NULL)
                            MessageCheckNetwork = "";
                        if(MessageCheckComment == NULL)
                            MessageCheckComment = "";
                        PubMessage* pubMessage = new PubMessage(
                                    MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, aModifiedStatus[eNotModified], "");

                        newICD.v_pPublishedMessages.append(pubMessage);



                    }

                    if(strcmp(element->Name(), "Enum") == 0)
                    {
                        string EnumCheckName = element->Attribute("name");
                        string EnumCheckComment = element->Attribute("comment");
                        qDebug() << EnumCheckName.c_str();
                        qDebug() << EnumCheckComment.c_str();

                        QVector<EnumValue*> enumValues;

                        for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
                        {
                            string ValueCheckName = elementValue->Attribute("name");
                            string ValueCheckValue = elementValue->Attribute("value");
                            string ValueCheckComment = elementValue->Attribute("comment");

                            qDebug() << ValueCheckName.c_str();
                            qDebug() << ValueCheckValue.c_str();
                            qDebug() << ValueCheckComment.c_str();

                            EnumValue* enumValue = new EnumValue(
                                        ValueCheckName,ValueCheckValue,ValueCheckComment, "", ""); //Smells like memory leaks.

                            enumValues.append(enumValue);
                        }

                        Enum* eNum = new Enum(
                                    EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                        newICD.v_pEnums.append(eNum);
                    }

                    if(strcmp(element->Name(), "Struct") == 0)
                    {
                        string StructCheckName = element->Attribute("name");
                        const char* StructCheckNetwork = element->Attribute("network");
                        string StructCheckComment = element->Attribute("comment");
                        qDebug() << StructCheckName.c_str();
                        qDebug() << StructCheckNetwork;
                        qDebug() << StructCheckComment.c_str();

                        QVector<StructParameter*> strPrms;


                        for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementStruct->Attribute("name");
                            const char* ParameterCheckType = elementStruct->Attribute("type");
                            const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                            const char* ParameterCheckDefault = elementStruct->Attribute("default");
                            const char* ParameterCheckMin = elementStruct->Attribute("min");
                            const char* ParameterCheckMax = elementStruct->Attribute("max");
                            const char* ParameterCheckComment = elementStruct->Attribute("comment");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == 0)
                                ParameterCheckUnit = "";
                            if(ParameterCheckDefault == 0)
                                ParameterCheckDefault = "";
                            if(ParameterCheckMin == 0)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == 0)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == 0)
                                ParameterCheckComment = "";

                            StructParameter* structParameter = new StructParameter(
                                        ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, "", ""); //Smells like memory leaks.

                            strPrms.append(structParameter);
                        }

                        if(StructCheckNetwork == NULL)
                            StructCheckNetwork = "";

                        Struct* str = new Struct(
                                    StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

                        newICD.v_pStructs.append(str);
                    }
                }

                newICD.name = strName;
                QString parsedName = strName.c_str();
                parsedName.remove("Icd", Qt::CaseSensitive);
                parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
                newICD.parsedName = parsedName.toStdString();
                newICD.recentIcdDate = recentIcdDate;

                v_ICDs.append(newICD);


                //This takes the existing file and saves it into
                QFile file(filePath.c_str());
                if(!QDir(cSTDLocation + QString::fromStdString(strName)).exists())
                {
                    if(!yesToAll)
                    {
                        if(QMessageBox::YesToAll == QMessageBox::critical(this, QString::fromStdString(strName) +" is new",
                                                            "An ICD that has not been scanned before has been detected, and the folder directory will now be created",
                                                            QMessageBox::Ok, QMessageBox::YesToAll))
                        {
                            yesToAll = true;
                        }
                    }
                    QDir dir(cSTDLocation + QString::fromStdString(strName));
                    if (!dir.exists()) {
                        dir.mkpath(".");
                        dir.mkpath("Functions");
                        dir.mkpath("Tasks");
                        dir.mkpath("Diagrams");
                    }
                }
//                qDebug()<<file.copy(STDsFolder + QString::fromStdString(strName) + "/" +
//                                    QString::fromStdString(strName) + ".xml");
        }
    }

    if(v_ICDs.isEmpty())
    {
        QMessageBox::critical(this, "No ICDs found",
                                            "No ICDs could be found in the directory you chose. Make sure you are looking for stock ICDs and not STD ICDs."
                              "\nReload the program and try again.",
                                            QMessageBox::Ok);

        exit(0);
    }
}

void MainWindow::ParseStdIcds()
{
    v_ICDs.clear();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open STD Directory"),
                                                    cSTDLocation,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);

    if(!it.hasNext())
        exit(0);

    while (it.hasNext())
    {

            std::string filePath;
            filePath = it.next().toStdString();
            if(filePath.find("stdIcd") != string::npos)
            if(it.fileName().toStdString() != "DatabaseH145.xml" &&
                    it.fileName().toStdString() != "IcdModelQTGHost.xml" &&
                    it.fileName().toStdString() != "IcdModelTemplate.xml" &&
                    it.fileName().toStdString() != "FlightModelH145_AerotimInputs.xml" &&
                    it.fileName().toStdString() != "IcdModelQTGExecutor.xml")
            {
                string Rmsf;
                string Name;
                XMLDocument doc;
                doc.LoadFile( filePath.c_str() );
                XMLElement* root = doc.FirstChildElement("StdInterface");
                Rmsf = root->Attribute("namespace");
                Name = root->Attribute("name");
                QDateTime recentIcdDate = QDateTime::fromString(root->Attribute("recentIcdDate"),Qt::TextDate);
                string strName(Name);

                string textNode = root->Attribute("name");
                qDebug() << textNode.c_str();

                ICD newICD;

                for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
                {
                    qDebug() << element->Name();

                    if(strcmp(element->Name(), "Subscribe") == 0)
                    {

                        for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
                        {
                            string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                            string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                            string SubscribeCheckModified= elementSubscribe->Attribute("modified");
                            string SubscribeCheckAccepted = elementSubscribe->Attribute("accepted");
                            qDebug() << SubscribeCheckIcd.c_str();
                            qDebug() << SubscribeCheckMessage.c_str();

                            SubMessage* subMessage = new SubMessage(
                                        SubscribeCheckIcd, SubscribeCheckMessage, SubscribeCheckModified, SubscribeCheckAccepted); //Smells like memory leaks.
                            newICD.v_pSubscribedMessages.append(subMessage);
                        }


                    }

                    if(strcmp(element->Name(), "Message") == 0)
                    {
                        string MessageCheckName = element->Attribute("name");
                        const char* MessageCheckNetwork = element->Attribute("network");
                        const char* MessageCheckComment = element->Attribute("comment");
                        const char* MessageCheckMod = element->Attribute("modified");
                        const char* MessageCheckAcc = element->Attribute("accepted");
                        qDebug() << MessageCheckName.c_str();
                        qDebug() << MessageCheckNetwork;
                        qDebug() << MessageCheckComment;

                        QVector<MessageParameter*> msgPrms;


                        for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementPublish->Attribute("name");
                            const char* ParameterCheckType = elementPublish->Attribute("type");
                            const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                            const char* ParameterCheckDefault = elementPublish->Attribute("default");
                            const char* ParameterCheckMin = elementPublish->Attribute("min");
                            const char* ParameterCheckMax = elementPublish->Attribute("max");
                            const char* ParameterCheckComment = elementPublish->Attribute("comment");
                            const char* ParameterCheckMod = elementPublish->Attribute("modified");
                            const char* ParameterCheckAcc = elementPublish->Attribute("accepted");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == NULL)
                                ParameterCheckUnit = "";
                            if(ParameterCheckMin == NULL)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == NULL)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == NULL)
                                ParameterCheckComment = "";
                            if(ParameterCheckDefault == NULL)
                                ParameterCheckDefault = "";


                            MessageParameter* messageParameter = new MessageParameter(
                                        ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, ParameterCheckMod, ParameterCheckAcc, -1); //Smells like memory leaks.

                            msgPrms.append(messageParameter);

                        }

                        if(MessageCheckNetwork == NULL)
                            MessageCheckNetwork = "";
                        if(MessageCheckComment == NULL)
                            MessageCheckComment = "";
                        PubMessage* pubMessage = new PubMessage(
                                    MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, MessageCheckMod, MessageCheckAcc);

                        newICD.v_pPublishedMessages.append(pubMessage);



                    }

                    if(strcmp(element->Name(), "Enum") == 0)
                    {
                        string EnumCheckName = element->Attribute("name");
                        string EnumCheckComment = element->Attribute("comment");
                        qDebug() << EnumCheckName.c_str();
                        qDebug() << EnumCheckComment.c_str();

                        QVector<EnumValue*> enumValues;

                        for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
                        {
                            string ValueCheckName = elementValue->Attribute("name");
                            string ValueCheckValue = elementValue->Attribute("value");
                            string ValueCheckComment = elementValue->Attribute("comment");
                            string ValueCheckMod = elementValue->Attribute("modified");
                            string ValueCheckAcc = elementValue->Attribute("accepted");

                            qDebug() << ValueCheckName.c_str();
                            qDebug() << ValueCheckValue.c_str();
                            qDebug() << ValueCheckComment.c_str();

                            EnumValue* enumValue = new EnumValue(
                                        ValueCheckName,ValueCheckValue,ValueCheckComment,ValueCheckMod,ValueCheckAcc); //Smells like memory leaks.

                            enumValues.append(enumValue);
                        }

                        Enum* eNum = new Enum(
                                    EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                        newICD.v_pEnums.append(eNum);


                    }

                    if(strcmp(element->Name(), "Struct") == 0)
                    {
                        string StructCheckName = element->Attribute("name");
                        const char* StructCheckNetwork = element->Attribute("network");
                        string StructCheckComment = element->Attribute("comment");
                        qDebug() << StructCheckName.c_str();
                        qDebug() << StructCheckNetwork;
                        qDebug() << StructCheckComment.c_str();

                        QVector<StructParameter*> strPrms;


                        for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementStruct->Attribute("name");
                            const char* ParameterCheckType = elementStruct->Attribute("type");
                            const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                            const char* ParameterCheckDefault = elementStruct->Attribute("default");
                            const char* ParameterCheckMin = elementStruct->Attribute("min");
                            const char* ParameterCheckMax = elementStruct->Attribute("max");
                            const char* ParameterCheckComment = elementStruct->Attribute("comment");
                            const char* ParameterCheckMod = elementStruct->Attribute("modified");
                            const char* ParameterCheckAcc = elementStruct->Attribute("accepted");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == 0)
                                ParameterCheckUnit = "";
                            if(ParameterCheckDefault == 0)
                                ParameterCheckDefault = "";
                            if(ParameterCheckMin == 0)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == 0)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == 0)
                                ParameterCheckComment = "";

                            StructParameter* structParameter = new StructParameter(
                                        ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, ParameterCheckMod, ParameterCheckAcc); //Smells like memory leaks.

                            strPrms.append(structParameter);
                        }

                        if(StructCheckNetwork == NULL)
                            StructCheckNetwork = "";

                        Struct* str = new Struct(
                                    StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

                        newICD.v_pStructs.append(str);
                    }
                }

                newICD.name = strName;
                QString parsedName = strName.c_str();
                parsedName.remove("Icd", Qt::CaseSensitive);
                parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
                newICD.parsedName = parsedName.toStdString();
                newICD.recentIcdDate = recentIcdDate;


                v_ICDs.append(newICD);

                ui->tabWidget->setCurrentIndex(0); //reset back to the icd tab to avoid errors

                /*
                //This takes the existing file and saves it into
                QFile file(filePath.c_str());
                QString STDsFolder = "/home/jryan/simulation/dev/common/icd/STDs/";
                if(!QDir(STDsFolder + QString::fromStdString(strName)).exists())
                {
                    QMessageBox::critical(this, QString::fromStdString(strName) +" is new",
                                                        "An ICD that has not been scanned before has been detected, and the folder directory will now be created",
                                                        QMessageBox::Ok);

                    QDir dir(STDsFolder + QString::fromStdString(strName));
                    if (!dir.exists()) {
                        dir.mkpath(".");
                        dir.mkpath("Functions");
                        dir.mkpath("Tasks");
                    }
                }
                qDebug()<<file.copy(STDsFolder + QString::fromStdString(strName) + "/" +
                                    QString::fromStdString(strName) + ".xml")
                */
        }
    }

    if(v_ICDs.isEmpty())
    {
        QMessageBox::critical(this, "No ICDs found",
                                            "No ICDs could be found in the directory you chose. Make sure you are looking for STD ICDs and not stock ICDs."
                              "\nReload the program and try again.",
                                            QMessageBox::Ok);

        exit(0);
    }
}

ICD MainWindow::ParseSingleStdIcd(QString _filePath)
{
    QString filePath = _filePath;
    string Rmsf;
    string Name;
    XMLDocument doc;
    doc.LoadFile( filePath.toStdString().c_str() );
    XMLElement* root = doc.FirstChildElement("StdInterface");
    Rmsf = root->Attribute("namespace");
    Name = root->Attribute("name");
    QDateTime recentIcdDate = QDateTime::fromString(root->Attribute("recentIcdDate"),Qt::TextDate);
    string strName(Name);

    string textNode = root->Attribute("name");
    qDebug() << textNode.c_str();

    ICD newICD;

    for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
    {
        qDebug() << element->Name();

        if(strcmp(element->Name(), "Subscribe") == 0)
        {

            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                string SubscribeCheckModified= elementSubscribe->Attribute("modified");
                string SubscribeCheckAccepted = elementSubscribe->Attribute("accepted");
                qDebug() << SubscribeCheckIcd.c_str();
                qDebug() << SubscribeCheckMessage.c_str();

                SubMessage* subMessage = new SubMessage(
                            SubscribeCheckIcd, SubscribeCheckMessage, SubscribeCheckModified, SubscribeCheckAccepted); //Smells like memory leaks.
                newICD.v_pSubscribedMessages.append(subMessage);
            }


        }

        if(strcmp(element->Name(), "Message") == 0)
        {
            string MessageCheckName = element->Attribute("name");
            const char* MessageCheckNetwork = element->Attribute("network");
            const char* MessageCheckComment = element->Attribute("comment");
            const char* MessageCheckMod = element->Attribute("modified");
            const char* MessageCheckAcc = element->Attribute("accepted");
            qDebug() << MessageCheckName.c_str();
            qDebug() << MessageCheckNetwork;
            qDebug() << MessageCheckComment;

            QVector<MessageParameter*> msgPrms;


            for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
            {
                const char* ParameterCheckName = elementPublish->Attribute("name");
                const char* ParameterCheckType = elementPublish->Attribute("type");
                const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                const char* ParameterCheckDefault = elementPublish->Attribute("default");
                const char* ParameterCheckMin = elementPublish->Attribute("min");
                const char* ParameterCheckMax = elementPublish->Attribute("max");
                const char* ParameterCheckComment = elementPublish->Attribute("comment");
                const char* ParameterCheckMod = elementPublish->Attribute("modified");
                const char* ParameterCheckAcc = elementPublish->Attribute("accepted");
                qDebug() << ParameterCheckName;
                qDebug() << ParameterCheckType;
                qDebug() << ParameterCheckUnit;
                qDebug() << ParameterCheckDefault;
                qDebug() << ParameterCheckMin;
                qDebug() << ParameterCheckMax;
                qDebug() << ParameterCheckComment;

                if(ParameterCheckUnit == NULL)
                    ParameterCheckUnit = "";
                if(ParameterCheckMin == NULL)
                    ParameterCheckMin = "";
                if(ParameterCheckMax == NULL)
                    ParameterCheckMax = "";
                if(ParameterCheckComment == NULL)
                    ParameterCheckComment = "";
                if(ParameterCheckDefault == NULL)
                    ParameterCheckDefault = "";


                MessageParameter* messageParameter = new MessageParameter(
                            ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                            ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                            ParameterCheckComment, ParameterCheckMod, ParameterCheckAcc, -1); //Smells like memory leaks.

                msgPrms.append(messageParameter);

            }

            if(MessageCheckNetwork == NULL)
                MessageCheckNetwork = "";
            if(MessageCheckComment == NULL)
                MessageCheckComment = "";
            PubMessage* pubMessage = new PubMessage(
                        MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, MessageCheckMod, MessageCheckAcc);

            newICD.v_pPublishedMessages.append(pubMessage);



        }

        if(strcmp(element->Name(), "Enum") == 0)
        {
            string EnumCheckName = element->Attribute("name");
            string EnumCheckComment = element->Attribute("comment");
            qDebug() << EnumCheckName.c_str();
            qDebug() << EnumCheckComment.c_str();

            QVector<EnumValue*> enumValues;

            for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
            {
                string ValueCheckName = elementValue->Attribute("name");
                string ValueCheckValue = elementValue->Attribute("value");
                string ValueCheckComment = elementValue->Attribute("comment");
                string ValueCheckMod = elementValue->Attribute("modified");
                string ValueCheckAcc = elementValue->Attribute("accepted");

                qDebug() << ValueCheckName.c_str();
                qDebug() << ValueCheckValue.c_str();
                qDebug() << ValueCheckComment.c_str();

                EnumValue* enumValue = new EnumValue(
                            ValueCheckName,ValueCheckValue,ValueCheckComment,ValueCheckMod,ValueCheckAcc); //Smells like memory leaks.

                enumValues.append(enumValue);
            }

            Enum* eNum = new Enum(
                        EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
            newICD.v_pEnums.append(eNum);


        }

        if(strcmp(element->Name(), "Struct") == 0)
        {
            string StructCheckName = element->Attribute("name");
            const char* StructCheckNetwork = element->Attribute("network");
            string StructCheckComment = element->Attribute("comment");
            qDebug() << StructCheckName.c_str();
            qDebug() << StructCheckNetwork;
            qDebug() << StructCheckComment.c_str();

            QVector<StructParameter*> strPrms;


            for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
            {
                const char* ParameterCheckName = elementStruct->Attribute("name");
                const char* ParameterCheckType = elementStruct->Attribute("type");
                const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                const char* ParameterCheckDefault = elementStruct->Attribute("default");
                const char* ParameterCheckMin = elementStruct->Attribute("min");
                const char* ParameterCheckMax = elementStruct->Attribute("max");
                const char* ParameterCheckComment = elementStruct->Attribute("comment");
                const char* ParameterCheckMod = elementStruct->Attribute("modified");
                const char* ParameterCheckAcc = elementStruct->Attribute("accepted");
                qDebug() << ParameterCheckName;
                qDebug() << ParameterCheckType;
                qDebug() << ParameterCheckUnit;
                qDebug() << ParameterCheckDefault;
                qDebug() << ParameterCheckMin;
                qDebug() << ParameterCheckMax;
                qDebug() << ParameterCheckComment;

                if(ParameterCheckUnit == 0)
                    ParameterCheckUnit = "";
                if(ParameterCheckDefault == 0)
                    ParameterCheckDefault = "";
                if(ParameterCheckMin == 0)
                    ParameterCheckMin = "";
                if(ParameterCheckMax == 0)
                    ParameterCheckMax = "";
                if(ParameterCheckComment == 0)
                    ParameterCheckComment = "";

                StructParameter* structParameter = new StructParameter(
                            ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                            ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                            ParameterCheckComment, ParameterCheckMod, ParameterCheckAcc); //Smells like memory leaks.

                strPrms.append(structParameter);
            }

            if(StructCheckNetwork == NULL)
                StructCheckNetwork = "";

            Struct* str = new Struct(
                        StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

            newICD.v_pStructs.append(str);

        }
    }

    newICD.name = strName;
    QString parsedName = strName.c_str();
    parsedName.remove("Icd", Qt::CaseSensitive);
    parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
    newICD.parsedName = parsedName.toStdString();
    newICD.recentIcdDate = recentIcdDate;

    return newICD;
}

void MainWindow::SetupIcdMenu()
{
    ui->listWidget->clear();
    foreach (ICD icd, v_ICDs) {
        ui->listWidget->addItem(QString::fromUtf8(icd.name.c_str()));
    }
    ui->listWidget->sortItems(Qt::AscendingOrder);
    onListIcdClicked(ui->listWidget->item(0));
}

void MainWindow::SetupMessageBrowser()
{
    ui->messageBrowser->clear();
    pubOrSub.clear();
    ui->messageBrowser->setColumnCount(1);
    ui->messageBrowser->setHeaderLabel("Message Names");
    pubOrSub.append(new QTreeWidgetItem((QTreeWidget*)0,
                                         QStringList("Sub")));
    pubOrSub.append(new QTreeWidgetItem((QTreeWidget*)0,
                                         QStringList("Pub")));
    ui->messageBrowser->insertTopLevelItems(0, pubOrSub);
}

void MainWindow::SetupFunctionBrowser()
{
    //update task FunctionList
    ui->taskFunctionList->clear();

    QString dir = cSTDLocation+QString::fromStdString(selectedICD.name)+"/Functions/";

    QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::NoIteratorFlags);

    while (it.hasNext())
    {
        QString temp = it.next();
        temp.remove(dir);
        temp.remove(".xml");
        ui->taskFunctionList->addItem(temp);
    }

    ui->taskFunctionList->sortItems(Qt::AscendingOrder);
}

void MainWindow::SetupTaskflowScene()
{
    taskflowDrawnFunctions.clear();
    taskTitle.clear();
    taskflowSelectedButton = NULL;
    taskflowSelectedFunctionObject = NULL;
    taskComments = "";

    DrawnTaskFunction* startObject = new DrawnTaskFunction;
    startObject->name = "startObject";
    startObject->dir = "";
    taskflowDrawnFunctions.append(startObject);
    QRectF startCircle = QRectF(cTaskflowHorizontalMidpoint+75, taskflowVerticalSpacing, 50, 50);

    taskflowVerticalSpacing += cTaskflowVerticalSpacing;

    taskflowScene->addEllipse(startCircle, QPen(Qt::black, 5), QBrush(Qt::black, Qt::NoBrush));

    DrawnTaskFunction* endObject = new DrawnTaskFunction;
    endObject->name = "endObject";
    endObject->dir = "";
    taskflowDrawnFunctions.append(endObject);
    QRectF endCircle = QRectF(cTaskflowHorizontalMidpoint+75, taskflowVerticalSpacing, 50, 50);

    taskflowVerticalSpacing += cTaskflowVerticalSpacing;

    taskflowScene->addEllipse(endCircle, QPen(Qt::black, 10), QBrush(Qt::black, Qt::NoBrush));

    ui->taskflowGraphicsView->update();

    RedrawTaskflowScene();
}

void MainWindow::SetupContextMenu()
{
    functionDropdownMenu = new QMenu();
    QList<QAction*> v_pfunctionDropdownMenuActions;
    v_pfunctionDropdownMenuActions.append(new QAction("Move Up", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Move Down", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Delete", this));
    QAction *act = new QAction(this);
    act->setSeparator(true);
     v_pfunctionDropdownMenuActions.append(act);
    v_pfunctionDropdownMenuActions.append(new QAction("Accept Change", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Reject Change", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Change to be Assessed", this));
    functionDropdownMenu->addActions(v_pfunctionDropdownMenuActions);
    functionDropdownMenu->setObjectName("functionDropdownMenu");

    foreach(QAction* action, functionDropdownMenu->actions())
    {
        action->setEnabled(false);
    }

    taskflowDropdownMenu = new QMenu();
    QList<QAction*> v_pTaskflowDropdownMenuActions;
    v_pTaskflowDropdownMenuActions.append(new QAction("Move Up", this));
    v_pTaskflowDropdownMenuActions.append(new QAction("Move Down", this));
    v_pTaskflowDropdownMenuActions.append(new QAction("Delete", this));
    v_pTaskflowDropdownMenuActions.append(new QAction("View Function", this));
    taskflowDropdownMenu->addActions(v_pTaskflowDropdownMenuActions);
    taskflowDropdownMenu->setObjectName("taskflowDropdownMenu");

    foreach(QAction* action, taskflowDropdownMenu->actions())
    {
        action->setEnabled(false);
    }
}

void MainWindow::ResetScroll()
{
    int currentindex = ui->tabWidget->currentIndex();

    ui->tabWidget->setCurrentIndex(0);
    onHomeTriggered();
    ui->tabWidget->setCurrentIndex(1);
    onHomeTriggered();
    ui->tabWidget->setCurrentIndex(2);
    onHomeTriggered();

    ui->tabWidget->setCurrentIndex(currentindex);
}

void MainWindow::onScrollEvent(int value)
{
    ui->statusBar->showMessage(QString::number(ui->graphicsView->verticalScrollBar()->value()) + " " + QString::number(ui->graphicsView->horizontalScrollBar()->value()));
}

void MainWindow::DrawUnsortedDiagram(ICD _icd)
{
    const char* strDiagramTitle = _icd.name.c_str();

//    int modelLineLength = _icd.v_pSubscribedMessages.length() +
//            _icd.v_pPublishedMessages.length() * 100;

    icdScene->clear();
    icdScene->update();

    //Diagram Title
    QLabel *DiagramTitle = new QLabel(tr(strDiagramTitle));
    DiagramTitle->setAlignment(Qt::AlignHCenter);
    DiagramTitle->move(10,10);
    QFont f( "Arial", 20, QFont::Bold);
    f.setUnderline(true);
    DiagramTitle->setFont(f);
    icdScene->addWidget(DiagramTitle);

    //Initial drawing
    QString parsedDiagramTitle = strDiagramTitle;

    if(QString::compare(parsedDiagramTitle.left(8),"IcdModel") == 0)
        parsedDiagramTitle.remove(0,8);

    QRectF mainModelRect = QRect(50,50,100,50);
    QPointF mainMidPoint = QLineF(mainModelRect.bottomLeft(),mainModelRect.bottomRight()).pointAt(0.5);
    QLineF mainModelLine = QLineF(mainMidPoint, mainMidPoint);
    mainModelLine.setP2(QPointF(mainMidPoint.x(),mainMidPoint.y()+cClassLineLength));

    QLabel *mainModelName = new QLabel(parsedDiagramTitle);
    int width = mainModelName->fontMetrics().boundingRect(mainModelName->text()).width();
    mainModelName->move(mainMidPoint.x() - width/2, mainMidPoint.y()-30);

    icdScene->addWidget(mainModelName);
    icdScene->addRect(mainModelRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
    icdScene->addLine(mainModelLine, QPen(Qt::black));

    QVector<DrawnModelObject*> drawnModels;
    QVector<DrawnDataObject*> drawnData;
    DrawnModelObject* newModelObject = new DrawnModelObject;
    QString parsedsMsgName;
    QString parsedpMsgName;

    //Draw Subscribed data
    foreach (SubMessage* sMsg, _icd.v_pSubscribedMessages) {

        bool newModelToBeDrawn = true;
        DrawnModelObject* newModelObject = new DrawnModelObject;

        //Validation check
        foreach (DrawnModelObject* drawn, drawnModels) {
            if(strcmp(sMsg->sMicd.c_str(),drawn->name.c_str()) == 0)
                newModelToBeDrawn = false;
        }
        //Draw new Model if needed
        if(newModelToBeDrawn)
        {
            parsedsMsgName = sMsg->sMicd.c_str(); //Chop the 'IcdModel' off

            if(QString::compare(parsedsMsgName.left(8),"IcdModel") == 0)
                parsedsMsgName.remove(0,8);

            QRectF newIcdRect = QRect(horizontalSpacing,50,100,50);

            horizontalSpacing += cHorizontalSpacing;
            CheckUnsortedSceneResize();

            QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
            QLineF modelLine = QLineF(midPoint, midPoint);
            modelLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

            QLabel *newIcdName = new QLabel(parsedsMsgName);
            int width = newIcdName->fontMetrics().boundingRect(newIcdName->text()).width();
            newIcdName->move(midPoint.x() - width/2, midPoint.y()-30);

            icdScene->addLine(modelLine, QPen(Qt::black));
            icdScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
            icdScene->addWidget(newIcdName);

            //Add the New Model Object to the ModelVector
            newModelObject->rect = newIcdRect;
            newModelObject->name = sMsg->sMicd.c_str();
            newModelObject->line = modelLine;
            newModelObject->parsedName = parsedsMsgName.toStdString();
            drawnModels.append(newModelObject);
        }
        //Draw Arrows
        if(newModelObject->name=="") //Check the model we are drawing to/from is not NULL
        {
            foreach (DrawnModelObject* drawn, drawnModels) {
                if(strcmp(sMsg->sMicd.c_str(),drawn->name.c_str())==0)
                     newModelObject = drawn;
            }
        }

        QLineF dataArrow = QLine(newModelObject->line.p1().x(), verticalSpacing, mainModelLine.p1().x(), verticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, false);
        icdScene->addLine(dataArrow);
        icdScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

        verticalSpacing+= 30;
        CheckUnsortedSceneResize();

        //Draw Labels
        QPushButton *newDataName = new QPushButton(tr(sMsg->sMmessage.c_str()));
        newDataName->setFlat(true);
        newDataName->setStyleSheet("background-color: transparent");
        newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
        icdScene->addWidget(newDataName);

        //Add the New Data Object to the DataVector
        DrawnDataObject* newData = new DrawnDataObject();
        newData->arrowPoly = arrowPoly;
        newData->label = newDataName;
        newData->line = dataArrow;
        newData->name = newDataName->text().toStdString();
        newData->pubOrSub = "sub";

        drawnData.append(newData);
    }

    //Draw Published data
    foreach (PubMessage* pMsg, _icd.v_pPublishedMessages) {

        bool newModelToBeDrawn = true;

        parsedpMsgName = pMsg->pMname.c_str();
        parsedpMsgName.remove("Msg", Qt::CaseSensitive);
        parsedpMsgName.remove(parsedDiagramTitle, Qt::CaseSensitive);

        //Validation check
        foreach (DrawnModelObject* drawn, drawnModels) {
//            qDebug() << parsedpMsgName.toStdString().c_str();
//            qDebug() << drawn->parsedName.c_str();
//            qDebug() << strcmp(parsedpMsgName.toStdString().c_str(), drawn->parsedName.c_str());
            if(strcmp(parsedpMsgName.toStdString().c_str(), drawn->parsedName.c_str()) == 0)
            {
                newModelToBeDrawn = false;
                newModelObject = drawn;
            }
        }

        //Draw new Model if needed
        if(newModelToBeDrawn)
        {
//            CheckUnsortedSceneResize();

            QRectF newIcdRect = QRect(horizontalSpacing, 50, 100, 50);

            QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
            QLineF modelLine = QLineF(midPoint, midPoint);
            modelLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

            QLabel *newIcdName = new QLabel(parsedpMsgName);
            width = newIcdName->fontMetrics().boundingRect(newIcdName->text()).width();
            newIcdName->move(midPoint.x() - width/2, midPoint.y()-30);

            icdScene->addLine(modelLine, QPen(Qt::black));
            icdScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
            icdScene->addWidget(newIcdName);

            //Add the New Model Object to the ModelVector
            newModelObject->rect = newIcdRect;
            newModelObject->name = pMsg->pMname.c_str();
            newModelObject->line = modelLine;
            newModelObject->parsedName = parsedpMsgName.toStdString();
            drawnModels.append(newModelObject);

            horizontalSpacing += 150;
            CheckUnsortedSceneResize();
        }

        //Draw Arrows
        if(newModelObject->name == "") //Check the model we are drawing to/from is not NULL
        {
            foreach (DrawnModelObject* drawn, drawnModels) {
                if(strcmp(pMsg->pMname.c_str(),drawn->name.c_str())==0)
                     newModelObject = drawn;
            }
        }

        QLineF dataArrow = QLine(newModelObject->line.p1().x(), verticalSpacing, mainModelLine.p1().x(), verticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, true);
        icdScene->addLine(dataArrow);
        icdScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

        verticalSpacing+= 30;
        CheckUnsortedSceneResize();

        //Draw Labels
        QPushButton *newDataName = new QPushButton(tr(pMsg->pMname.c_str()));
        newDataName->setFlat(true);
        newDataName->setStyleSheet("background-color: transparent");
        newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
        icdScene->addWidget(newDataName);

        //Add the New Data Object to the DataVector
        DrawnDataObject* newData = new DrawnDataObject();
        newData->arrowPoly = arrowPoly;
        newData->label = newDataName;
        newData->line = dataArrow;
        newData->name = newDataName->text().toStdString();
        newData->pubOrSub = "pub";

        drawnData.append(newData);
    }

    ui->graphicsView->update();
}

void MainWindow::SetupDrawingArea()
{
    horizontalSpacing = 200;
    verticalSpacing = 150;
    functionHorizontalSpacing = 350;
    functionVerticalSpacing = 150;
    sceneUnsortedHorizontalSizing = 500;
    sceneUnsortedVerticalSizing = 500;
    sceneFunctionHorizontalSizing = 500;
    sceneFunctionVerticalSizing = 500;
    taskflowVerticalSpacing = 100;
    sceneTaskflowVerticalSizing = 1000;

    if(icdScene)
    {
        icdScene->setSceneRect(0,0,sceneUnsortedHorizontalSizing,sceneUnsortedVerticalSizing);
        ui->graphicsView->setScene(icdScene);
        ui->graphicsView->setBackgroundBrush(Qt::white);

        ui->graphicsView->update();

        /*
        QRectF testRect = QRect(icdScene->width()/2,50,100,50);
        QLineF testLine = QLine(300+(testRect.width()/2),(testRect.bottom()),300+(testRect.width()/2),350);

        QRectF testRect2 = testRect;
        QLineF testLine2 = testLine;

        testRect2.translate(testRect.width()*1.5,0);
        testLine2.translate(testRect.width()*1.5,0);

        QLineF testArrow = QLine(testLine2.p1().x(),200,testLine.p1().x(),200);

        QPolygonF arrowPoly = CreateArrowHead(testArrow,true);

        QLabel *ModelName = new QLabel(tr("ModelName"));
        QLabel *DataName = new QLabel(tr("DataName"));

        ModelName->setAlignment(Qt::AlignHCenter);
        ModelName->move(testRect.x()+10,testRect.center().y());
        DataName->move(testArrow.x2()+20,testArrow.y2()-20);

        icdScene->addWidget(ModelName);
        icdScene->addWidget(DataName);
        icdScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));
        icdScene->addRect(testRect, QPen(Qt::red), QBrush(Qt::red, Qt::NoBrush));
        icdScene->addLine(testLine, QPen(Qt::red));

        icdScene->addRect(testRect2, QPen(Qt::red), QBrush(Qt::red, Qt::NoBrush));
        icdScene->addLine(testLine2, QPen(Qt::red));

        icdScene->addLine(testArrow, QPen(Qt::red));
        */
    }

    if(functionScene)
    {
        functionDrawnModels.clear();
        functionDrawnData.clear();
        ui->functionsGraphicsView->items().clear();
        functionScene = new QGraphicsScene();
        functionScene->setSceneRect(0,0,sceneFunctionHorizontalSizing, sceneFunctionVerticalSizing);
        ui->functionsGraphicsView->setScene(functionScene);
        ui->functionsGraphicsView->setBackgroundBrush(Qt::white);

        //Diagram Title
        const char* strDiagramTitle = selectedICD.name.c_str();
        functionTitle = strDiagramTitle;

        QLabel *DiagramTitle = new QLabel(tr(strDiagramTitle));
        DiagramTitle->setAlignment(Qt::AlignHCenter);
        DiagramTitle->move(10,10);
        QFont f( "Arial", 20, QFont::Bold);
        f.setUnderline(true);
        DiagramTitle->setFont(f);
        functionScene->addWidget(DiagramTitle);

        //Pilot
        QRectF pilotRect = QRect(50,50,100,50);
        QPointF pilotMidPoint = QLineF(pilotRect.bottomLeft(), pilotRect.bottomRight()).pointAt(0.5);
        QLineF pilotClassLine = QLineF(pilotMidPoint, QPointF(pilotMidPoint.x(), pilotMidPoint.y() + cClassLineLength));

        QLabel *pilotLabel = new QLabel(tr("Pilot"));
        int width = pilotLabel->fontMetrics().boundingRect(pilotLabel->text()).width();
        pilotLabel->move(pilotMidPoint.x() - width/2, pilotMidPoint.y()-30);

        functionScene->addEllipse(pilotRect, QPen(Qt::black, 5), QBrush(Qt::black, Qt::NoBrush));
        functionScene->addLine(pilotClassLine, QPen(Qt::black));
        functionScene->addWidget(pilotLabel);

        DrawnModelObject* newModelObject = new DrawnModelObject();
        newModelObject->rect = pilotRect;
        newModelObject->name = pilotLabel->text().toStdString();
        newModelObject->line = pilotClassLine;
        newModelObject->midpoint = pilotMidPoint;
        newModelObject->parsedName = pilotLabel->text().toStdString();
        newModelObject->linkedDataObjects = 99;

        functionDrawnModels.append(newModelObject);

        //MainModel
        QString mainModelTitle = selectedICD.name.c_str(); //Chop the 'IcdModel' off

        if(QString::compare(mainModelTitle.left(8),"IcdModel") == 0)
            mainModelTitle.remove(0,8);

        QRectF mainModelRect = QRect(200,50,100,50);
        QPointF mainMidPoint = QLineF(mainModelRect.bottomLeft(),mainModelRect.bottomRight()).pointAt(0.5);
        QLineF mainModelLine = QLineF(mainMidPoint, mainMidPoint);
        mainModelLine.setP2(QPointF(mainMidPoint.x(),mainMidPoint.y()+cClassLineLength));

        QLabel *mainModelName = new QLabel(mainModelTitle);
        width = mainModelName->fontMetrics().boundingRect(mainModelName->text()).width();
        mainModelName->move(mainMidPoint.x() - width/2, mainMidPoint.y()-30);

        functionScene->addWidget(mainModelName);
        functionScene->addRect(mainModelRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
        functionScene->addLine(mainModelLine, QPen(Qt::black));

        newModelObject = new DrawnModelObject();
        newModelObject->rect = mainModelRect;
        newModelObject->name = strDiagramTitle;
        newModelObject->line = mainModelLine;
        newModelObject->midpoint = mainMidPoint;
        newModelObject->parsedName = mainModelTitle.toStdString();
        functionDrawnModels.append(newModelObject);

        functionSelectedButton = NULL;
        functionSelectedDataObject = NULL;
        toggleFunctionStyleOn = true;
    }

    if(taskflowScene)
    {
        ui->taskflowGraphicsView->items().clear();
        taskflowScene->setSceneRect(0,0,600,700);
        ui->taskflowGraphicsView->setScene(taskflowScene);
        ui->taskflowGraphicsView->setBackgroundBrush(Qt::white);

        taskflowSelectedButton = NULL;
        taskflowSelectedFunctionObject = NULL;
        toggleTaskStyleOn = true;

    }

}

void MainWindow::SaveAllXMLs()
{
    for (int i = 0; i < v_ICDs.size(); i++)
    {
        string fileName = v_ICDs[i].name.c_str();
        fileName += ".xml";
        fileName.insert(0, "/std");
        fileName.insert(0, v_ICDs[i].name.c_str());
        fileName.insert(0,cSTDLocation.toStdString());
        XMLDocument diagram;
        diagram.SaveFile(fileName.c_str()); // Creates the file.
        FILE* pFile;
        pFile = fopen(fileName.c_str(), "w");
        if (pFile == NULL)
            qDebug() << "error opening file";

        XMLPrinter printer(pFile);

        printer.PushComment(v_ICDs[i].name.c_str());
        printer.OpenElement("StdInterface");
        printer.PushAttribute("namespace", "STD");
        printer.PushAttribute("name", v_ICDs[i].name.c_str());
        printer.PushAttribute("recentIcdDate", v_ICDs[i].recentIcdDate.toString(Qt::TextDate).toStdString().c_str());

        printer.OpenElement("Subscribe");
        for(int j = 0; j < v_ICDs[i].v_pSubscribedMessages.size(); j++)
        {
            printer.OpenElement("Message");
            printer.PushAttribute("icd", v_ICDs[i].v_pSubscribedMessages[j]->sMicd.c_str());
            printer.PushAttribute("message", v_ICDs[i].v_pSubscribedMessages[j]->sMmessage.c_str());
            printer.PushAttribute("modified", v_ICDs[i].v_pSubscribedMessages[j]->sMmodified.c_str());
            printer.PushAttribute("accepted", v_ICDs[i].v_pSubscribedMessages[j]->sMaccepted.c_str());
            printer.CloseElement();
        }
        printer.CloseElement();

        for(int j = 0; j < v_ICDs[i].v_pEnums.size(); j++)
        {
            printer.OpenElement("Enum");
            printer.PushAttribute("name" ,v_ICDs[i].v_pEnums[j]->eNname.c_str());
            printer.PushAttribute("comment" ,v_ICDs[i].v_pEnums[j]->eNcomment.c_str());

            for(int k = 0; k < v_ICDs[i].v_pEnums[j]->v_pEnumValues.size(); k++)
            {
                printer.OpenElement("Value");
                printer.PushAttribute("name", v_ICDs[i].v_pEnums[j]->v_pEnumValues[k]->eNValname.c_str());
                printer.PushAttribute("value", v_ICDs[i].v_pEnums[j]->v_pEnumValues[k]->eNValvalue.c_str());
                printer.PushAttribute("comment", v_ICDs[i].v_pEnums[j]->v_pEnumValues[k]->eNValcomment.c_str());
                printer.PushAttribute("modified", v_ICDs[i].v_pEnums[j]->v_pEnumValues[k]->eNValmodified.c_str());
                printer.PushAttribute("accepted", v_ICDs[i].v_pEnums[j]->v_pEnumValues[k]->eNValaccepted.c_str());
                printer.CloseElement();
            }
            printer.CloseElement();
        }

        for(int j = 0; j < v_ICDs[i].v_pStructs.size(); j++)
        {
            printer.OpenElement("Struct");
            printer.PushAttribute("name" ,v_ICDs[i].v_pStructs[j]->sTname.c_str());
            printer.PushAttribute("comment" ,v_ICDs[i].v_pStructs[j]->sTcomment.c_str());

            for(int k = 0; k < v_ICDs[i].v_pStructs[j]->v_pStructParameters.size(); k++)
            {
                printer.OpenElement("Value");
                printer.PushAttribute("name", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPname.c_str());
                printer.PushAttribute("type", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPtype.c_str());
                printer.PushAttribute("unit", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPunit.c_str());
                printer.PushAttribute("default", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPdefault.c_str());
                printer.PushAttribute("min", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPmin.c_str());
                printer.PushAttribute("max", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPmax.c_str());
                printer.PushAttribute("comment", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPcomment.c_str());
                printer.PushAttribute("modified", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPmodified.c_str());
                printer.PushAttribute("accepted", v_ICDs[i].v_pStructs[j]->v_pStructParameters[k]->sPaccepted.c_str());
                printer.CloseElement();
            }
            printer.CloseElement();
        }

        for(int j = 0; j < v_ICDs[i].v_pPublishedMessages.size(); j++)
        {
            printer.OpenElement("Message");
            printer.PushAttribute("name", v_ICDs[i].v_pPublishedMessages[j]->pMname.c_str());
            printer.PushAttribute("comment", v_ICDs[i].v_pPublishedMessages[j]->pMcomment.c_str());
            printer.PushAttribute("network", v_ICDs[i].v_pPublishedMessages[j]->pMnetwork.c_str());
            printer.PushAttribute("modified", v_ICDs[i].v_pPublishedMessages[j]->pMmodified.c_str());
            printer.PushAttribute("accepted", v_ICDs[i].v_pPublishedMessages[j]->pMaccepted.c_str());

            for(int k = 0; k < v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters.size(); k++)
            {
                printer.OpenElement("Parameter");
                printer.PushAttribute("name", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMname.c_str());
                printer.PushAttribute("type", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMtype.c_str());
                printer.PushAttribute("unit", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMunit.c_str());
                printer.PushAttribute("default", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMdefault.c_str());
                printer.PushAttribute("min", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmin.c_str());
                printer.PushAttribute("max", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmax.c_str());
                printer.PushAttribute("comment", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMcomment.c_str());
                printer.PushAttribute("modified", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmodified.c_str());
                printer.PushAttribute("accepted", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMaccepted.c_str());
                printer.PushAttribute("ranking", v_ICDs[i].v_pPublishedMessages[j]->v_pPubMparameters[k]->pMranking);
                printer.CloseElement();
            }

            printer.CloseElement();
        }

        printer.CloseElement();
        diagram.Print(&printer);
        fclose(pFile);

        qDebug() <<  "save done";
        ui->statusBar->showMessage("Save Completed");

    }
}

void MainWindow::SaveOneXML(ICD _icd)
{
    string fileName = _icd.name.c_str();
    fileName += ".xml";
    fileName.insert(0, "/std");
    fileName.insert(0, _icd.name.c_str());
    fileName.insert(0,cSTDLocation.toStdString());
    XMLDocument diagram;
    diagram.SaveFile(fileName.c_str()); // Creates the file.
    FILE* pFile;
    pFile = fopen(fileName.c_str(), "w");
    if (pFile == NULL)
        qDebug() << "error opening file";

    XMLPrinter printer(pFile);

    printer.PushComment(_icd.name.c_str());
    printer.OpenElement("StdInterface");
    printer.PushAttribute("namespace", "STD");
    printer.PushAttribute("name", _icd.name.c_str());
    printer.PushAttribute("recentIcdDate", _icd.recentIcdDate.toString(Qt::TextDate).toStdString().c_str());

    printer.OpenElement("Subscribe");
    for(int j = 0; j < _icd.v_pSubscribedMessages.size(); j++)
    {
        printer.OpenElement("Message");
        printer.PushAttribute("icd", _icd.v_pSubscribedMessages[j]->sMicd.c_str());
        printer.PushAttribute("message", _icd.v_pSubscribedMessages[j]->sMmessage.c_str());
        printer.PushAttribute("modified", _icd.v_pSubscribedMessages[j]->sMmodified.c_str());
        printer.PushAttribute("accepted", _icd.v_pSubscribedMessages[j]->sMaccepted.c_str());
        printer.CloseElement();
    }
    printer.CloseElement();

    for(int j = 0; j < _icd.v_pEnums.size(); j++)
    {
        printer.OpenElement("Enum");
        printer.PushAttribute("name" ,_icd.v_pEnums[j]->eNname.c_str());
        printer.PushAttribute("comment" ,_icd.v_pEnums[j]->eNcomment.c_str());

        for(int k = 0; k < _icd.v_pEnums[j]->v_pEnumValues.size(); k++)
        {
            printer.OpenElement("Value");
            printer.PushAttribute("name", _icd.v_pEnums[j]->v_pEnumValues[k]->eNValname.c_str());
            printer.PushAttribute("value", _icd.v_pEnums[j]->v_pEnumValues[k]->eNValvalue.c_str());
            printer.PushAttribute("comment", _icd.v_pEnums[j]->v_pEnumValues[k]->eNValcomment.c_str());
            printer.PushAttribute("modified", _icd.v_pEnums[j]->v_pEnumValues[k]->eNValmodified.c_str());
            printer.PushAttribute("accepted", _icd.v_pEnums[j]->v_pEnumValues[k]->eNValaccepted.c_str());
            printer.CloseElement();
        }
        printer.CloseElement();
    }

    for(int j = 0; j < _icd.v_pStructs.size(); j++)
    {
        printer.OpenElement("Struct");
        printer.PushAttribute("name" ,_icd.v_pStructs[j]->sTname.c_str());
        printer.PushAttribute("comment" ,_icd.v_pStructs[j]->sTcomment.c_str());

        for(int k = 0; k < _icd.v_pStructs[j]->v_pStructParameters.size(); k++)
        {
            printer.OpenElement("Value");
            printer.PushAttribute("name", _icd.v_pStructs[j]->v_pStructParameters[k]->sPname.c_str());
            printer.PushAttribute("type", _icd.v_pStructs[j]->v_pStructParameters[k]->sPtype.c_str());
            printer.PushAttribute("unit", _icd.v_pStructs[j]->v_pStructParameters[k]->sPunit.c_str());
            printer.PushAttribute("default", _icd.v_pStructs[j]->v_pStructParameters[k]->sPdefault.c_str());
            printer.PushAttribute("min", _icd.v_pStructs[j]->v_pStructParameters[k]->sPmin.c_str());
            printer.PushAttribute("max", _icd.v_pStructs[j]->v_pStructParameters[k]->sPmax.c_str());
            printer.PushAttribute("comment", _icd.v_pStructs[j]->v_pStructParameters[k]->sPcomment.c_str());
            printer.PushAttribute("modified", _icd.v_pStructs[j]->v_pStructParameters[k]->sPmodified.c_str());
            printer.PushAttribute("accepted", _icd.v_pStructs[j]->v_pStructParameters[k]->sPaccepted.c_str());
            printer.CloseElement();
        }
        printer.CloseElement();
    }


    for(int j = 0; j < _icd.v_pPublishedMessages.size(); j++)
    {
        printer.OpenElement("Message");
        printer.PushAttribute("name", _icd.v_pPublishedMessages[j]->pMname.c_str());
        printer.PushAttribute("comment", _icd.v_pPublishedMessages[j]->pMcomment.c_str());
        printer.PushAttribute("network", _icd.v_pPublishedMessages[j]->pMnetwork.c_str());
        printer.PushAttribute("modified", _icd.v_pPublishedMessages[j]->pMmodified.c_str());
        printer.PushAttribute("accepted", _icd.v_pPublishedMessages[j]->pMaccepted.c_str());

        for(int k = 0; k < _icd.v_pPublishedMessages[j]->v_pPubMparameters.size(); k++)
        {
            printer.OpenElement("Parameter");
            printer.PushAttribute("name", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMname.c_str());
            printer.PushAttribute("type", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMtype.c_str());
            printer.PushAttribute("unit", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMunit.c_str());
            printer.PushAttribute("default", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMdefault.c_str());
            printer.PushAttribute("min", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmin.c_str());
            printer.PushAttribute("max", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmax.c_str());
            printer.PushAttribute("comment", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMcomment.c_str());
            printer.PushAttribute("modified", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMmodified.c_str());
            printer.PushAttribute("accepted", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMaccepted.c_str());
            printer.PushAttribute("ranking", _icd.v_pPublishedMessages[j]->v_pPubMparameters[k]->pMranking);
            printer.CloseElement();
        }

        printer.CloseElement();
    }

    printer.CloseElement();
    diagram.Print(&printer);
    fclose(pFile);

    qDebug() <<  "save done";
    ui->statusBar->showMessage("Save Completed");
}

void MainWindow::SetupFileDirectories()
{
    foreach (ICD _icd, v_ICDs) {
        QDir dir(cIcdLocation+QString::fromStdString("/STDs/"+_icd.name));
        if (!dir.exists()) {
            dir.mkpath(".");
            dir.mkpath("Functions");
            dir.mkpath("Tasks");
            dir.mkpath("Diagrams");
        }
    }
}

QPolygonF MainWindow::CreateArrowHead(QLineF arrowLine, bool rightPointing)
{
    //const int arrowHeadWidth = 5;

    QPointF arrowPoint;
    QPointF arrowRight;
    QPointF arrowLeft;

    if(rightPointing)
    {
        arrowPoint = QPointF(arrowLine.p2());
        arrowRight = QPointF(arrowPoint.x()+10,arrowPoint.y()-5);
        arrowLeft = QPointF(arrowPoint.x()+10,arrowPoint.y()+5);
    }
    else
    {
        arrowPoint = QPointF(arrowLine.p1());
        arrowRight = QPointF(arrowPoint.x()-10,arrowPoint.y()+5);
        arrowLeft = QPointF(arrowPoint.x()-10,arrowPoint.y()-5);
    }
    QVector<QPointF> arrowHeadVector;
    arrowHeadVector.append(arrowPoint);
    arrowHeadVector.append(arrowRight);
    arrowHeadVector.append(arrowLeft);
    QPolygonF arrowPoly = QPolygonF(arrowHeadVector);

    return arrowPoly;
}

QPolygonF MainWindow::CreateTaskArrowHead(QLineF arrowLine)
{
    QPointF arrowPoint;
    QPointF arrowRight;
    QPointF arrowLeft;

    arrowPoint = QPointF(arrowLine.p2());
    arrowRight = QPointF(arrowPoint.x()+5,arrowPoint.y()-10);
    arrowLeft = QPointF(arrowPoint.x()-5,arrowPoint.y()-10);

    QVector<QPointF> arrowHeadVector;
    arrowHeadVector.append(arrowPoint);
    arrowHeadVector.append(arrowRight);
    arrowHeadVector.append(arrowLeft);
    QPolygonF arrowPoly = QPolygonF(arrowHeadVector);

    return arrowPoly;
}

void MainWindow::CheckFunctionSceneResize()
{
    if(functionHorizontalSpacing + 150 > sceneFunctionHorizontalSizing)
    {
        sceneFunctionHorizontalSizing += cSceneSizeIncrement;
    }
    if(functionVerticalSpacing > sceneFunctionVerticalSizing)
    {
        sceneFunctionVerticalSizing += cSceneSizeIncrement;
    }
    functionScene->setSceneRect(0,0,sceneFunctionHorizontalSizing,sceneFunctionVerticalSizing);
}

void MainWindow::CheckUnsortedSceneResize()
{
    if(horizontalSpacing + 150 > sceneUnsortedHorizontalSizing)
    {
        sceneUnsortedHorizontalSizing += cSceneSizeIncrement;
    }
    if(verticalSpacing > sceneUnsortedVerticalSizing)
    {
        sceneUnsortedVerticalSizing += cSceneSizeIncrement;
    }
    icdScene->setSceneRect(0,0,sceneUnsortedHorizontalSizing,sceneUnsortedVerticalSizing);
}

void MainWindow::CheckTaskflowSceneResize()
{
    if(taskflowVerticalSpacing + 300 > sceneTaskflowVerticalSizing)
    {
        sceneTaskflowVerticalSizing += 600;
    }
    taskflowScene->setSceneRect(0,0,600,sceneTaskflowVerticalSizing);
}

void MainWindow::onHorizontalSpacing()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("Enter your new horizontal spacing value"),
                                    tr("Spacing:"), cHorizontalSpacing, 50, 500, 5, &ok);
    if (ok)
        cHorizontalSpacing = i;

    RedrawFunctionScene();
}

void MainWindow::onVerticalSpacing()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("Enter your new vertical spacing value"),
                                    tr("Spacing:"), cVerticalSpacing, 10, 100, 2, &ok);
    if (ok)
        cVerticalSpacing = i;

    RedrawFunctionScene();
}

void MainWindow::onTaskVerticalSpacing()
{
    bool ok;
    int i = QInputDialog::getInt(this, tr("Enter your new vertical spacing value"),
                                    tr("Spacing:"), cTaskflowVerticalSpacing, 10, 300, 5, &ok);
    if (ok)
        cTaskflowVerticalSpacing = i;

    RedrawTaskflowScene();
}

void MainWindow::onFunctionComments()
{
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, tr("Function Comments"),
                                                  tr("Name\tComment:"), functionComments, &ok);

    if (ok && !text.isEmpty())
        functionComments = text;
}

void MainWindow::onTaskComments()
{
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, tr("Task Comments"),
                                                  tr("Name\tComment:"), taskComments, &ok);

    if (ok && !text.isEmpty())
        taskComments = text;
}

void MainWindow::onAcceptAllMods()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Accept all Modifications",
                                                                tr("Are you sure you wish to reject every modification in this ICD? \n"
                                                                   "This will overwrite any previous status"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        foreach (SubMessage* sm, selectedICD.v_pSubscribedMessages) {
            sm->sMaccepted = aAcceptedStatus[eAccepted];
        }
        foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
            pm->pMaccepted = aAcceptedStatus[eAccepted];
            foreach (MessageParameter* mp, pm->v_pPubMparameters) {
                mp->pMaccepted = aAcceptedStatus[eAccepted];
            }
        }
        foreach (Enum* en, selectedICD.v_pEnums) {
            foreach (EnumValue* env, en->v_pEnumValues) {
                env->eNValaccepted = aAcceptedStatus[eAccepted];
            }
        }
        foreach (Struct* str, selectedICD.v_pStructs) {
            foreach (StructParameter* strp, str->v_pStructParameters) {
                strp->sPaccepted = aAcceptedStatus[eAccepted];
            }
        }
    }

    SaveOneXML(selectedICD);
}

void MainWindow::onRejectAllMods() {
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Reject all Modifications",
                                                                tr("Are you sure you wish to reject every modification in this ICD? \n"
                                                                   "This will overwrite any previous status"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        foreach (SubMessage* sm, selectedICD.v_pSubscribedMessages) {
            sm->sMaccepted = aAcceptedStatus[eRejected];
        }
        foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
            pm->pMaccepted = aAcceptedStatus[eRejected];
            foreach (MessageParameter* mp, pm->v_pPubMparameters) {
                mp->pMaccepted = aAcceptedStatus[eRejected];
            }
        }
        foreach (Enum* en, selectedICD.v_pEnums) {
            foreach (EnumValue* env, en->v_pEnumValues) {
                env->eNValaccepted = aAcceptedStatus[eRejected];
            }
        }
        foreach (Struct* str, selectedICD.v_pStructs) {
            foreach (StructParameter* strp, str->v_pStructParameters) {
                strp->sPaccepted = aAcceptedStatus[eRejected];
            }
        }
    }

    SaveOneXML(selectedICD);
}

void MainWindow::RedrawFunctionScene()
{
    pilotModelObject = functionDrawnModels[0];
    targetModelObject = functionDrawnModels[1];

    functionHorizontalSpacing = 50;
    functionVerticalSpacing = 150;
    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;
    sceneFunctionHorizontalSizing = 500;
    sceneFunctionVerticalSizing = 500;

    functionScene->clear();
    functionScene->clear();
    functionScene->clear();
    functionScene = new QGraphicsScene();
    functionScene->setSceneRect(0,0,sceneFunctionHorizontalSizing, sceneFunctionVerticalSizing);
    ui->functionsGraphicsView->setScene(functionScene);
    ui->functionsGraphicsView->setBackgroundBrush(Qt::white);

    //Diagram Title
//    const char* strDiagramTitle = functionTitle.toStdString().c_str();

    QLabel *DiagramTitle = new QLabel(tr(functionTitle.toStdString().c_str()));
    DiagramTitle->setAlignment(Qt::AlignHCenter);
    DiagramTitle->move(10,10);
    QFont f( "Arial", 20, QFont::Bold);
    f.setUnderline(true);
    DiagramTitle->setFont(f);
    functionScene->addWidget(DiagramTitle);

    foreach (DrawnModelObject* dmo, functionDrawnModels) { 

        dmo->rect.setY(50);
        dmo->rect.setX(functionHorizontalSpacing);
        dmo->rect.setWidth(100);
        dmo->rect.setHeight(50);
        QPointF midPoint = QLineF(dmo->rect.bottomLeft(),dmo->rect.bottomRight()).pointAt(0.5);
        QLineF classLine = QLineF(midPoint, QPointF(midPoint.x(), midPoint.y() + cClassLineLength));
        dmo->midpoint = midPoint;
        dmo->line = classLine;

        if(dmo->name=="Pilot")
            functionScene->addEllipse(dmo->rect, QPen(Qt::black, 5), QBrush(Qt::black, Qt::NoBrush));
        else
            functionScene->addRect(dmo->rect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));

        functionScene->addLine(dmo->line, QPen(Qt::black));
        QLabel* tempLabel = new QLabel(dmo->parsedName.c_str());
        int width = tempLabel->fontMetrics().boundingRect(tempLabel->text()).width();
        tempLabel->move(dmo->midpoint.x() - width/2, dmo->midpoint.y()-30);
        functionScene->addWidget(tempLabel);

        functionHorizontalSpacing += cHorizontalSpacing;
        CheckFunctionSceneResize();
    }

    foreach (DrawnDataObject* doo, functionDrawnData) {

        if(doo->pubOrSub == "pub"){
            doo->line = QLine(doo->model->line.p2().x(), functionVerticalSpacing, targetModelObject->line.p1().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, false);
        }
        else if(doo->pubOrSub == "sub"){
            doo->line = QLine(doo->model->line.p2().x(), functionVerticalSpacing, targetModelObject->line.p1().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, true);
        }
        else if(doo->pubOrSub == "pilot"){
            doo->line = QLine(doo->model->line.p1().x(), functionVerticalSpacing, pilotModelObject->line.p1().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, false);
        }

        functionScene->addLine(doo->line, QPen(Qt::black));
        QPushButton* tempLabel = new QPushButton(doo->name.c_str());
        tempLabel->move(doo->line.p2().x()+15,doo->line.p2().y()-25);
        tempLabel->setFlat(true);

        if(toggleFunctionStyleOn)
        {
            if(doo->modified == aModifiedStatus[eNotModified])
                tempLabel->setStyleSheet("color: rgba(0,0,0,1.0)");
            else if(doo->modified == aModifiedStatus[eRemoved])
                tempLabel->setStyleSheet("color: rgba(255,0,0,1.0)");
            else if(doo->modified == aModifiedStatus[eAdded])
                tempLabel->setStyleSheet("color: rgba(0,255,0,1.0)");
            else if(doo->modified == aModifiedStatus[eModified])
                tempLabel->setStyleSheet("color: rgba(0,0,255,1.0)");
            else
                tempLabel->setStyleSheet("color: rgba(0,0,0,1.0)");

            if(doo->accepted == aAcceptedStatus[eAccepted])
                tempLabel->setStyleSheet("background-color: rgba(0,255,0,0.3)");
            else if(doo->accepted == aAcceptedStatus[eRejected])
                tempLabel->setStyleSheet("background-color: rgba(255,0,0,0.3)");
            else if(doo->accepted == aAcceptedStatus[eToBeAssessed])
                tempLabel->setStyleSheet("background-color: rgba(192,192,192,0.3)");
        }
        else
        {
            tempLabel->setStyleSheet("color: rgba(0,0,0,1.0)");
            tempLabel->setStyleSheet("background-color: transparent");
        }

        functionScene->addWidget(tempLabel);
        functionScene->addPolygon(doo->arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));
        doo->label = tempLabel;

        connect(tempLabel, SIGNAL(clicked()),
                    this, SLOT(onDataObjectClicked()));

        functionVerticalSpacing += cVerticalSpacing;
        CheckFunctionSceneResize();
    }
}

void MainWindow::ResetFunctionScene()
{
    functionHorizontalSpacing = 350;
    functionVerticalSpacing = 150;
    sceneFunctionHorizontalSizing = 500;
    sceneFunctionVerticalSizing = 500;

    functionDrawnModels.clear();
    functionDrawnData.clear();
    ui->functionsGraphicsView->items().clear();
    functionScene = new QGraphicsScene();
    functionScene->setSceneRect(0,0,sceneFunctionHorizontalSizing, sceneFunctionVerticalSizing);
    ui->functionsGraphicsView->setScene(functionScene);
    ui->functionsGraphicsView->setBackgroundBrush(Qt::white);

    //Diagram Title
    const char* strDiagramTitle = selectedICD.name.c_str();
    functionTitle = strDiagramTitle;

    QLabel *DiagramTitle = new QLabel(tr(strDiagramTitle));
    DiagramTitle->setAlignment(Qt::AlignHCenter);
    DiagramTitle->move(10,10);
    QFont f( "Arial", 20, QFont::Bold);
    f.setUnderline(true);
    DiagramTitle->setFont(f);
    functionScene->addWidget(DiagramTitle);

    //Pilot
    QRectF pilotRect = QRect(50,50,100,50);
    QPointF pilotMidPoint = QLineF(pilotRect.bottomLeft(), pilotRect.bottomRight()).pointAt(0.5);
    QLineF pilotClassLine = QLineF(pilotMidPoint, QPointF(pilotMidPoint.x(), pilotMidPoint.y() + cClassLineLength));

    QLabel *pilotLabel = new QLabel(tr("Pilot"));
    int width = pilotLabel->fontMetrics().boundingRect(pilotLabel->text()).width();
    pilotLabel->move(pilotMidPoint.x() - width/2, pilotMidPoint.y()-30);

    functionScene->addEllipse(pilotRect, QPen(Qt::black, 5), QBrush(Qt::black, Qt::NoBrush));
    functionScene->addLine(pilotClassLine, QPen(Qt::black));
    functionScene->addWidget(pilotLabel);

    DrawnModelObject* newModelObject = new DrawnModelObject();
    newModelObject->rect = pilotRect;
    newModelObject->name = pilotLabel->text().toStdString();
    newModelObject->line = pilotClassLine;
    newModelObject->midpoint = pilotMidPoint;
    newModelObject->parsedName = pilotLabel->text().toStdString();
    newModelObject->linkedDataObjects = 99;

    functionDrawnModels.append(newModelObject);

    //MainModel
    QString mainModelTitle = selectedICD.name.c_str(); //Chop the 'IcdModel' off

    if(QString::compare(mainModelTitle.left(8),"IcdModel") == 0)
        mainModelTitle.remove(0,8);

    QRectF mainModelRect = QRect(200,50,100,50);
    QPointF mainMidPoint = QLineF(mainModelRect.bottomLeft(),mainModelRect.bottomRight()).pointAt(0.5);
    QLineF mainModelLine = QLineF(mainMidPoint, mainMidPoint);
    mainModelLine.setP2(QPointF(mainMidPoint.x(),mainMidPoint.y()+cClassLineLength));

    QLabel *mainModelName = new QLabel(mainModelTitle);
    width = mainModelName->fontMetrics().boundingRect(mainModelName->text()).width();
    mainModelName->move(mainMidPoint.x() - width/2, mainMidPoint.y()-30);

    functionScene->addWidget(mainModelName);
    functionScene->addRect(mainModelRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
    functionScene->addLine(mainModelLine, QPen(Qt::black));

    newModelObject = new DrawnModelObject();
    newModelObject->rect = mainModelRect;
    newModelObject->name = strDiagramTitle;
    newModelObject->line = mainModelLine;
    newModelObject->midpoint = mainMidPoint;
    newModelObject->parsedName = mainModelTitle.toStdString();
    functionDrawnModels.append(newModelObject);

    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;
}

void MainWindow::SaveFunctionScene()
{
    if(functionTitle == QString::fromStdString(selectedICD.name))
        onRenameFunction();
    if(functionTitle == QString::fromStdString(selectedICD.name))
    {
        ui->statusBar->showMessage("ERROR: Save Cancelled, Function must be renamed");
        return;
    }

    QString saveDirectory = cSTDLocation + QString::fromStdString(selectedICD.name) +"/Functions/";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Directory"),
                                                    saveDirectory,
                                                    tr("XML files (*.xml)"));

    //Cancel Catch
    if(fileName == NULL)
    {
        ui->statusBar->showMessage("ERROR: Save Cancelled");
        return;
    }

    fileName.remove(".xml");
    fileName += ".xml";
    QString fullDir = fileName;
    fileName.remove(saveDirectory);
    XMLDocument diagram;
    diagram.SaveFile(fullDir.toStdString().c_str());// Creates the file.
    FILE* pFile;
    pFile = fopen(fullDir.toStdString().c_str(), "w");
    if (pFile == NULL)
        qDebug() << "error opening file";

    XMLPrinter printer(pFile);

    printer.PushComment(selectedICD.name.c_str());
    printer.OpenElement("FunctionTitle");
    printer.PushAttribute("name", functionTitle.toStdString().c_str());

    printer.OpenElement("Models");
    foreach (DrawnModelObject* dmo, functionDrawnModels) {
        printer.OpenElement("ModelObject");
        printer.PushAttribute("name", dmo->name.c_str());
        printer.PushAttribute("parsedName", dmo->parsedName.c_str());
        printer.CloseElement();
    }
    printer.CloseElement();

    printer.OpenElement("Data");
    foreach (DrawnDataObject* ddo, functionDrawnData) {
        printer.OpenElement("DataObject");
        printer.PushAttribute("name", ddo->name.c_str());
        printer.PushAttribute("model", ddo->model->name.c_str());
        printer.PushAttribute("pubOrSub", ddo->pubOrSub.c_str());

        printer.CloseElement();
    }
    printer.CloseElement();

    printer.OpenElement("Comments");
    printer.PushAttribute("comments", functionComments.toStdString().c_str());
    printer.CloseElement();

    printer.CloseElement();

    diagram.Print(&printer);
    fclose(pFile);

    ui->statusBar->showMessage("Saving Complete");
    SetupFunctionBrowser();
}

void MainWindow::LoadFunctionScene()
{
    //Clear everything out
    functionDrawnModels.clear();
    functionDrawnData.clear();
    functionTitle.clear();
    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    cSTDLocation + QString::fromStdString(selectedICD.name) +"/Functions/",
                                                    tr("XML files (*.xml)"));
    //Cancel Catch
    if(fileName == NULL)
    {
        ui->statusBar->showMessage("ERROR: Load Cancelled");
        return;
    }

    XMLDocument doc;
    doc.LoadFile( fileName.toStdString().c_str() );
    XMLElement* root = doc.FirstChildElement();
    functionTitle = root->Attribute("name");

    for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
    {
        if(strcmp(element->Name(), "Models") == 0)
        {
            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                DrawnModelObject* dmo = new DrawnModelObject();
                string dmoName = elementSubscribe->Attribute("name");
                string dmoParsedName = elementSubscribe->Attribute("parsedName");

                dmo->name = dmoName;
                dmo->parsedName= dmoParsedName;

                functionDrawnModels.append(dmo);
            }
        }

        if(strcmp(element->Name(), "Data") == 0)
        {

            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                DrawnDataObject* ddo = new DrawnDataObject();
                string ddoName = elementSubscribe->Attribute("name");
                string ddoModel = elementSubscribe->Attribute("model");
                string ddoPubOrSub = elementSubscribe->Attribute("pubOrSub");

                ddo->name = ddoName;
                foreach (DrawnModelObject* dmo, functionDrawnModels) {
                    if(ddoModel == dmo->name)
                        ddo->model = dmo;
                }

                ddo->pubOrSub = ddoPubOrSub;

                functionDrawnData.append(ddo);
            }
        }

        if(strcmp(element->Name(), "Comments") == 0)
        {
                string comment = element->Attribute("comments");

                functionComments = QString::fromStdString(comment);
        }
    }

    //Get current modified status of each variable from the respective ICD
    ICD stdICD = ParseSingleStdIcd(cSTDLocation + QString::fromStdString(selectedICD.name) +
                                   "/std" + QString::fromStdString(selectedICD.name) + ".xml");

    //ATTENTION : A thing of beauty
    foreach (DrawnDataObject* ddo, functionDrawnData)
    {
        if(ddo->pubOrSub == "sub")
        {
            foreach (SubMessage* sm, stdICD.v_pSubscribedMessages)
            {
                if(sm->sMmessage == ddo->name)
                {
                    ddo->modified = sm->sMmodified;
                    ddo->accepted = sm->sMaccepted;
                }
            }
        }
        else if(ddo->pubOrSub == "pub")
        {
            foreach (PubMessage* pm, stdICD.v_pPublishedMessages)
            {
                if(pm->pMname == ddo->name)
                {
                    ddo->modified = pm->pMmodified;
                    ddo->accepted = pm->pMaccepted;
                }
                else
                {
                    foreach (MessageParameter* mp, pm->v_pPubMparameters)
                    {
                        if(mp->pMname == ddo->name)
                        {
                            ddo->modified = mp->pMmodified;
                            ddo->accepted = mp->pMaccepted;
                        }
                    }
                }
            }
        }
    }

    ui->statusBar->showMessage("Loading Complete");
    RedrawFunctionScene();
    onHomeTriggered();
}

void MainWindow::LoadCustomFunctionScene(QString _fileName)
{
    //Clear everything out
    functionDrawnModels.clear();
    functionDrawnData.clear();
    functionTitle.clear();
    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;

    QString fileName = _fileName;

    XMLDocument doc;
    doc.LoadFile( fileName.toStdString().c_str() );
    XMLElement* root = doc.FirstChildElement();
    functionTitle = root->Attribute("name");

    for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
    {
        if(strcmp(element->Name(), "Models") == 0)
        {
            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                DrawnModelObject* dmo = new DrawnModelObject();
                string dmoName = elementSubscribe->Attribute("name");
                string dmoParsedName = elementSubscribe->Attribute("parsedName");

                dmo->name = dmoName;
                dmo->parsedName= dmoParsedName;

                functionDrawnModels.append(dmo);
            }
        }

        if(strcmp(element->Name(), "Data") == 0)
        {

            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                DrawnDataObject* ddo = new DrawnDataObject();
                string ddoName = elementSubscribe->Attribute("name");
                string ddoModel = elementSubscribe->Attribute("model");
                string ddoPubOrSub = elementSubscribe->Attribute("pubOrSub");

                ddo->name = ddoName;
                foreach (DrawnModelObject* dmo, functionDrawnModels) {
                    if(ddoModel == dmo->name)
                        ddo->model = dmo;
                }

                ddo->pubOrSub = ddoPubOrSub;

                functionDrawnData.append(ddo);
            }
        }

    }

    //Get current modified status of each variable from the respective ICD
    ICD stdICD = ParseSingleStdIcd(cSTDLocation + QString::fromStdString(selectedICD.name) +
                                   "/std" + QString::fromStdString(selectedICD.name) + ".xml");

    //ATTENTION : A thing of beauty
    foreach (DrawnDataObject* ddo, functionDrawnData)
    {
        if(ddo->pubOrSub == "sub")
        {
            foreach (SubMessage* sm, stdICD.v_pSubscribedMessages)
            {
                if(sm->sMmessage == ddo->name)
                {
                    ddo->modified = sm->sMmodified;
                    ddo->accepted = sm->sMaccepted;
                }
            }
        }
        else if(ddo->pubOrSub == "pub")
        {
            foreach (PubMessage* pm, stdICD.v_pPublishedMessages)
            {
                if(pm->pMname == ddo->name)
                {
                    ddo->modified = pm->pMmodified;
                    ddo->accepted = pm->pMaccepted;
                }
                else
                {
                    foreach (MessageParameter* mp, pm->v_pPubMparameters)
                    {
                        if(mp->pMname == ddo->name)
                        {
                            ddo->modified = mp->pMmodified;
                            ddo->accepted = mp->pMaccepted;
                        }
                    }
                }
            }
        }
    }

    ui->statusBar->showMessage("Loading Complete");
    RedrawFunctionScene();
    onHomeTriggered();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "MainWindow",
                                                                tr("Do you want to save your work?\n"
                                                                   "Joey Ryan accepts no responsibility for loss of work (but does enjoy your pain)"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn == QMessageBox::Cancel) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::onHomeTriggered()
{
    if(ui->tabWidget->currentIndex() == 0)
    {
        ui->graphicsView->horizontalScrollBar()->setValue(0);
        ui->graphicsView->verticalScrollBar()->setValue(0);
    }
    else if(ui->tabWidget->currentIndex() == 1)
    {
        ui->functionsGraphicsView->horizontalScrollBar()->setValue(0);
        ui->functionsGraphicsView->verticalScrollBar()->setValue(0);
    }
    else if(ui->tabWidget->currentIndex() == 2)
    {
        ui->taskflowGraphicsView->horizontalScrollBar()->setValue(0);
        ui->taskflowGraphicsView->verticalScrollBar()->setValue(0);
    }
}

void MainWindow::onListIcdClicked(QListWidgetItem* _item)
{
    horizontalSpacing = 200;
    verticalSpacing = 150;
    functionHorizontalSpacing = 350;
    functionVerticalSpacing = 150;
    sceneUnsortedHorizontalSizing = 500;
    sceneUnsortedVerticalSizing = 500;
    sceneFunctionHorizontalSizing = 500;
    sceneFunctionVerticalSizing = 500;
    functionComments = "";

    //find the icd and draw its unsorted diagram
    const char* selectedEntry = _item->text().toUtf8().constData();
    foreach (ICD icd, v_ICDs) {
        if(strcmp(selectedEntry, icd.name.c_str()) == 0)
        {
            selectedICD = icd;
            break;
        }
    }
//    ui->graphicsView->horizontalScrollBar()->setValue(0);
//    ui->graphicsView->verticalScrollBar()->setValue(0);
//    ui->graphicsView->update();

    //update Function msg browser
    subItems.clear(); //MEMORY LEAKS
    pubItems.clear(); //MEMORY LEAKS
    SetupMessageBrowser();
//    ui->messageBrowser->topLevelItem(0)->takeChildren();
//    ui->messageBrowser->topLevelItem(1)->takeChildren();

    foreach (SubMessage* subMsg, selectedICD.v_pSubscribedMessages) {
        QString IcdName = QString::fromStdString(subMsg->sMicd);
        foreach (ICD _icd, v_ICDs) {
            if(_icd.name.compare(IcdName.toStdString()) == 0)
            {
                foreach (PubMessage* pubMsg, _icd.v_pPublishedMessages) {
                    if(pubMsg->pMname.compare(subMsg->sMmessage) == 0)
                    {
                        QTreeWidgetItem* temp = new QTreeWidgetItem((QTreeWidget*)0,
                                                                 QStringList(QString::fromStdString(pubMsg->pMname)));
                        foreach (MessageParameter* param, pubMsg->v_pPubMparameters) {
                            temp->addChild(new QTreeWidgetItem((QTreeWidget*)0,
                                                               QStringList(QString::fromStdString(param->pMname))));
                        }
                        subItems.append(temp);
                        break;
                    }
                }
            }
        }
    }
    foreach (PubMessage* pubMsg, selectedICD.v_pPublishedMessages) {
        QTreeWidgetItem* temp = new QTreeWidgetItem((QTreeWidget*)0,
                                                 QStringList(QString::fromStdString(pubMsg->pMname)));
        foreach (MessageParameter* param, pubMsg->v_pPubMparameters) {
            temp->addChild(new QTreeWidgetItem((QTreeWidget*)0,
                                               QStringList(QString::fromStdString(param->pMname))));
        }
        pubItems.append(temp);
    }

    ui->messageBrowser->topLevelItem(0)->addChildren(subItems);
    ui->messageBrowser->topLevelItem(1)->addChildren(pubItems);

    ui->messageBrowser->sortItems(0, Qt::AscendingOrder);

    SetupDrawingArea();
    ResetScroll();
    DrawUnsortedDiagram(selectedICD);
    SetupFunctionBrowser();

}

void MainWindow::onAddDataExchangeClicked()
{
    if(ui->messageBrowser->currentItem()== NULL)
    {
        ui->statusBar->showMessage("ERROR: No entry selected");
        return;
    }

    CheckFunctionSceneResize();

    SubMessage* selectedSub;
    PubMessage* selectedPub;
    MessageParameter* selectedPubParameter;
    bool newModelToBeDrawn = true;
    DrawnModelObject* newModelObject = new DrawnModelObject();
    QString paramName;

    //Get the currently selected data
    if(ui->messageBrowser->currentItem()->text(0) != NULL)
        paramName = ui->messageBrowser->currentItem()->text(0);
    else
        return;

    if(ui->messageBrowser->currentItem()->parent() == NULL)
        return;

    //Check the user is not trying to add the Message name rather than the parameters
    if(ui->messageBrowser->currentItem()->parent()->parent() == NULL)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "MainWindow",
                                                                    tr("Please Confirm you wish to add a 'Message' to the Function,"
                                                                       "and not just a 'Parameter'."),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            if(ui->messageBrowser->currentItem()->parent()->text(0) == "Sub")
            {
                foreach (SubMessage* subMsg, selectedICD.v_pSubscribedMessages) {
                    if(QString::compare(paramName, QString::fromStdString(subMsg->sMmessage)) == 0)
                    {
                        selectedSub = subMsg;
                        break;
                    }
                }

                //Check The model it needs hasnt been drawn before
                foreach (DrawnModelObject* drawn, functionDrawnModels) {
                    if(strcmp(selectedSub->sMicd.c_str(), drawn->name.c_str()) == 0)
                        newModelToBeDrawn = false;
                }

                //Draw the model
                if(newModelToBeDrawn)
                {
                    //Chop the 'IcdModel' off
                    QString parsedsMsgName = selectedSub->sMicd.c_str();

                    if(QString::compare(parsedsMsgName.left(8),"IcdModel") == 0)
                        parsedsMsgName.remove(0,8);

                    QRectF newIcdRect = QRect(functionHorizontalSpacing,50,100,50);
                    QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
                    QLineF classLine = QLineF(midPoint, midPoint);
                    classLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

                    QLabel *newIcdName = new QLabel(parsedsMsgName);
                    int width = newIcdName->fontMetrics().boundingRect(newIcdName->text()).width();
                    newIcdName->move(midPoint.x() - width/2, midPoint.y()-30);

                    functionScene->addLine(classLine, QPen(Qt::black));
                    functionScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
                    functionScene->addWidget(newIcdName);

                    //Add the New Model Object to the ModelVector
                    newModelObject->rect = newIcdRect;
                    newModelObject->name = selectedSub->sMicd.c_str();
                    newModelObject->midpoint = midPoint;
                    newModelObject->line = classLine;
                    newModelObject->label = newIcdName;
                    newModelObject->parsedName = parsedsMsgName.toStdString();
                    functionDrawnModels.append(newModelObject);

                    functionHorizontalSpacing+= cHorizontalSpacing;
                }

                //Check the model we are drawing to/from is not NULL
                if(newModelObject->name=="")
                {
                    foreach (DrawnModelObject* drawn, functionDrawnModels) {
                        if(strcmp(selectedSub->sMicd.c_str(), drawn->name.c_str()) == 0)
                             newModelObject = drawn;
                    }
                }

                //Draw Arrows
                if(newModelObject->name!="")
                {
                QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
                QPolygonF arrowPoly = CreateArrowHead(dataArrow, true);
                functionScene->addLine(dataArrow);
                functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

                functionVerticalSpacing+= 30;

                //Draw Labels
                QPushButton *newDataName = new QPushButton(paramName);
                newDataName->setFlat(true);
                newDataName->setStyleSheet("background-color: transparent");
                newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
                functionScene->addWidget(newDataName);

                connect(newDataName, SIGNAL(clicked()),
                            this, SLOT(onDataObjectClicked()));

                //Add the New Data Object to the DataVector
                DrawnDataObject* newData = new DrawnDataObject();
                newData->arrowPoly = arrowPoly;
                newData->label = newDataName;
                newData->line = dataArrow;
                newData->name = newDataName->text().toStdString();
                newData->pubOrSub = "sub";
                newData->model = newModelObject;
                newData->model->linkedDataObjects++;

                functionDrawnData.append(newData);
                }
            }

            if(ui->messageBrowser->currentItem()->parent()->text(0) == "Pub")
            {
                foreach (PubMessage* pubMsg, selectedICD.v_pPublishedMessages) {
                    if(QString::compare(paramName, QString::fromStdString(pubMsg->pMname)) == 0)
                    {
                        selectedPub = pubMsg;
                        break;
                    }
                }

                QString parsedpMsgName = selectedPub->pMname.c_str();
                parsedpMsgName.remove("Msg", Qt::CaseSensitive);
                parsedpMsgName.remove(selectedICD.parsedName.c_str(), Qt::CaseSensitive);

                //Validation check for duplicates
                foreach (DrawnModelObject* drawn, functionDrawnModels) {
                    if(strcmp(parsedpMsgName.toStdString().c_str(), drawn->parsedName.c_str()) == 0)
                    {
                        newModelToBeDrawn = false;
                        newModelObject = drawn;
                    }
                }


                //Draw the model
                if(newModelToBeDrawn)
                {
                    QString parsedMsgName = selectedPub->pMname.c_str();
                    parsedMsgName.remove("Msg", Qt::CaseSensitive);
                    parsedMsgName.remove(selectedICD.parsedName.c_str(), Qt::CaseSensitive);

                    QRectF newIcdRect = QRect(functionHorizontalSpacing,50,100,50);
                    QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
                    QLineF classLine = QLineF(midPoint, midPoint);
                    classLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

                    QLabel *newIcdName = new QLabel(parsedMsgName);
                    int width = newIcdName->fontMetrics().boundingRect(newIcdName->text()).width();
                    newIcdName->move(midPoint.x() - width/2, midPoint.y()-30);

                    functionScene->addLine(classLine, QPen(Qt::black));
                    functionScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
                    functionScene->addWidget(newIcdName);

                    //Add the New Model Object to the ModelVector
                    newModelObject->rect = newIcdRect;
                    newModelObject->name = selectedPub->pMname.c_str();
                    newModelObject->midpoint = midPoint;
                    newModelObject->line = classLine;
                    newModelObject->label = newIcdName;
                    newModelObject->parsedName = parsedMsgName.toStdString();
                    functionDrawnModels.append(newModelObject);

                    functionHorizontalSpacing+= cHorizontalSpacing;
                }

                //Check the model we are drawing to/from is not NULL
                if(newModelObject->name=="")
                {
                    foreach (DrawnModelObject* drawn, functionDrawnModels) {
                        if(strcmp(selectedPub->pMname.c_str(), drawn->name.c_str()) == 0)
                             newModelObject = drawn;
                    }
                }

                //Draw Arrows
                if(newModelObject->name!="")
                {
                QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
                QPolygonF arrowPoly = CreateArrowHead(dataArrow, false);
                functionScene->addLine(dataArrow);
                functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

                functionVerticalSpacing+= 30;

                //Draw Labels
                QPushButton *newDataName = new QPushButton(paramName);
                newDataName->setFlat(true);
                newDataName->setStyleSheet("background-color: transparent");
                newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
                functionScene->addWidget(newDataName);

                connect(newDataName, SIGNAL(clicked()),
                            this, SLOT(onDataObjectClicked()));

                //Add the New Data Object to the DataVector
                DrawnDataObject* newData = new DrawnDataObject();
                newData->arrowPoly = arrowPoly;
                newData->label = newDataName;
                newData->line = dataArrow;
                newData->name = newDataName->text().toStdString();
                newData->pubOrSub = "pub";
                newData->model = newModelObject;
                newData->model->linkedDataObjects++;

                functionDrawnData.append(newData);
                }
            }

        }
        return;
    }

    //Sub Messages
    if(ui->messageBrowser->currentItem()->parent()->parent()->text(0) == "Sub") // THIS IS BAD
    {
        QString MsgName = ui->messageBrowser->currentItem()->parent()->text(0);

        //Get the selected icd messages from the clicked button
        foreach (SubMessage* subMsg, selectedICD.v_pSubscribedMessages) {
            if(QString::compare(MsgName, QString::fromStdString(subMsg->sMmessage)) == 0)
            {
                selectedSub = subMsg;
                break;
            }
        }

        //Check The model it needs hasnt been drawn before - check for duplicates

        foreach (DrawnModelObject* drawn, functionDrawnModels) {
            if(strcmp(selectedSub->sMicd.c_str(), drawn->name.c_str()) == 0)
                newModelToBeDrawn = false;
        }
        //Draw new Model if needed
        if(newModelToBeDrawn)
        {
            //Chop the 'IcdModel' off
            QString parsedsMsgName = QString::fromStdString(selectedSub->sMicd);

            if(QString::compare(parsedsMsgName.left(8),"IcdModel") == 0)
                parsedsMsgName.remove(0,8);

            QRectF newIcdRect = QRect(functionHorizontalSpacing,50,100,50);
            QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
            QLineF classLine = QLineF(midPoint, midPoint);
            classLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

            QLabel *newIcdName = new QLabel(parsedsMsgName);
            newIcdName->setText(parsedsMsgName);
            int width = newIcdName->fontMetrics().boundingRect(newIcdName->text()).width();
            newIcdName->move(midPoint.x() - width/2, midPoint.y()-30);

            functionScene->addLine(classLine, QPen(Qt::black));
            functionScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
            functionScene->addWidget(newIcdName);

            //Add the New Model Object to the ModelVector
            newModelObject->rect = newIcdRect;
            newModelObject->name = selectedSub->sMicd.c_str();
            newModelObject->line = classLine;
            newModelObject->label = newIcdName;
            newModelObject->midpoint = midPoint;
            newModelObject->parsedName = parsedsMsgName.toStdString();
            functionDrawnModels.append(newModelObject);

            functionHorizontalSpacing+= cHorizontalSpacing;
        }

        //Draw Arrows
        if(newModelObject->name=="") //Check the model we are drawing to/from is not NULL
        {
            foreach (DrawnModelObject* drawn, functionDrawnModels) {
                if(strcmp(selectedSub->sMicd.c_str(), drawn->name.c_str()) == 0)
                     newModelObject = drawn;
            }
        }

        QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, true);
        functionScene->addLine(dataArrow);
        functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

        functionVerticalSpacing+= 30;

        //Draw Labels
        QPushButton* newDataName = new QPushButton(paramName);
        newDataName->setFlat(true);
        newDataName->setObjectName(paramName);
        newDataName->setStyleSheet("background-color: transparent");
        newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
        functionScene->addWidget(newDataName);

        connect(newDataName, SIGNAL(clicked()),
                    this, SLOT(onDataObjectClicked()));

        //Add the New Data Object to the DataVector
        DrawnDataObject* newData = new DrawnDataObject();
        newData->arrowPoly = arrowPoly;
        newData->label = newDataName;
        newData->line = dataArrow;
        newData->name = newDataName->text().toStdString();
        newData->pubOrSub = "sub";
        newData->model = newModelObject;
        newData->model->linkedDataObjects++;
        newData->posX = newModelObject->line.p1().x();
        newData->posY = functionVerticalSpacing;

        functionDrawnData.append(newData);
    }

    //Pub Messages
    if(ui->messageBrowser->currentItem()->parent()->parent()->text(0) == "Pub") // THIS IS BAD
    {
        QString MsgName = ui->messageBrowser->currentItem()->parent()->text(0);

        //Get the selected icd messages and param from the clicked button
        foreach (PubMessage* pubMsg, selectedICD.v_pPublishedMessages) {
            if(QString::compare(MsgName, QString::fromStdString(pubMsg->pMname)) == 0)
            {
                foreach (MessageParameter* param, pubMsg->v_pPubMparameters) {
                    if(QString::compare(paramName, QString::fromStdString(param->pMname)) == 0)
                    {
                        selectedPub = pubMsg;
                        selectedPubParameter = param;
                        break;
                    }
                }
            }
        }

        QString parsedpMsgName = selectedPub->pMname.c_str();
        parsedpMsgName.remove("Msg", Qt::CaseSensitive);
        parsedpMsgName.remove(selectedICD.parsedName.c_str(), Qt::CaseSensitive);

        //Validation check for duplicates
        foreach (DrawnModelObject* drawn, functionDrawnModels) {
//            qDebug() << parsedpMsgName.toStdString().c_str();
//            qDebug() << drawn->parsedName.c_str();
//            qDebug() << strcmp(parsedpMsgName.toStdString().c_str(), drawn->parsedName.c_str());
            if(strcmp(parsedpMsgName.toStdString().c_str(), drawn->parsedName.c_str()) == 0)
            {
                newModelToBeDrawn = false;
                newModelObject = drawn;
            }
        }

        //Draw new Model if needed
        if(newModelToBeDrawn)
        {
            QLabel *newIcdName = new QLabel(parsedpMsgName);
            QRectF newIcdRect = QRect(functionHorizontalSpacing, 50, 100, 50);
            newIcdName->move(newIcdRect.x()+10,newIcdRect.center().y());

            QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
            QLineF modelLine = QLineF(midPoint, midPoint);
            modelLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

            QLabel *mainModelName = new QLabel(parsedpMsgName);
            int width = mainModelName->fontMetrics().boundingRect(mainModelName->text()).width();
            mainModelName->move(midPoint.x() - width/2, midPoint.y()-30);

            functionScene->addLine(modelLine, QPen(Qt::black));
            functionScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
            functionScene->addWidget(newIcdName);

            //Add the New Model Object to the ModelVector
            newModelObject->rect = newIcdRect;
            newModelObject->name = selectedPub->pMname.c_str();
            newModelObject->line = modelLine;
            newModelObject->label = newIcdName;
            newModelObject->midpoint = midPoint;
            newModelObject->parsedName = parsedpMsgName.toStdString();
            functionDrawnModels.append(newModelObject);

            functionHorizontalSpacing += cHorizontalSpacing;
            CheckFunctionSceneResize();
        }

        QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, false);
        functionScene->addLine(dataArrow);
        functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

        functionVerticalSpacing+= 30;
        CheckFunctionSceneResize();

        //Draw Labels
        QPushButton *newDataName = new QPushButton(selectedPubParameter->pMname.c_str());
        newDataName->setFlat(true);
        newDataName->setStyleSheet("background-color: transparent");
        newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
        functionScene->addWidget(newDataName);

        connect(newDataName, SIGNAL(clicked()),
                    this, SLOT(onDataObjectClicked()));

        //Add the New Data Object to the DataVector
        DrawnDataObject* newData = new DrawnDataObject();
        newData->arrowPoly = arrowPoly;
        newData->label = newDataName;
        newData->line = dataArrow;
        newData->name = newDataName->text().toStdString();
        newData->pubOrSub = "pub";
        newData->model = newModelObject;
        newData->model->linkedDataObjects++;
        newData->posX = newModelObject->line.p1().x();
        newData->posY = functionVerticalSpacing;

        functionDrawnData.append(newData);
    }

    ui->messageBrowser->update();
}

void MainWindow::onAddPilotInteraction()
{
    DrawnModelObject* newModelObject = new DrawnModelObject();
    QString parsedpMsgName;
    bool newModelToBeDrawn = true;
    PubMessage* selectedPub;

    CheckFunctionSceneResize();

    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Pilot Interaction"),
                                           tr("Pilot Action:"), QLineEdit::Normal,
                                           tr(""), &ok);
    if (ok && !text.isEmpty())
    {
        bool ok2;
        QStringList items;
        foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
            parsedpMsgName = QString::fromStdString(pm->pMname);
            items.append(parsedpMsgName);
        }
        items.append(QString::fromStdString(functionDrawnModels[1]->parsedName));
        QString model = QInputDialog::getItem(this, tr("Add Pilot Interaction"),
                                               tr("To Model:"), items, 0, false, &ok2);
        if (ok2 && !model.isEmpty())
        {
            if(model == QString::fromStdString(functionDrawnModels[1]->parsedName))
            {
                newModelToBeDrawn = false;
                newModelObject = functionDrawnModels[1];
            }
            else
            {
                foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
                    if(model == QString::fromStdString(pm->pMname))
                    {
                        selectedPub = pm;
                    }
                }
            }

            model.remove("Msg", Qt::CaseSensitive);
            model.remove(selectedICD.parsedName.c_str(), Qt::CaseSensitive);

            //Validation check for duplicates
            foreach (DrawnModelObject* drawn, functionDrawnModels) {
                if(strcmp(model.toStdString().c_str(), drawn->parsedName.c_str()) == 0)
                {
                    newModelToBeDrawn = false;
                    newModelObject = drawn;
                }
            }

            //Draw new Model if needed
            if(newModelToBeDrawn)
            {
                QRectF newIcdRect = QRect(functionHorizontalSpacing, 50, 100, 50);
                QPointF midPoint = QLineF(newIcdRect.bottomLeft(),newIcdRect.bottomRight()).pointAt(0.5);
                QLineF modelLine = QLineF(midPoint, midPoint);
                modelLine.setP2(QPointF(midPoint.x(),midPoint.y()+cClassLineLength));

                QLabel *mainModelName = new QLabel(model);
                int width = mainModelName->fontMetrics().boundingRect(mainModelName->text()).width();
                mainModelName->move(midPoint.x() - width/2, midPoint.y()-30);

                functionScene->addLine(modelLine, QPen(Qt::black));
                functionScene->addRect(newIcdRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
                functionScene->addWidget(mainModelName);

                //Add the New Model Object to the ModelVector
                newModelObject->rect = newIcdRect;
                newModelObject->name = selectedPub->pMname.c_str();
                newModelObject->line = modelLine;
                newModelObject->label = mainModelName;
                newModelObject->midpoint = midPoint;
                newModelObject->parsedName = model.toStdString();
                newModelObject->linkedDataObjects++;

                functionDrawnModels.append(newModelObject);

                functionHorizontalSpacing += cHorizontalSpacing;
            }

            DrawnModelObject* pilotModelObject = functionDrawnModels[0];
            DrawnModelObject* targetModelObject = functionDrawnModels[1];

            QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, pilotModelObject->line.p1().x(), functionVerticalSpacing);
            QPolygonF arrowPoly = CreateArrowHead(dataArrow, false);
            functionScene->addLine(dataArrow);
            functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

            functionVerticalSpacing+= 30;

            //Draw Labels
            QPushButton *newDataName = new QPushButton(text);
            newDataName->setFlat(true);
            newDataName->setStyleSheet("background-color: transparent");
            newDataName->move(dataArrow.p2().x()+15,dataArrow.p2().y()-25);
            functionScene->addWidget(newDataName);

            connect(newDataName, SIGNAL(clicked()),
                        this, SLOT(onDataObjectClicked()));

            //Add the New Data Object to the DataVector
            DrawnDataObject* newData = new DrawnDataObject();
            newData->arrowPoly = arrowPoly;
            newData->label = newDataName;
            newData->line = dataArrow;
            newData->model = newModelObject;
            newData->name = text.toStdString();
            newData->pubOrSub = "pilot";

            functionDrawnData.append(newData);
        }
    }
}

void MainWindow::onDataObjectClicked()
{
    // Deselect
    if (functionSelectedButton == (QPushButton*)sender())
    {
        functionSelectedButton->setStyleSheet("border: none; background-color: transparent");
        functionSelectedButton = NULL;
        functionSelectedDataObject = NULL;

        foreach(QAction* action, functionDropdownMenu->actions())
        {
            action->setEnabled(false);
        }

        return;
    }

    //Else deal with the old button before setting the new one
    if (functionSelectedButton != NULL)
        functionSelectedButton->setStyleSheet("border: none; background-color: transparent");

    functionSelectedButton = (QPushButton*)sender();

    // Set the selection as #selected and find its dataObject
    functionSelectedButton->setStyleSheet("border: 2px solid #8f8f91");

    foreach(DrawnDataObject* ddo, functionDrawnData)
    {
        if(functionSelectedButton == ddo->label)
        {
            functionSelectedDataObject = ddo;

            foreach(QAction* action, functionDropdownMenu->actions())
            {
                action->setEnabled(true);
            }
            break;
        }
    }

    if (functionSelectedDataObject==NULL)
    {
        functionSelectedButton->setStyleSheet("border: none; background-color: transparent");
        functionSelectedButton = NULL;
        functionSelectedDataObject = NULL;

        qDebug() << "Not Found";
    }
}

void MainWindow::functionContextMenuRequested(const QPoint& point)
{
    functionDropdownMenu->popup(mapToGlobal(point));
}

void MainWindow::taskflowContextMenuRequested(const QPoint& point)
{
    taskflowDropdownMenu->popup(mapToGlobal(point));
}

void MainWindow::onRenameFunction()
{
    if(ui->tabWidget->currentIndex() == 1)
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Rename Function"),
                                             tr("New Function Name:"), QLineEdit::Normal,
                                             functionTitle, &ok);
        if (ok && !text.isEmpty())
        {
            functionTitle = text;
            RedrawFunctionScene();
        }
    }
    else if(ui->tabWidget->currentIndex() == 2)
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Rename Task"),
                                             tr("New Task Name:"), QLineEdit::Normal,
                                             taskTitle, &ok);
        if (ok && !text.isEmpty())
        {
            taskTitle = text;
            RedrawTaskflowScene();
        }
    }
}

void MainWindow::onAddFunctionToTask()
{
    if(ui->taskFunctionList->currentItem()== NULL)
    {
        ui->statusBar->showMessage("ERROR: No entry selected");
        return;
    }

    QString functionName;

    //Get the currently selected data
    if(ui->taskFunctionList->currentItem()->text() != NULL)
        functionName = ui->taskFunctionList->currentItem()->text();
    else
        return;

    QString dir = cSTDLocation + QString::fromStdString(selectedICD.name) + "/Functions/";

    QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::NoIteratorFlags);

    while (it.hasNext())
    {
        QString fileName = it.next();
        QString fileDir = fileName;
        fileName.remove(dir);
        fileName.remove(".xml");

        if(fileName.compare(functionName) == 0)
        {
//            QRectF newTaskRect = QRect(cTaskflowHorizontalMidpoint-100,taskflowVerticalSpacing,200,50);
//            QPointF midPoint = QLineF(newTaskRect.bottomLeft(),newTaskRect.bottomRight()).pointAt(0.5);
//            QLineF line = QLineF(midPoint, midPoint);
//            line.setP2(QPointF(midPoint.x(),midPoint.y()+50));

//            QPushButton *newTaskFunctionName = new QPushButton(fileName);
//            newTaskFunctionName->setText(fileName);
//            newTaskFunctionName->setFlat(true);
//            int width = newTaskFunctionName->fontMetrics().boundingRect(newTaskFunctionName->text()).width();
//            newTaskFunctionName->move(midPoint.x() - width/2, midPoint.y()-30);

//            QPolygonF arrowHead = CreateArrowHead(line,true);

//            taskflowScene->addLine(line, QPen(Qt::black));
//            taskflowScene->addRect(newTaskRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));
//            taskflowScene->addWidget(newTaskFunctionName);
//            taskflowScene->addPolygon(arrowHead);

            DrawnTaskFunction* newTaskFunction = new DrawnTaskFunction;
            newTaskFunction->name = fileName;
            newTaskFunction->dir = fileDir;
//            newTaskFunction->line = line;
//            newTaskFunction->label = newTaskFunctionName;
//            newTaskFunction->arrowPoly = arrowHead;
            taskflowDrawnFunctions.insert(taskflowDrawnFunctions.size()-1, newTaskFunction);

            RedrawTaskflowScene();

            break;
        }

    }


}

void MainWindow::onFunctionObjectClicked() //derived from onDataObjectClicked
{
    // Deselect
    if (taskflowSelectedButton == (QPushButton*)sender())
    {
        taskflowSelectedButton->setStyleSheet("border: none; background-color: transparent");
        taskflowSelectedButton = NULL;
        functionSelectedDataObject = NULL;

        foreach(QAction* action, taskflowDropdownMenu->actions())
        {
            action->setEnabled(false);
        }

        return;
    }

    //Else deal with the old button before setting the new one
    if (taskflowSelectedButton != NULL)
        taskflowSelectedButton->setStyleSheet("border: none; background-color: transparent");

    taskflowSelectedButton = (QPushButton*)sender();

    // Set the selection as #selected and find its dataObject
    taskflowSelectedButton->setStyleSheet("border: 2px solid #8f8f91");

    foreach(DrawnTaskFunction* dtf, taskflowDrawnFunctions)
    {
        if(taskflowSelectedButton == dtf->label)
        {
            taskflowSelectedFunctionObject = dtf;

            foreach(QAction* action, taskflowDropdownMenu->actions())
            {
                action->setEnabled(true);
            }
            break;
        }
    }

    if (taskflowSelectedFunctionObject==NULL)
    {
        taskflowSelectedButton->setStyleSheet("border: none; background-color: transparent");
        taskflowSelectedButton = NULL;
        taskflowSelectedFunctionObject = NULL;

        qDebug() << "Not Found";
    }}

void MainWindow::RedrawTaskflowScene()
{
    taskflowVerticalSpacing = 100;

    taskflowScene->clear();
    ui->taskflowGraphicsView->update();

    if(taskTitle != NULL)
    {
        QLabel *DiagramTitle = new QLabel(tr(taskTitle.toStdString().c_str()));
        DiagramTitle->setAlignment(Qt::AlignHCenter);
        DiagramTitle->move(10,10);
        QFont f( "Arial", 20, QFont::Bold);
        f.setUnderline(true);
        DiagramTitle->setFont(f);
        taskflowScene->addWidget(DiagramTitle);
    }

    QLineF line;
    QRectF newTaskRect;
    QPushButton *newTaskFunctionName;
    QPolygonF arrowHead;

    foreach (DrawnTaskFunction* dtf, taskflowDrawnFunctions) {

        CheckTaskflowSceneResize();

        if(dtf->name==("startObject"))
        {
            newTaskRect = QRect(cTaskflowHorizontalMidpoint+75, taskflowVerticalSpacing,50,50);
            QPointF midPoint = QLineF(newTaskRect.bottomLeft(),newTaskRect.bottomRight()).pointAt(0.5);
            line = QLineF(midPoint, midPoint);
            line.setP2(QPointF(midPoint.x(),midPoint.y()+cTaskflowVerticalSpacing-50));
            arrowHead = CreateTaskArrowHead(line);
            taskflowScene->addEllipse(newTaskRect, QPen(Qt::black, 5), QBrush(Qt::black, Qt::NoBrush));
            taskflowScene->addLine(line, QPen(Qt::black));
            taskflowScene->addPolygon(arrowHead);

            dtf->arrowPoly = arrowHead;
            dtf->line = line;
        }
        else if(dtf->name==("endObject"))
        {
            newTaskRect = QRect(cTaskflowHorizontalMidpoint+75, taskflowVerticalSpacing+7,50,50);
            taskflowScene->addEllipse(newTaskRect, QPen(Qt::black, 10), QBrush(Qt::black, Qt::NoBrush));
        }
        else
        {
            newTaskRect = QRect(cTaskflowHorizontalMidpoint, taskflowVerticalSpacing,200,50);
            QPointF midPoint = QLineF(newTaskRect.bottomLeft(),newTaskRect.bottomRight()).pointAt(0.5);
            line = QLineF(midPoint, midPoint);
            line.setP2(QPointF(midPoint.x(),midPoint.y()+cTaskflowVerticalSpacing-50));

            newTaskFunctionName = new QPushButton(dtf->name);
            newTaskFunctionName->setText(dtf->name);
            newTaskFunctionName->setFlat(true);

            if(toggleTaskStyleOn)
            {
                if(dtf->modified == aModifiedStatus[eNotModified])
                    newTaskFunctionName->setStyleSheet("color: rgba(0,0,0,1.0)");
                else if(dtf->modified == aModifiedStatus[eModified])
                    newTaskFunctionName->setStyleSheet("color: rgba(0,0,255,1.0)");
                else
                    newTaskFunctionName->setStyleSheet("color: rgba(0,0,0,1.0)");
            }
            else
                newTaskFunctionName->setStyleSheet("color: rgba(0,0,0,1.0)");

            newTaskFunctionName->setFocusPolicy(Qt::NoFocus);
            int width = newTaskFunctionName->fontMetrics().boundingRect(newTaskFunctionName->text()).width();
            newTaskFunctionName->move(midPoint.x() - width/2 - 5, midPoint.y()-30);
            taskflowScene->addWidget(newTaskFunctionName);

            connect(newTaskFunctionName, SIGNAL(clicked()),
                        this, SLOT(onFunctionObjectClicked()));

            arrowHead = CreateTaskArrowHead(line);
            taskflowScene->addPolygon(arrowHead);
            taskflowScene->addLine(line, QPen(Qt::black));
            taskflowScene->addRect(newTaskRect, QPen(Qt::black), QBrush(Qt::black, Qt::NoBrush));

            dtf->label = newTaskFunctionName;
            dtf->arrowPoly = arrowHead;
            dtf->line = line;
        }

        taskflowVerticalSpacing += cTaskflowVerticalSpacing;

    }
}

void MainWindow::onSaveTaskflowScene()
{
    if(taskTitle == selectedICD.name.c_str()) //Catch for not naming the Task
        onRenameFunction();

    QString saveDirectory = cSTDLocation + QString::fromStdString(selectedICD.name) +"/Tasks/";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Directory"),
                                                    saveDirectory+taskTitle,
                                                    tr("XML files (*.xml)"));

    //Cancel Catch
    if(fileName == NULL)
    {
        ui->statusBar->showMessage("ERROR: Save Cancelled");
        return;
    }

    fileName.remove(".xml");
    fileName += ".xml";
    QString fullDir = fileName;
    fileName.remove(saveDirectory);

    XMLDocument diagram;
    diagram.SaveFile(fullDir.toStdString().c_str());// Creates the file.
    FILE* pFile;
    pFile = fopen(fullDir.toStdString().c_str(), "w");
    if (pFile == NULL)
        qDebug() << "error opening file";

    XMLPrinter printer(pFile);

    printer.PushComment(selectedICD.name.c_str());
    printer.OpenElement("TaskTitle");
    printer.PushAttribute("name", taskTitle.toStdString().c_str());

    printer.OpenElement("Functions");
    foreach (DrawnTaskFunction* dtf, taskflowDrawnFunctions) {
        printer.OpenElement("Function");
        printer.PushAttribute("name", dtf->name.toStdString().c_str());
        printer.PushAttribute("dir", dtf->dir.toStdString().c_str());
        printer.CloseElement();
    }
    printer.CloseElement();

    printer.OpenElement("Comments");
    printer.PushAttribute("comments", taskComments.toStdString().c_str());
    printer.CloseElement();

    printer.CloseElement();

    diagram.Print(&printer);
    fclose(pFile);

    ui->statusBar->showMessage("Saving Complete");
}

void MainWindow::onLoadTaskflowScene()
{
    //Clear everything out
    taskflowDrawnFunctions.clear();
    taskTitle.clear();
    taskflowSelectedButton = NULL;
    taskflowSelectedFunctionObject = NULL;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    cSTDLocation + QString::fromStdString(selectedICD.name) +"/Tasks/",
                                                    tr("XML files (*.xml)"));
    //Cancel Catch
    if(fileName == NULL)
    {
        ui->statusBar->showMessage("ERROR: Load Cancelled");
        return;
    }

    XMLDocument doc;
    doc.LoadFile( fileName.toStdString().c_str() );
    XMLElement* root = doc.FirstChildElement();
    taskTitle = root->Attribute("name");

    for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
    {
        if(strcmp(element->Name(), "Functions") == 0)
        {
            for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
            {
                DrawnTaskFunction* dtf = new DrawnTaskFunction();
                QString dmoName = elementSubscribe->Attribute("name");
                QString dmoParsedName = elementSubscribe->Attribute("dir");

                dtf->name = dmoName;
                dtf->dir = dmoParsedName;

                taskflowDrawnFunctions.append(dtf);
            }
        }

        if(strcmp(element->Name(), "Comments") == 0)
        {
                string comment = element->Attribute("comments");

                taskComments = QString::fromStdString(comment);
        }
    }

    foreach (DrawnTaskFunction* dtf, taskflowDrawnFunctions) {
        if(dtf->dir != "")
        {
            LoadCustomFunctionScene(dtf->dir);
            foreach (DrawnDataObject* ddo, functionDrawnData) {
                if(ddo->modified == aModifiedStatus[eModified] ||
                        ddo->modified == aModifiedStatus[eAdded] ||
                        ddo->modified == aModifiedStatus[eRemoved])
                {
                    dtf->modified = aModifiedStatus[eModified];
                    break;
                }
            }
        }
    }

    ui->statusBar->showMessage("Loading Complete");
    RedrawTaskflowScene();
    onHomeTriggered();
}

void MainWindow::onTabChange(int _tab)
{
    if(_tab == 0)
    {
        ui->menuTasks->setEnabled(false);
        ui->menuFunctions->setEnabled(false);
    }
    else if(_tab == 1)
    {
        ui->menuTasks->setEnabled(false);
        ui->menuFunctions->setEnabled(true);
    }
    else if(_tab == 2)
    {
        ui->menuTasks->setEnabled(true);
        ui->menuFunctions->setEnabled(false);
    }
}

void MainWindow::onCheckforChanges()
{
    //get all xmls from the icd folder
    QDirIterator it(cIcdLocation, QStringList() << "*.xml", QDir::Files, QDirIterator::NoIteratorFlags);

    while (it.hasNext())
    {
        QString filePath;

        filePath = it.next();
        if(it.fileName().toStdString() != "DatabaseH145.xml" &&
                it.fileName().toStdString() != "IcdModelQTGHost.xml" &&
                it.fileName().toStdString() != "IcdModelTemplate.xml" &&
                it.fileName().toStdString() != "FlightModelH145_AerotimInputs.xml" &&
                it.fileName().toStdString() != "IcdModelQTGExecutor.xml")
        {
            QString fileName = filePath;
            fileName.remove(cIcdLocation);
            fileName.remove(".xml");
            //take one xml
            QFileInfo file1(filePath);
            QDir stdIcd (cSTDLocation);

                //check it exists ie if it is new
            if(!stdIcd.exists(fileName))
            {
                //if it is new, create the directory and save it
                if(QMessageBox::No == QMessageBox::critical(this, fileName +" is new",
                                                    "An ICD that has not been scanned before has been detected, and the folder directory will now be created",
                                                    QMessageBox::Ok, QMessageBox::No))
                {
                    ui->statusBar->showMessage("ERROR: CheckForChanges was cancelled");
                    return;
                }

                QDir dir(cSTDLocation + fileName);
                if (!dir.exists()) {
                    dir.mkpath(".");
                    dir.mkpath("Functions");
                    dir.mkpath("Tasks");
                    dir.mkpath("Diagrams");
                }

                XMLDocument doc;
                doc.LoadFile( filePath.toStdString().c_str());
                XMLElement* root = doc.FirstChildElement("RmsfInterface");
                string strName = root->Attribute("name");
                QDateTime recentIcdDate = it.fileInfo().lastModified();

                string textNode = root->Attribute("name");
                qDebug() << textNode.c_str();

                ICD newICD;

                for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
                {
                    qDebug() << element->Name();

                    if(strcmp(element->Name(), "Subscribe") == 0)
                    {

                        for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
                        {
                            string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                            string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                            qDebug() << SubscribeCheckIcd.c_str();
                            qDebug() << SubscribeCheckMessage.c_str();

                            SubMessage* subMessage = new SubMessage(
                                        SubscribeCheckIcd, SubscribeCheckMessage, "", ""); //Smells like memory leaks.
                            newICD.v_pSubscribedMessages.append(subMessage);
                        }


                    }

                    if(strcmp(element->Name(), "Message") == 0)
                    {
                        string MessageCheckName = element->Attribute("name");
                        const char* MessageCheckNetwork = element->Attribute("network");
                        const char* MessageCheckComment = element->Attribute("comment");
                        qDebug() << MessageCheckName.c_str();
                        qDebug() << MessageCheckNetwork;
                        qDebug() << MessageCheckComment;

                        QVector<MessageParameter*> msgPrms;


                        for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementPublish->Attribute("name");
                            const char* ParameterCheckType = elementPublish->Attribute("type");
                            const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                            const char* ParameterCheckDefault = elementPublish->Attribute("default");
                            const char* ParameterCheckMin = elementPublish->Attribute("min");
                            const char* ParameterCheckMax = elementPublish->Attribute("max");
                            const char* ParameterCheckComment = elementPublish->Attribute("comment");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == NULL)
                                ParameterCheckUnit = "";
                            if(ParameterCheckMin == NULL)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == NULL)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == NULL)
                                ParameterCheckComment = "";
                            if(ParameterCheckDefault == NULL)
                                ParameterCheckDefault = "";


                            MessageParameter* messageParameter = new MessageParameter(
                                        ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, "", "", -1); //Smells like memory leaks.

                            msgPrms.append(messageParameter);

                        }

                        if(MessageCheckNetwork == NULL)
                            MessageCheckNetwork = "";
                        if(MessageCheckComment == NULL)
                            MessageCheckComment = "";
                        PubMessage* pubMessage = new PubMessage(
                                    MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, "", "");

                        newICD.v_pPublishedMessages.append(pubMessage);



                    }

                    if(strcmp(element->Name(), "Enum") == 0)
                    {
                        string EnumCheckName = element->Attribute("name");
                        string EnumCheckComment = element->Attribute("comment");
                        qDebug() << EnumCheckName.c_str();
                        qDebug() << EnumCheckComment.c_str();

                        QVector<EnumValue*> enumValues;

                        for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
                        {
                            string ValueCheckName = elementValue->Attribute("name");
                            string ValueCheckValue = elementValue->Attribute("value");
                            string ValueCheckComment = elementValue->Attribute("comment");

                            qDebug() << ValueCheckName.c_str();
                            qDebug() << ValueCheckValue.c_str();
                            qDebug() << ValueCheckComment.c_str();

                            EnumValue* enumValue = new EnumValue(
                                        ValueCheckName,ValueCheckValue,ValueCheckComment, "", ""); //Smells like memory leaks.

                            enumValues.append(enumValue);
                        }

                        Enum* eNum = new Enum(
                                    EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                        newICD.v_pEnums.append(eNum);
                    }

                    if(strcmp(element->Name(), "Struct") == 0)
                    {
                        string StructCheckName = element->Attribute("name");
                        const char* StructCheckNetwork = element->Attribute("network");
                        string StructCheckComment = element->Attribute("comment");
                        qDebug() << StructCheckName.c_str();
                        qDebug() << StructCheckNetwork;
                        qDebug() << StructCheckComment.c_str();

                        QVector<StructParameter*> strPrms;


                        for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
                        {
                            const char* ParameterCheckName = elementStruct->Attribute("name");
                            const char* ParameterCheckType = elementStruct->Attribute("type");
                            const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                            const char* ParameterCheckDefault = elementStruct->Attribute("default");
                            const char* ParameterCheckMin = elementStruct->Attribute("min");
                            const char* ParameterCheckMax = elementStruct->Attribute("max");
                            const char* ParameterCheckComment = elementStruct->Attribute("comment");
                            qDebug() << ParameterCheckName;
                            qDebug() << ParameterCheckType;
                            qDebug() << ParameterCheckUnit;
                            qDebug() << ParameterCheckDefault;
                            qDebug() << ParameterCheckMin;
                            qDebug() << ParameterCheckMax;
                            qDebug() << ParameterCheckComment;

                            if(ParameterCheckUnit == 0)
                                ParameterCheckUnit = "";
                            if(ParameterCheckDefault == 0)
                                ParameterCheckDefault = "";
                            if(ParameterCheckMin == 0)
                                ParameterCheckMin = "";
                            if(ParameterCheckMax == 0)
                                ParameterCheckMax = "";
                            if(ParameterCheckComment == 0)
                                ParameterCheckComment = "";

                            StructParameter* structParameter = new StructParameter(
                                        ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                                        ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                        ParameterCheckComment, "", ""); //Smells like memory leaks.

                            strPrms.append(structParameter);
                        }

                        if(StructCheckNetwork == NULL)
                            StructCheckNetwork = "";

                        Struct* str = new Struct(
                                    StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

                        newICD.v_pStructs.append(str);
                    }
                }

                newICD.name = strName;
                QString parsedName = strName.c_str();
                parsedName.remove("Icd", Qt::CaseSensitive);
                parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
                newICD.parsedName = parsedName.toStdString();
                newICD.recentIcdDate = recentIcdDate;

                SaveOneXML(newICD);

            }

            else
            {
                //if it does exist, start by scanning both the stock and std icd
                //get it's equivalent stdIcd
                XMLDocument doc;
                doc.LoadFile( (cSTDLocation.toStdString() + fileName.toStdString() + "/std" + fileName.toStdString() + ".xml").c_str());
                XMLElement* root = doc.FirstChildElement("StdInterface");
                QDateTime stdIcdDate = QDateTime::fromString(root->Attribute("recentIcdDate"),Qt::TextDate);

                    //check the date it was created on
                QDateTime stockIcdDate = file1.lastModified();

                    //see if the the date has changed
                //if it has not changed moved on
                // if it has

                if(stdIcdDate != stockIcdDate)
                {
                    if(QMessageBox::Ok != QMessageBox::critical(this, fileName +" changes have been detected",
                                        "An ICD that has been modified has been detected, and shall now be dealt with",
                                        QMessageBox::Ok, QMessageBox::Cancel))
                    {
                        ui->statusBar->showMessage("ERROR: CheckForChanges was cancelled");
                        return;
                    }
                    else
                    {
                        //Scan in this modified icd
                        doc.LoadFile( filePath.toStdString().c_str());
                        XMLElement* root = doc.FirstChildElement("RmsfInterface");
                        string Rmsf = root->Attribute("namespace");
                        string strName = root->Attribute("name");               

                        ICD tempStockICD;

                        //SCAN LOOP
                        for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
                        {
                            qDebug() << element->Name();

                            if(strcmp(element->Name(), "Subscribe") == 0)
                            {

                                for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
                                {
                                    string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                                    string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                                    qDebug() << SubscribeCheckIcd.c_str();
                                    qDebug() << SubscribeCheckMessage.c_str();

                                    SubMessage* subMessage = new SubMessage(
                                                SubscribeCheckIcd, SubscribeCheckMessage, "", ""); //Smells like memory leaks.
                                    tempStockICD.v_pSubscribedMessages.append(subMessage);
                                }


                            }

                            if(strcmp(element->Name(), "Message") == 0)
                            {
                                string MessageCheckName = element->Attribute("name");
                                const char* MessageCheckNetwork = element->Attribute("network");
                                const char* MessageCheckComment = element->Attribute("comment");
                                qDebug() << MessageCheckName.c_str();
                                qDebug() << MessageCheckNetwork;
                                qDebug() << MessageCheckComment;

                                QVector<MessageParameter*> msgPrms;


                                for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                                {
                                    const char* ParameterCheckName = elementPublish->Attribute("name");
                                    const char* ParameterCheckType = elementPublish->Attribute("type");
                                    const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                                    const char* ParameterCheckDefault = elementPublish->Attribute("default");
                                    const char* ParameterCheckMin = elementPublish->Attribute("min");
                                    const char* ParameterCheckMax = elementPublish->Attribute("max");
                                    const char* ParameterCheckComment = elementPublish->Attribute("comment");
                                    qDebug() << ParameterCheckName;
                                    qDebug() << ParameterCheckType;
                                    qDebug() << ParameterCheckUnit;
                                    qDebug() << ParameterCheckDefault;
                                    qDebug() << ParameterCheckMin;
                                    qDebug() << ParameterCheckMax;
                                    qDebug() << ParameterCheckComment;

                                    if(ParameterCheckUnit == NULL)
                                        ParameterCheckUnit = "";
                                    if(ParameterCheckMin == NULL)
                                        ParameterCheckMin = "";
                                    if(ParameterCheckMax == NULL)
                                        ParameterCheckMax = "";
                                    if(ParameterCheckComment == NULL)
                                        ParameterCheckComment = "";
                                    if(ParameterCheckDefault == NULL)
                                        ParameterCheckDefault = "";


                                    MessageParameter* messageParameter = new MessageParameter(
                                                ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                                                ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                                ParameterCheckComment, "", "", -1); //Smells like memory leaks.

                                    msgPrms.append(messageParameter);

                                }

                                if(MessageCheckNetwork == NULL)
                                    MessageCheckNetwork = "";
                                if(MessageCheckComment == NULL)
                                    MessageCheckComment = "";
                                PubMessage* pubMessage = new PubMessage(
                                            MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, "", "");

                                tempStockICD.v_pPublishedMessages.append(pubMessage);



                            }

                            if(strcmp(element->Name(), "Enum") == 0)
                            {
                                string EnumCheckName = element->Attribute("name");
                                string EnumCheckComment = element->Attribute("comment");
                                qDebug() << EnumCheckName.c_str();
                                qDebug() << EnumCheckComment.c_str();

                                QVector<EnumValue*> enumValues;

                                for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
                                {
                                    string ValueCheckName = elementValue->Attribute("name");
                                    string ValueCheckValue = elementValue->Attribute("value");
                                    string ValueCheckComment = elementValue->Attribute("comment");

                                    qDebug() << ValueCheckName.c_str();
                                    qDebug() << ValueCheckValue.c_str();
                                    qDebug() << ValueCheckComment.c_str();


                                    EnumValue* enumValue = new EnumValue(
                                                ValueCheckName,ValueCheckValue,ValueCheckComment,"",""); //Smells like memory leaks.

                                    enumValues.append(enumValue);
                                }

                                Enum* eNum = new Enum(
                                            EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                                tempStockICD.v_pEnums.append(eNum);
                            }

                            if(strcmp(element->Name(), "Struct") == 0)
                            {
                                string StructCheckName = element->Attribute("name");
                                const char* StructCheckNetwork = element->Attribute("network");
                                string StructCheckComment = element->Attribute("comment");
                                qDebug() << StructCheckName.c_str();
                                qDebug() << StructCheckNetwork;
                                qDebug() << StructCheckComment.c_str();

                                QVector<StructParameter*> strPrms;


                                for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
                                {
                                    const char* ParameterCheckName = elementStruct->Attribute("name");
                                    const char* ParameterCheckType = elementStruct->Attribute("type");
                                    const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                                    const char* ParameterCheckDefault = elementStruct->Attribute("default");
                                    const char* ParameterCheckMin = elementStruct->Attribute("min");
                                    const char* ParameterCheckMax = elementStruct->Attribute("max");
                                    const char* ParameterCheckComment = elementStruct->Attribute("comment");
                                    qDebug() << ParameterCheckName;
                                    qDebug() << ParameterCheckType;
                                    qDebug() << ParameterCheckUnit;
                                    qDebug() << ParameterCheckDefault;
                                    qDebug() << ParameterCheckMin;
                                    qDebug() << ParameterCheckMax;
                                    qDebug() << ParameterCheckComment;

                                    if(ParameterCheckUnit == 0)
                                        ParameterCheckUnit = "";
                                    if(ParameterCheckDefault == 0)
                                        ParameterCheckDefault = "";
                                    if(ParameterCheckMin == 0)
                                        ParameterCheckMin = "";
                                    if(ParameterCheckMax == 0)
                                        ParameterCheckMax = "";
                                    if(ParameterCheckComment == 0)
                                        ParameterCheckComment = "";

                                    StructParameter* structParameter = new StructParameter(
                                                ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                                                ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                                ParameterCheckComment, "", ""); //Smells like memory leaks.

                                    strPrms.append(structParameter);
                                }

                                if(StructCheckNetwork == NULL)
                                    StructCheckNetwork = "";

                                Struct* str = new Struct(
                                            StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

                                tempStockICD.v_pStructs.append(str);
                            }
                        }

                        tempStockICD.name = strName;
                        QString parsedName = strName.c_str();
                        parsedName.remove("Icd", Qt::CaseSensitive);
                        parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
                        tempStockICD.parsedName = parsedName.toStdString();
                        tempStockICD.recentIcdDate = stockIcdDate;

                        //scan in the STD Icd
                        XMLDocument doc;
                        doc.LoadFile( (cSTDLocation.toStdString() + fileName.toStdString() + "/std" + fileName.toStdString() + ".xml").c_str());
                        root = doc.FirstChildElement("StdInterface");
                        Rmsf = root->Attribute("namespace");
                        strName = root->Attribute("name");
                        QDateTime recentIcdDate = QDateTime::fromString(root->Attribute("recentIcdDate"),Qt::TextDate);


                        string textNode = root->Attribute("name");
                        qDebug() << textNode.c_str();

                        ICD tempStdICD;

                        //SCAN LOOP
                        for(XMLElement* element = root->FirstChildElement(); element != NULL; element = element->NextSiblingElement())
                        {
                            qDebug() << element->Name();

                            if(strcmp(element->Name(), "Subscribe") == 0)
                            {

                                for(XMLElement* elementSubscribe = element->FirstChildElement(); elementSubscribe != NULL; elementSubscribe = elementSubscribe->NextSiblingElement())
                                {
                                    string SubscribeCheckIcd = elementSubscribe->Attribute("icd");
                                    string SubscribeCheckMessage = elementSubscribe->Attribute("message");
                                    string SubscribeCheckModified = elementSubscribe->Attribute("modified");
                                    string SubscribeCheckAccepted = elementSubscribe->Attribute("accepted");
                                    qDebug() << SubscribeCheckIcd.c_str();
                                    qDebug() << SubscribeCheckMessage.c_str();

                                    SubMessage* subMessage = new SubMessage(
                                                SubscribeCheckIcd, SubscribeCheckMessage, aModifiedStatus[eRemoved], SubscribeCheckAccepted); //Smells like memory leaks.
                                    tempStdICD.v_pSubscribedMessages.append(subMessage);
                                }


                            }

                            if(strcmp(element->Name(), "Message") == 0)
                            {
                                string MessageCheckName = element->Attribute("name");
                                const char* MessageCheckNetwork = element->Attribute("network");
                                const char* MessageCheckComment = element->Attribute("comment");
                                const char* MessageCheckMod = element->Attribute("modified");
                                const char* MessageCheckAcc = element->Attribute("accepted");
                                qDebug() << MessageCheckName.c_str();
                                qDebug() << MessageCheckNetwork;
                                qDebug() << MessageCheckComment;

                                QVector<MessageParameter*> msgPrms;


                                for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                                {
                                    const char* ParameterCheckName = elementPublish->Attribute("name");
                                    const char* ParameterCheckType = elementPublish->Attribute("type");
                                    const char* ParameterCheckUnit = elementPublish->Attribute("unit");
                                    const char* ParameterCheckDefault = elementPublish->Attribute("default");
                                    const char* ParameterCheckMin = elementPublish->Attribute("min");
                                    const char* ParameterCheckMax = elementPublish->Attribute("max");
                                    const char* ParameterCheckComment = elementPublish->Attribute("comment");
                                    const char* ParameterCheckMod = elementPublish->Attribute("modified");
                                    const char* ParameterCheckAcc = elementPublish->Attribute("accepted");
                                    qDebug() << ParameterCheckName;
                                    qDebug() << ParameterCheckType;
                                    qDebug() << ParameterCheckUnit;
                                    qDebug() << ParameterCheckDefault;
                                    qDebug() << ParameterCheckMin;
                                    qDebug() << ParameterCheckMax;
                                    qDebug() << ParameterCheckComment;

                                    if(ParameterCheckUnit == NULL)
                                        ParameterCheckUnit = "";
                                    if(ParameterCheckMin == NULL)
                                        ParameterCheckMin = "";
                                    if(ParameterCheckMax == NULL)
                                        ParameterCheckMax = "";
                                    if(ParameterCheckComment == NULL)
                                        ParameterCheckComment = "";
                                    if(ParameterCheckDefault == NULL)
                                        ParameterCheckDefault = "";


                                    MessageParameter* messageParameter = new MessageParameter(
                                                ParameterCheckName, ParameterCheckType, ParameterCheckUnit,
                                                ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                                ParameterCheckComment, aModifiedStatus[eRemoved], ParameterCheckAcc, -1); //Smells like memory leaks.

                                    msgPrms.append(messageParameter);

                                }

                                if(MessageCheckNetwork == NULL)
                                    MessageCheckNetwork = "";
                                if(MessageCheckComment == NULL)
                                    MessageCheckComment = "";
                                PubMessage* pubMessage = new PubMessage(
                                            MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms, aModifiedStatus[eRemoved], MessageCheckAcc);

                                tempStdICD.v_pPublishedMessages.append(pubMessage);



                            }

                            if(strcmp(element->Name(), "Enum") == 0)
                            {
                                string EnumCheckName = element->Attribute("name");
                                string EnumCheckComment = element->Attribute("comment");
                                qDebug() << EnumCheckName.c_str();
                                qDebug() << EnumCheckComment.c_str();

                                QVector<EnumValue*> enumValues;

                                for(XMLElement* elementValue = element->FirstChildElement(); elementValue != NULL; elementValue = elementValue->NextSiblingElement())
                                {
                                    string ValueCheckName = elementValue->Attribute("name");
                                    string ValueCheckValue = elementValue->Attribute("value");
                                    string ValueCheckComment = elementValue->Attribute("comment");
                                    string ValueCheckMod = elementValue->Attribute("modified");
                                    string ValueCheckAcc = elementValue->Attribute("accepted");

                                    qDebug() << ValueCheckName.c_str();
                                    qDebug() << ValueCheckValue.c_str();
                                    qDebug() << ValueCheckComment.c_str();

                                    EnumValue* enumValue = new EnumValue(
                                                ValueCheckName,ValueCheckValue,ValueCheckComment,ValueCheckMod,ValueCheckAcc); //Smells like memory leaks.

                                    enumValues.append(enumValue);
                                }

                                Enum* eNum = new Enum(
                                            EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                                tempStdICD.v_pEnums.append(eNum);


                            }

                            if(strcmp(element->Name(), "Struct") == 0)
                            {
                                string StructCheckName = element->Attribute("name");
                                const char* StructCheckNetwork = element->Attribute("network");
                                string StructCheckComment = element->Attribute("comment");
                                qDebug() << StructCheckName.c_str();
                                qDebug() << StructCheckNetwork;
                                qDebug() << StructCheckComment.c_str();

                                QVector<StructParameter*> strPrms;


                                for(XMLElement* elementStruct = element->FirstChildElement(); elementStruct != NULL; elementStruct = elementStruct->NextSiblingElement())
                                {
                                    const char* ParameterCheckName = elementStruct->Attribute("name");
                                    const char* ParameterCheckType = elementStruct->Attribute("type");
                                    const char* ParameterCheckUnit = elementStruct->Attribute("unit");
                                    const char* ParameterCheckDefault = elementStruct->Attribute("default");
                                    const char* ParameterCheckMin = elementStruct->Attribute("min");
                                    const char* ParameterCheckMax = elementStruct->Attribute("max");
                                    const char* ParameterCheckComment = elementStruct->Attribute("comment");
                                    const char* ParameterCheckMod = elementStruct->Attribute("modified");
                                    const char* ParameterCheckAcc = elementStruct->Attribute("accepted");
                                    qDebug() << ParameterCheckName;
                                    qDebug() << ParameterCheckType;
                                    qDebug() << ParameterCheckUnit;
                                    qDebug() << ParameterCheckDefault;
                                    qDebug() << ParameterCheckMin;
                                    qDebug() << ParameterCheckMax;
                                    qDebug() << ParameterCheckComment;

                                    if(ParameterCheckUnit == 0)
                                        ParameterCheckUnit = "";
                                    if(ParameterCheckDefault == 0)
                                        ParameterCheckDefault = "";
                                    if(ParameterCheckMin == 0)
                                        ParameterCheckMin = "";
                                    if(ParameterCheckMax == 0)
                                        ParameterCheckMax = "";
                                    if(ParameterCheckComment == 0)
                                        ParameterCheckComment = "";

                                    StructParameter* structParameter = new StructParameter(
                                                ParameterCheckName,ParameterCheckType,ParameterCheckUnit,
                                                ParameterCheckDefault, ParameterCheckMin, ParameterCheckMax,
                                                ParameterCheckComment, ParameterCheckMod, ParameterCheckAcc); //Smells like memory leaks.

                                    strPrms.append(structParameter);
                                }

                                if(StructCheckNetwork == NULL)
                                    StructCheckNetwork = "";

                                Struct* str = new Struct(
                                            StructCheckName, StructCheckNetwork, StructCheckComment, strPrms);

                                tempStdICD.v_pStructs.append(str);
                            }
                        }

                        tempStdICD.name = strName;
                        parsedName = strName.c_str();
                        parsedName.remove("Icd", Qt::CaseSensitive);
                        parsedName.remove("Model", Qt::CaseSensitive); //ERROR SHOULD BE DYNAMIC
                        tempStdICD.parsedName = parsedName.toStdString();
                        tempStdICD.recentIcdDate = recentIcdDate;


                        /*
                        //      **PROGRAM BEGINS CHECKING FOR CHANGES HERE**
                        //        Take a message
                        //        If it exists in the std
                        //            Set it to 'unmodifed'
                        //        else if it does not exist
                        //            Ask the User what to do
                        //                add
                        //                    add the message to the std
                        //                    set it to added
                        //                replace
                        //                    replace an existing messages
                        //                    set it to modified
                        //                ignore
                        //                    do not add the message to the std
                        ////        any message now still set to 'remove'
                        ////            remove from the std
                        */


                        ///////////////////////////////////////////////////////////
                        //                  Subscribed Messages                  //
                        ///////////////////////////////////////////////////////////

                        //check each variable to see if it exists
                            //if it does not, create it and set it as #new
                            //if it does, alter what is needed and set it as #modified
                        foreach (SubMessage* stockSub, tempStockICD.v_pSubscribedMessages) {
                            foreach (SubMessage* stdSub, tempStdICD.v_pSubscribedMessages) {
                                if(stdSub->sMicd == stockSub->sMicd && stdSub->sMmessage == stockSub->sMmessage)
                                {
                                    tempStockICD.v_pSubscribedMessages.removeOne(stockSub);
                                    stdSub->sMmodified = aModifiedStatus[eNotModified];
                                    stdSub->sMaccepted = aAcceptedStatus[eToBeAssessed];
                                }
                            }
                        }

                        //any remainding messgaes need to be put to the user
                        foreach (SubMessage* stockSub, tempStockICD.v_pSubscribedMessages) {
                            bool msgHandled = false;

                            while(msgHandled != true)
                            {
                                QString one = QString::fromStdString(stockSub->sMicd);
                                QString two = QString::fromStdString(stockSub->sMmessage);
                                QString str = QString("%1      %2\n 'Add' to keep this Message, 'Replace' to replace an existing Message, 'Ignore' to disregard the Message")
                                                       .arg(one).arg(two);
                                QMessageBox msgBox(QMessageBox::Warning, tr("A new Subscriber Message requires your input"),
                                                   str , 0, this);

                                QPushButton* pButtonAdd = msgBox.addButton(tr("Add"), QMessageBox::YesRole);
                                QAbstractButton* pButtonReplace = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
                                QAbstractButton* pButtonIgnore= msgBox.addButton(tr("Ignore"), QMessageBox::NoRole);
                                msgBox.setDefaultButton(pButtonAdd);

                                msgBox.exec();

                                if(msgBox.clickedButton() == pButtonAdd)
                                {
                                    msgHandled = true;
                                    stockSub->sMmodified = aModifiedStatus[eModified];
                                    stockSub->sMaccepted = aAcceptedStatus[eToBeAssessed];
                                    tempStdICD.v_pSubscribedMessages.append(stockSub);
                                    tempStockICD.v_pSubscribedMessages.removeOne(stockSub);
                                    break;
                                }
                                if(msgBox.clickedButton() == pButtonReplace)
                                {
                                    bool ok;
                                    QStringList items;
                                    foreach (SubMessage* sm, tempStdICD.v_pSubscribedMessages) {
                                        items.append(QString::fromStdString(sm->sMmessage));
                                    }
                                    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                                                         tr("Existing Messages:"), items, 0, false, &ok);
                                    if (ok && !item.isEmpty())
                                    {
                                        //add the new entry and delete the old
                                        foreach (SubMessage* sm, tempStdICD.v_pSubscribedMessages) {
                                            if(item.compare(QString::fromStdString(sm->sMmessage)) == 0)
                                            {
                                                stockSub->sMmodified = aModifiedStatus[eModified];
                                                stockSub->sMaccepted = aAcceptedStatus[eToBeAssessed];
                                                tempStdICD.v_pSubscribedMessages.append(stockSub);
                                                tempStdICD.v_pSubscribedMessages.removeOne(sm);
                                                tempStockICD.v_pSubscribedMessages.removeOne(stockSub);
                                                break;
                                            }
                                        }
                                        msgHandled = true;
                                    }
                                    break;
                                }
                                if(msgBox.clickedButton() == pButtonIgnore)
                                {
                                    msgHandled = true;
                                    tempStockICD.v_pSubscribedMessages.removeOne(stockSub);
                                    break;
                                }
                            }
                        }

                        //remove any deleted sub messages
//                        foreach (SubMessage* stdSub, tempStdICD.v_pSubscribedMessages) {
//                            if(stdSub->sMmodified == aModifiedStatus[eRemoved])
//                            {
//                                tempStdICD.v_pSubscribedMessages.removeOne(stdSub);
//                            }
//                        }

                        ///////////////////////////////////////////////////////////
                        //                  Published Messages                   //
                        ///////////////////////////////////////////////////////////
/*
                        //same for pubs, check the pub name, then check the variables too
                        foreach (PubMessage* stockPub, tempStockICD.v_pPublishedMessages) {
                            foreach (PubMessage* stdPub, tempStockICD.v_pPublishedMessages) {
                                bool msgHandled = false;

                                if(stockPub->pMname == stdPub->pMname)
                                {
                                    stockPub->pMmodified = aModifiedStatus[eNotModified];

                                    foreach (MessageParameter* stockPubValue, stockPub->v_pPubMparameters) {
                                        foreach (MessageParameter* stdPubValue, stdPub->v_pPubMparameters) {

                                            if(stockPubValue->pMname == stdPubValue->pMname) //first check the name then check everything else
                                            {
                                                msgHandled = true;
                                                if(stockPubValue->pMtype != stdPubValue->pMtype) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Type Discrepancy",
                                                                                        QString::fromStdString("A New type for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMtype = stockPubValue->pMtype;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }
                                                if(stockPubValue->pMunit != stdPubValue->pMunit) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Unit Discrepancy",
                                                                                        QString::fromStdString("A New unit for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMunit = stockPubValue->pMunit;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }
                                                if(stockPubValue->pMdefault != stdPubValue->pMdefault) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Default Discrepancy",
                                                                                        QString::fromStdString("A New Default for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMdefault = stockPubValue->pMdefault;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }
                                                if(stockPubValue->pMmin != stdPubValue->pMmin) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Min Discrepancy",
                                                                                        QString::fromStdString("A New Min for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMmin = stockPubValue->pMmin;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }
                                                if(stockPubValue->pMmax != stdPubValue->pMmax) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Max Discrepancy",
                                                                                        QString::fromStdString("A New Max for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMmax = stockPubValue->pMmax;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }
                                                if(stockPubValue->pMcomment != stdPubValue->pMcomment) //Have to parse individually to handle individual conflicts
                                                {
                                                    bool accept = QMessageBox::question(this,"Comment Discrepancy",
                                                                                        QString::fromStdString("A New Comment for '" + stdPubValue->pMname + "' has been made"),
                                                                                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
                                                    if(accept)
                                                    {
                                                        stdPubValue->pMcomment = stockPubValue->pMcomment;
                                                        stdPubValue->pMmodified = aModifiedStatus[eModified];
                                                        stdPubValue->pMaccepted = "eToBeAssessed";
                                                        msgHandled = true;
                                                    }
                                                }

                                                if(msgHandled == false)
                                                    stdPubValue->pMmodified = aModifiedStatus[eNotModified];
                                            }

                                        }

                                    }


                                    tempStockICD.v_pPublishedMessages.removeOne(stockPub);
                                }
                            }
                        }
                        //Any left over messages are either new or replacing old
                        foreach (PubMessage* stockPub, tempStockICD.v_pPublishedMessages) {

                        }

                        //delete all removed Messages
                        foreach (PubMessage* stdPub, tempStdICD.v_pPublishedMessages) {
                            if(stdPub->pMmodified == aModifiedStatus[eRemoved])
                            {
                                tempStdICD.v_pPublishedMessages.removeOne(stdPub);
                            }
                        }

                        tempStdICD.recentIcdDate = stockIcdDate;
                        SaveOneXML(tempStdICD);
*/
                        foreach (PubMessage* stockPub, tempStockICD.v_pPublishedMessages) {
                            foreach (PubMessage* stdPub, tempStdICD.v_pPublishedMessages) {
                                if(stockPub->pMname == stdPub->pMname)
                                {
                                    stdPub->pMmodified = aModifiedStatus[eNotModified];
                                    foreach (MessageParameter* stockPubParam, stockPub->v_pPubMparameters) {
                                        foreach (MessageParameter* stdPubParam, stdPub->v_pPubMparameters) {
                                            if(stockPubParam->pMname == stdPubParam->pMname)
                                            {
                                                stdPubParam->pMmodified = aModifiedStatus[eNotModified];
                                                stockPub->v_pPubMparameters.removeOne(stockPubParam);
                                            }
                                        }
                                    }

                                    foreach (MessageParameter* stockPubParam, stockPub->v_pPubMparameters) {
                                        bool msgHandled = false;

                                        while(msgHandled != true)
                                        {
                                            QString one = QString::fromStdString(stockPub->pMname);
                                            QString two = QString::fromStdString(stockPubParam->pMname);
                                            QString str = QString("%1      %2\n 'Add' to keep this Parameter, 'Replace' to replace an existing Parameter, 'Ignore' to disregard the Parameter")
                                                                   .arg(one).arg(two);
                                            QMessageBox msgBox(QMessageBox::Warning, tr("A new Message Parameter requires your input"),
                                                               str , 0, this);

                                            QPushButton* pButtonAdd = msgBox.addButton(tr("Add"), QMessageBox::YesRole);
                                            QAbstractButton* pButtonReplace = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
                                            QAbstractButton* pButtonIgnore= msgBox.addButton(tr("Ignore"), QMessageBox::NoRole);
                                            msgBox.setDefaultButton(pButtonAdd);

                                            msgBox.exec();

                                            if(msgBox.clickedButton() == pButtonAdd)
                                            {
                                                msgHandled = true;
                                                stockPubParam->pMmodified = aModifiedStatus[eModified];
                                                stockPubParam->pMaccepted = aAcceptedStatus[eToBeAssessed];
                                                stdPub->v_pPubMparameters.append(stockPubParam);
                                                stockPub->v_pPubMparameters.removeOne(stockPubParam);
                                                break;
                                            }
                                            if(msgBox.clickedButton() == pButtonReplace)
                                            {
                                                bool ok;
                                                QStringList items;
                                                foreach (MessageParameter* pm, stdPub->v_pPubMparameters) {
                                                    items.append(QString::fromStdString(pm->pMname));
                                                }
                                                QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                                                                     tr("Existing Messages:"), items, 0, false, &ok);
                                                if (ok && !item.isEmpty())
                                                {
                                                    //add the new entry and delete the old
                                                    foreach (MessageParameter* stdPubParam, stdPub->v_pPubMparameters) {
                                                        if(item.compare(QString::fromStdString(stdPubParam->pMname)) == 0)
                                                        {
                                                            stockPubParam->pMmodified = aModifiedStatus[eModified];
                                                            stockPubParam->pMaccepted = aAcceptedStatus[eToBeAssessed];
                                                            stdPub->v_pPubMparameters.append(stockPubParam);
                                                            stdPub->v_pPubMparameters.removeOne(stdPubParam);
                                                            stockPub->v_pPubMparameters.removeOne(stockPubParam);
                                                            break;
                                                        }
                                                    }
                                                    msgHandled = true;
                                                }
                                                break;
                                            }
                                            if(msgBox.clickedButton() == pButtonIgnore)
                                            {
                                                msgHandled = true;
                                                stockPub->v_pPubMparameters.removeOne(stockPubParam);
                                                break;
                                            }

                                            if(stockPub->v_pPubMparameters.isEmpty())
                                                tempStockICD.v_pPublishedMessages.removeOne(stockPub);
                                        }
                                    }
                                    tempStockICD.v_pPublishedMessages.removeOne(stockPub);

                                    //Remove any deleted Parameters
//                                    foreach (MessageParameter* stdPubParam, stdPub->v_pPubMparameters) {
//                                        if(stdPubParam->pMmodified == aModifiedStatus[eRemoved])
//                                        {
//                                            stdPub->v_pPubMparameters.removeOne(stdPubParam);
//                                        }
//                                    }
                                }
                            }
                        }

                        foreach (PubMessage* stockPub, tempStockICD.v_pPublishedMessages) {
                            bool msgHandled = false;

                            while(msgHandled != true)
                            {
                                QString one = QString::fromStdString(stockPub->pMname);
                                QString str = QString("%1      \n 'Add' to keep this Parameter, 'Replace' to replace an existing Parameter, 'Ignore' to disregard the Parameter")
                                                       .arg(one);
                                QMessageBox msgBox(QMessageBox::Warning, tr("A new Published Message requires your input"),
                                                   str , 0, this);

                                QPushButton* pButtonAdd = msgBox.addButton(tr("Add"), QMessageBox::YesRole);
                                QAbstractButton* pButtonReplace = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
                                QAbstractButton* pButtonIgnore= msgBox.addButton(tr("Ignore"), QMessageBox::NoRole);
                                msgBox.setDefaultButton(pButtonAdd);

                                msgBox.exec();

                                if(msgBox.clickedButton() == pButtonAdd)
                                {
                                    msgHandled = true;
                                    stockPub->pMmodified = aModifiedStatus[eAdded];
                                    stockPub->pMaccepted = aAcceptedStatus[eToBeAssessed];
                                    foreach (MessageParameter* mp, stockPub->v_pPubMparameters) {
                                        mp->pMmodified = aModifiedStatus[eAdded];
                                        mp->pMaccepted = aAcceptedStatus[eToBeAssessed];
                                    }
                                    tempStdICD.v_pPublishedMessages.append(stockPub);
                                    tempStockICD.v_pPublishedMessages.removeOne(stockPub);
                                    break;
                                }
                                if(msgBox.clickedButton() == pButtonReplace)
                                {
                                    bool ok;
                                    QStringList items;
                                    foreach (PubMessage* pm, tempStdICD.v_pPublishedMessages) {
                                        items.append(QString::fromStdString(pm->pMname));
                                    }
                                    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
                                                                         tr("Existing Messages:"), items, 0, false, &ok);
                                    if (ok && !item.isEmpty())
                                    {
                                        //add the new entry and delete the old
                                        foreach (PubMessage* pm, tempStdICD.v_pPublishedMessages) {
                                            if(item.compare(QString::fromStdString(pm->pMname)) == 0)
                                            {
                                                stockPub->pMmodified = aModifiedStatus[eModified];
                                                stockPub->pMaccepted = aAcceptedStatus[eToBeAssessed];
                                                tempStdICD.v_pPublishedMessages.append(stockPub);
                                                tempStdICD.v_pPublishedMessages.removeOne(pm);
                                                tempStockICD.v_pPublishedMessages.removeOne(stockPub);
                                                msgHandled = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                if(msgBox.clickedButton() == pButtonIgnore)
                                {
                                    msgHandled = true;
                                    tempStockICD.v_pPublishedMessages.removeOne(stockPub);
                                    break;
                                }
                            }
                        }

//                        foreach (PubMessage* stdPub, tempStdICD.v_pPublishedMessages) {
//                            if(stdPub->pMmodified == aModifiedStatus[eRemoved])
//                            {
//                                tempStdICD.v_pPublishedMessages.removeOne(stdPub);
//                            }
//                        }

                        tempStdICD.recentIcdDate = stockIcdDate;
                        SaveOneXML(tempStdICD);
                        onParseStdXmls();
                    }
                }
            }
        }
    }
}

void MainWindow::onSaveToJPG()
{
    QGraphicsScene* scene;

    if(ui->tabWidget->currentIndex()==0)
        scene = icdScene;
    else if(ui->tabWidget->currentIndex()==1)
        scene = functionScene;
    else if(ui->tabWidget->currentIndex()==2)
        scene = taskflowScene;

//    scene->setSceneRect(scene->itemsBoundingRect());

    QString saveDirectory = cSTDLocation + QString::fromStdString(selectedICD.name) +"/Diagrams/";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Directory"),
                                                    saveDirectory,
                                                    tr("JPG files (*.jpg)"));

    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    scene->render(&painter);
    fileName += ".jpg";
    qDebug() << image.save(fileName);
}

void MainWindow::onFunctionDropdownMenuClicked(QAction * _action)
{
    if(functionSelectedButton == NULL)
        return;

    if(_action->text()=="Delete")
    {
        if(functionSelectedDataObject->pubOrSub != "" &&
                functionSelectedDataObject->pubOrSub != "pilot")
        {
            if(functionSelectedDataObject->model != NULL)
            {
                functionSelectedDataObject->model->linkedDataObjects--;
                if(functionSelectedDataObject->model->linkedDataObjects<1)
                {
                    functionDrawnModels.remove(functionDrawnModels.indexOf(functionSelectedDataObject->model));
                    functionHorizontalSpacing -= cHorizontalSpacing;
                }
            }
        }

        functionDrawnData.remove(functionDrawnData.indexOf(functionSelectedDataObject));
        functionVerticalSpacing -= cVerticalSpacing;


        foreach(QAction* action, functionDropdownMenu->actions())
        {
            action->setEnabled(false);
        }
    }
    else if(_action->text()=="Move Up")
    {
        int tempLocation = functionDrawnData.indexOf(functionSelectedDataObject);
        if(tempLocation == 0)
            ui->statusBar->showMessage("ERROR: Can't move any further up");
        else
        {
            DrawnDataObject* tempObject = functionDrawnData.at(tempLocation);
            functionDrawnData.replace(tempLocation, functionDrawnData.at(tempLocation-1));
            functionDrawnData.replace(tempLocation-1, tempObject);
        }
    }
    else if(_action->text()=="Move Down")
    {
        int tempLocation = functionDrawnData.indexOf(functionSelectedDataObject);
        if(functionDrawnData.size() == tempLocation+1)
            ui->statusBar->showMessage("ERROR: Can't move any further down");
        else
        {
            DrawnDataObject* tempObject = functionDrawnData.at(tempLocation);
            functionDrawnData.replace(tempLocation, functionDrawnData.at(tempLocation+1));
            functionDrawnData.replace(tempLocation+1, tempObject);
        }
    }
    else if(_action->text()=="Accept Change")
    {
        functionSelectedDataObject->accepted = aAcceptedStatus[eAccepted];

        if(functionSelectedDataObject->pubOrSub == "sub")
        {
            foreach (SubMessage* sm, selectedICD.v_pSubscribedMessages) {
                if(functionSelectedDataObject->name == sm->sMicd ||
                        functionSelectedDataObject->name == sm->sMmessage)
                {
                    sm->sMaccepted = aAcceptedStatus[eAccepted];
                    break;
                }
            }
        }
        else if(functionSelectedDataObject->pubOrSub == "pub")
        {
            foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
                if(functionSelectedDataObject->name == pm->pMname)
                {
                    pm->pMaccepted= aAcceptedStatus[eAccepted];
                    break;
                }
                else
                {
                    foreach (MessageParameter* mp, pm->v_pPubMparameters) {
                        if(functionSelectedDataObject->name == mp->pMname)
                        {
                            mp->pMaccepted = aAcceptedStatus[eAccepted];
                            break;
                        }
                    }
                }
            }
        }

        SaveOneXML(selectedICD);
    }
    else if(_action->text()=="Reject Change")
    {
        functionSelectedDataObject->accepted = aAcceptedStatus[eRejected];

        if(functionSelectedDataObject->pubOrSub == "sub")
        {
            foreach (SubMessage* sm, selectedICD.v_pSubscribedMessages) {
                if(functionSelectedDataObject->name == sm->sMicd ||
                        functionSelectedDataObject->name == sm->sMmessage)
                {
                    sm->sMaccepted = aAcceptedStatus[eRejected];
                    break;
                }
            }
        }
        else if(functionSelectedDataObject->pubOrSub == "pub")
        {
            foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
                if(functionSelectedDataObject->name == pm->pMname)
                {
                    pm->pMaccepted= aAcceptedStatus[eRejected];
                    break;
                }
                else
                {
                    foreach (MessageParameter* mp, pm->v_pPubMparameters) {
                        if(functionSelectedDataObject->name == mp->pMname)
                        {
                            mp->pMaccepted = aAcceptedStatus[eRejected];
                            break;
                        }
                    }
                }
            }
        }

        SaveOneXML(selectedICD);
    }
    else if(_action->text()=="Change to be Assessed")
    {
        functionSelectedDataObject->accepted = aAcceptedStatus[eToBeAssessed];

        if(functionSelectedDataObject->pubOrSub == "sub")
        {
            foreach (SubMessage* sm, selectedICD.v_pSubscribedMessages) {
                if(functionSelectedDataObject->name == sm->sMicd ||
                        functionSelectedDataObject->name == sm->sMmessage)
                {
                    sm->sMaccepted = aAcceptedStatus[eToBeAssessed];
                    break;
                }
            }
        }
        else if(functionSelectedDataObject->pubOrSub == "pub")
        {
            foreach (PubMessage* pm, selectedICD.v_pPublishedMessages) {
                if(functionSelectedDataObject->name == pm->pMname)
                {
                    pm->pMaccepted= aAcceptedStatus[eToBeAssessed];
                    break;
                }
                else
                {
                    foreach (MessageParameter* mp, pm->v_pPubMparameters) {
                        if(functionSelectedDataObject->name == mp->pMname)
                        {
                            mp->pMaccepted = aAcceptedStatus[eToBeAssessed];
                            break;
                        }
                    }
                }
            }
        }

        SaveOneXML(selectedICD);
    }

    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;
    RedrawFunctionScene();
    onNewTaskflowScene();
}

void MainWindow::onTaskflowDropdownMenuClicked(QAction * _action)
{
    if(taskflowSelectedButton == NULL)
        return;

    if(_action->text()=="Delete")
    {
        taskflowDrawnFunctions.remove(taskflowDrawnFunctions.indexOf(taskflowSelectedFunctionObject));
        taskflowVerticalSpacing -= cVerticalSpacing;


        foreach(QAction* action, taskflowDropdownMenu->actions())
        {
            action->setEnabled(false);
        }
    }
    else if(_action->text()=="Move Up")
    {
        int tempLocation = taskflowDrawnFunctions.indexOf(taskflowSelectedFunctionObject);
        if(tempLocation == 0)
            ui->statusBar->showMessage("ERROR: Can't move any further up");
        else
        {
            DrawnTaskFunction* tempObject = taskflowDrawnFunctions.at(tempLocation);
            taskflowDrawnFunctions.replace(tempLocation, taskflowDrawnFunctions.at(tempLocation-1));
            taskflowDrawnFunctions.replace(tempLocation-1, tempObject);
        }
    }
    else if(_action->text()=="Move Down")
    {
        int tempLocation = taskflowDrawnFunctions.indexOf(taskflowSelectedFunctionObject);
        if(taskflowDrawnFunctions.size() == tempLocation+1)
            ui->statusBar->showMessage("ERROR: Can't move any further down");
        else
        {
            DrawnTaskFunction* tempObject = taskflowDrawnFunctions.at(tempLocation);
            taskflowDrawnFunctions.replace(tempLocation, taskflowDrawnFunctions.at(tempLocation+1));
            taskflowDrawnFunctions.replace(tempLocation+1, tempObject);
        }
    }
    else if(_action->text()=="View Function")
    {
        LoadCustomFunctionScene(taskflowSelectedFunctionObject->dir);
        ui->tabWidget->setCurrentIndex(1);
    }
    taskflowSelectedButton = NULL;
    taskflowSelectedFunctionObject = NULL;
    RedrawTaskflowScene();
}
