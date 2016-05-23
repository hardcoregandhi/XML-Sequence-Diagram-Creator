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
const int cHorizontalSpacing = 150; //steps
const int cVerticalSpacing = 30; //steps
const QString cIcdLocation = "/home/jryan/simulation/dev/common/icd/";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    icdScene = new QGraphicsScene();
    functionScene = new QGraphicsScene();
    icdBrowser = ui->messageBrowser;

    ParseXML();
    SetupFileDirectories();
    SetupMessageBrowser();
    SetupIcdMenu();
    SetupDrawingArea();
    SetupContextMenu();

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
              SLOT(contextMenuRequested(const QPoint&)));
    connect(functionDropdownMenu, SIGNAL(triggered(QAction*)),this,
              SLOT(onFunctionDropdownMenuClicked(QAction*)));
    connect(ui->actionSave_Function, SIGNAL(triggered()),this,
              SLOT(onSaveFunction()));
    connect(ui->actionLoad_Function, SIGNAL(triggered()),this,
              SLOT(onLoadFunction()));
    connect(ui->actionRename, SIGNAL(triggered()),this,
              SLOT(onRenameFunction()));

    ui->statusBar->showMessage(QString::number(ui->graphicsView->verticalScrollBar()->value()) + " " + QString::number(ui->graphicsView->horizontalScrollBar()->value()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ParseXML()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home/jryan/simulation/dev/common/icd/",
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
                                    SubscribeCheckIcd, SubscribeCheckMessage); //Smells like memory leaks.
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
                                MessageCheckName, MessageCheckNetwork, MessageCheckComment, msgPrms);

                    newICD.v_pPublishedMessages.append(pubMessage);



                }

                if(strcmp(element->Name(), "Enum") == 0)
                {

                    for(XMLElement* elementEnum = element->FirstChildElement(); elementEnum != NULL; elementEnum = elementEnum->NextSiblingElement())
                    {
                        string EnumCheckName = elementEnum->Attribute("name");
                        string EnumCheckComment = elementEnum->Attribute("comment");
                        qDebug() << EnumCheckName.c_str();
                        qDebug() << EnumCheckComment.c_str();

                        QVector<EnumValue*> enumValues;

                        for(XMLElement* elementPublish = element->FirstChildElement(); elementPublish != NULL; elementPublish = elementPublish->NextSiblingElement())
                        {
                            string ValueCheckName = elementPublish->Attribute("name");
                            string ValueCheckValue = elementPublish->Attribute("value");
                            string ValueCheckComment = elementPublish->Attribute("comment");

                            qDebug() << ValueCheckName.c_str();
                            qDebug() << ValueCheckValue.c_str();
                            qDebug() << ValueCheckComment.c_str();


                            EnumValue* enumValue = new EnumValue(
                                        ValueCheckName,ValueCheckValue,ValueCheckComment); //Smells like memory leaks.

                            enumValues.append(enumValue);
                        }

                        Enum* eNum = new Enum(
                                    EnumCheckName, EnumCheckComment, enumValues); //Smells like memory leaks.
                        newICD.v_pEnums.append(eNum);
                    }

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

            v_ICDs.append(newICD);
        }
    }
}

void MainWindow::SetupIcdMenu()
{
    foreach (ICD icd, v_ICDs) {
        ui->listWidget->addItem(QString::fromUtf8(icd.name.c_str()));
    }
    ui->listWidget->sortItems(Qt::AscendingOrder);
    onListIcdClicked(ui->listWidget->item(0));
}

void MainWindow::SetupMessageBrowser()
{
    ui->messageBrowser->clear();
    ui->messageBrowser->setColumnCount(1);
    ui->messageBrowser->setHeaderLabel("Message Names");
    pubOrSub.append(new QTreeWidgetItem((QTreeWidget*)0,
                                         QStringList("Sub")));
    pubOrSub.append(new QTreeWidgetItem((QTreeWidget*)0,
                                         QStringList("Pub")));
    ui->messageBrowser->insertTopLevelItems(0, pubOrSub);
}

void MainWindow::SetupContextMenu()
{
    functionDropdownMenu = new QMenu();
    QList<QAction*> v_pfunctionDropdownMenuActions;
    v_pfunctionDropdownMenuActions.append(new QAction("Move Up", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Move Down", this));
    v_pfunctionDropdownMenuActions.append(new QAction("Delete", this));
    functionDropdownMenu->addActions(v_pfunctionDropdownMenuActions);
    functionDropdownMenu->setObjectName("functionDropdownMenu");

    foreach(QAction* action, functionDropdownMenu->actions())
    {
        action->setEnabled(false);
    }
}

void MainWindow::ResetScroll()
{
    ui->tabWidget->setCurrentIndex(0);
    onHomeTriggered();
    ui->tabWidget->setCurrentIndex(1);
    onHomeTriggered();
    ui->tabWidget->setCurrentIndex(0);
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
            QRectF newIcdRect = QRect(horizontalSpacing, 50, 100, 50);

            if (horizontalSpacing >2000)
                icdScene->setSceneRect(0,0,5000,3500);

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

    if(icdScene)
    {
        icdScene->setSceneRect(0,0,2500,2500);
        ui->graphicsView->setScene(icdScene);
        ui->graphicsView->setBackgroundBrush(Qt::white);

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

    //functionScene
    if(functionScene)
    {
        functionDrawnModels.clear();
        functionDrawnData.clear();
        ui->functionsGraphicsView->items().clear();
        functionScene = new QGraphicsScene();
        functionScene->setSceneRect(0,0,2500,2500);
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
}

void MainWindow::SaveAllSequenceDiagrams()
{
    for (int i = 0; i < v_ICDs.size(); i++)
    {
        string fileName = v_ICDs[i].name.c_str();
        fileName += ".xml";
        XMLDocument diagram;
        diagram.SaveFile(fileName.c_str()); // Creates the file.
        FILE* pFile;
        pFile = fopen(fileName.c_str(), "w");
        if (pFile == NULL)
            qDebug() << "error opening file";

        XMLPrinter printer(pFile);

        printer.PushComment(v_ICDs[i].name.c_str());
        printer.OpenElement(v_ICDs[i].name.c_str());

        printer.OpenElement("Subscribe");
        for(int j = 0; j < v_ICDs[i].v_pSubscribedMessages.size(); j++)
        {
            printer.OpenElement("Message");
            printer.PushAttribute("icd", v_ICDs[i].v_pSubscribedMessages[j]->sMicd.c_str());
            printer.PushAttribute("message", v_ICDs[i].v_pSubscribedMessages[j]->sMmessage.c_str());
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

        printer.OpenElement("Publish");
        printer.PushAttribute("Messages", v_ICDs[i].v_pPublishedMessages.size());
        for(int j = 0; j < v_ICDs[i].v_pPublishedMessages.size(); j++)
        {
            printer.OpenElement("Message");
            printer.PushAttribute("name", v_ICDs[i].v_pPublishedMessages[j]->pMname.c_str());
            printer.PushAttribute("comment", v_ICDs[i].v_pPublishedMessages[j]->pMcomment.c_str());

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

        printer.CloseElement();
        diagram.Print(&printer);
        fclose(pFile);

        qDebug() <<  "save done";
    }
}

void MainWindow::SetupFileDirectories()
{
    foreach (ICD _icd, v_ICDs) {
        QDir dir(cIcdLocation+QString::fromStdString("/STDs/"+_icd.name));
        if (!dir.exists()) {
            dir.mkpath(".");
            dir.mkpath("Functions");
            dir.mkpath("Tasks");
        }
    }
}

QPolygonF MainWindow::CreateArrowHead(QLineF arrowLine, bool rightPointing)
{
    //const int arrowHeadWidth = 5;

    QPointF arrowPoint;
    QPointF arrowRight;
    QPointF arrowLeft;

    if(!rightPointing)
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

void MainWindow::RedrawFunctionScene()
{
    functionHorizontalSpacing = 50;
    functionVerticalSpacing = 150;

    functionScene->clear();
    functionScene->clear();
    functionScene->clear();
    functionScene = new QGraphicsScene();
    functionScene->setSceneRect(0,0,2500,2500);
    ui->functionsGraphicsView->setScene(functionScene);
    ui->functionsGraphicsView->setBackgroundBrush(Qt::white);

    //Diagram Title
    const char* strDiagramTitle = functionTitle.toStdString().c_str();

    QLabel *DiagramTitle = new QLabel(tr(functionTitle.toStdString().c_str()));
    DiagramTitle->setAlignment(Qt::AlignHCenter);
    DiagramTitle->move(10,10);
    QFont f( "Arial", 20, QFont::Bold);
    f.setUnderline(true);
    DiagramTitle->setFont(f);
    functionScene->addWidget(DiagramTitle);

    foreach (DrawnModelObject* dmo, functionDrawnModels) {
        dmo->rect.setX(functionHorizontalSpacing);
        dmo->rect.setWidth(100);
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
    }

    foreach (DrawnDataObject* doo, functionDrawnData) {
        if(doo->pubOrSub == "pub"){
            doo->line = QLine(doo->model->line.p2().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, true);
        }
        else if(doo->pubOrSub == "sub"){
            doo->line = QLine(doo->model->line.p2().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, false);
        }
        else if(doo->pubOrSub == "pilot"){
            doo->line = QLine(doo->line.p1().x(), functionVerticalSpacing, doo->line.p2().x(), functionVerticalSpacing);
            doo->arrowPoly=CreateArrowHead(doo->line, true);
        }

        functionScene->addLine(doo->line, QPen(Qt::black));
        QPushButton* tempLabel = new QPushButton(doo->name.c_str());
        tempLabel->move(doo->line.p2().x()+15,doo->line.p2().y()-25);
        tempLabel->setFlat(true);
        tempLabel->setStyleSheet("background-color: transparent");
        functionScene->addWidget(tempLabel);
        functionScene->addPolygon(doo->arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));
        doo->label = tempLabel;

        connect(tempLabel, SIGNAL(clicked()),
                    this, SLOT(onDataObjectClicked()));

        functionVerticalSpacing += cVerticalSpacing;
    }
}

void MainWindow::SaveFunctionScene()
{
    QString saveDirectory = "/home/jryan/simulation/dev/common/icd/STDs/" + QString::fromStdString(selectedICD.name) +"/Functions/";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Directory"),
                                                    saveDirectory,
                                                    tr("XML files (*.xml)"));

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
    printer.OpenElement(functionTitle.toStdString().c_str());

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
    printer.CloseElement();

    diagram.Print(&printer);
    fclose(pFile);

    qDebug() <<  "save done";
}

void MainWindow::LoadFunctionScene()
{
    /*
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/home/jryan/simulation/dev/common/icd/STDs/"+ QString::fromStdString(selectedICD.name) +"/Functions/",
                                                    tr("XML files (*.xml)"));
    if(fileName==NULL)
    {
        ui->statusBar->showMessage("ERROR: Load Cancelled");
        return;
    }
    XMLDocument doc;
    doc.LoadFile( filePath.c_str() );

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
    printer.OpenElement(functionTitle.toStdString().c_str());

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
    printer.CloseElement();

    diagram.Print(&printer);
    fclose(pFile);

    qDebug() <<  "save done";
    */
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
}

void MainWindow::onListIcdClicked(QListWidgetItem* _item)
{
    horizontalSpacing = 200;
    verticalSpacing = 150;
    const char* selectedEntry = _item->text().toUtf8().constData();
        // This is the first item.
    foreach (ICD icd, v_ICDs) {
        if(strcmp(selectedEntry, icd.name.c_str()) == 0)
        {
            DrawUnsortedDiagram(icd);
            selectedICD = icd;
            break;
        }
    }
    ui->graphicsView->horizontalScrollBar()->setValue(0);
    ui->graphicsView->verticalScrollBar()->setValue(0);
    ui->graphicsView->update();

    subItems.clear(); //MEMORY LEAKS
    pubItems.clear(); //MEMORY LEAKS
    ui->messageBrowser->topLevelItem(0)->takeChildren();
    ui->messageBrowser->topLevelItem(1)->takeChildren();

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

    SetupDrawingArea();
    ResetScroll();
}

void MainWindow::onAddDataExchangeClicked()
{
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

                //Check The model it needs hasnt been drawn before
                foreach (DrawnModelObject* drawn, functionDrawnModels) {
                    if(strcmp(selectedPub->pMname.c_str(), drawn->name.c_str()) == 0)
                        newModelToBeDrawn = false;
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
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, false);
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
            if(strcmp(selectedPub->pMname.c_str(), drawn->name.c_str()) == 0)
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

            if (functionHorizontalSpacing >2000)
                icdScene->setSceneRect(0,0,5000,3500);

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
        }

        QLineF dataArrow = QLine(newModelObject->line.p1().x(), functionVerticalSpacing, functionDrawnModels[1]->line.p1().x(), functionVerticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, true);
        functionScene->addLine(dataArrow);
        functionScene->addPolygon(arrowPoly, QPen(Qt::red), QBrush(Qt::red, Qt::SolidPattern));

        functionVerticalSpacing+= 30;

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
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Pilot Interaction"),
                                           tr("Pilot Action:"), QLineEdit::Normal,
                                           tr(""), &ok);
    if (ok && !text.isEmpty())
    {
        DrawnModelObject* pilotModelObject = functionDrawnModels[0];
        DrawnModelObject* targetModelObject = functionDrawnModels[1];

        QLineF dataArrow = QLine(targetModelObject->line.p1().x(), functionVerticalSpacing, pilotModelObject->line.p1().x(), functionVerticalSpacing);
        QPolygonF arrowPoly = CreateArrowHead(dataArrow, true);
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
        newData->model = pilotModelObject;
        newData->name = text.toStdString();
        newData->pubOrSub = "pilot";

        functionDrawnData.append(newData);
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

void MainWindow::contextMenuRequested(const QPoint& point)
{
    functionDropdownMenu->popup(mapToGlobal(point));
}

void MainWindow::onRenameFunction()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Rename Function"),
                                         tr("New Function Name:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty())
    {
        functionTitle = text;
        RedrawFunctionScene();
    }
}

void MainWindow::onFunctionDropdownMenuClicked(QAction * _action)
{
    if(functionSelectedButton == NULL)
        return;

    if(_action->text()=="Delete")
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

    functionSelectedButton = NULL;
    functionSelectedDataObject = NULL;
    RedrawFunctionScene();
}

