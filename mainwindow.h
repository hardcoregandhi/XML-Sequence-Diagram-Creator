#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QListWidgetItem"
#include "QGraphicsScene"
#include "QGraphicsView"
#include "QLabel"
#include "QTreeWidget"
#include "QPushButton"
#include "QMenu"
#include <QDate>

namespace Ui {
class MainWindow;
}

enum AcceptedStatus
{
    eToBeAssessed = 0,
    eAccepted = 1,
    eRejected = 2
};

enum ModifiedStatus
{
    eNotModified = 0,
    eRemoved = 1,
    eAdded = 2,
    eModified = 3
};

struct SubMessage{
    std::string sMicd;
    std::string sMmessage;
    std::string sMmodified;
    std::string sMaccepted;

    SubMessage(std::string _sMicd,
               std::string _sMmessage,
               std::string _sMmodified,
               std::string _sMaccepted)
    {
        sMicd = _sMicd;
        sMmessage = _sMmessage;
        sMmodified = _sMmodified;
        sMaccepted = _sMaccepted;
    }
};

struct MessageParameter{
    std::string pMname;
    std::string pMtype;
    std::string pMunit;
    std::string pMdefault;
    std::string pMmin;
    std::string pMmax;
    std::string pMcomment;
    std::string pMmodified;
    std::string pMaccepted;
    int pMranking;  //this needs to go

    MessageParameter(std::string _pMname,
                      std::string _pMtype,
                      std::string _pMunit,
                      std::string _pMdefault,
                      std::string _pMmin,
                      std::string _pMmax,
                      std::string _pMcomment,
                      std::string _pMmodified,
                      std::string _pMaccepted,
                      int _ranking)
    {
        pMname = _pMname;
        pMtype = _pMtype;
        pMunit = _pMunit;
        pMdefault = _pMdefault;
        pMmin = _pMmin;
        pMmax = _pMmax;
        pMcomment = _pMcomment;
        pMmodified = _pMmodified;
        pMaccepted = _pMaccepted;
        pMranking = _ranking;
    }
};

struct PubMessage{
    std::string pMname;
    std::string pMnetwork;
    std::string pMcomment;
    std::string pMmodified;
    std::string pMaccepted;
    QVector<MessageParameter*> v_pPubMparameters;

    PubMessage(std::string _pMname,
                std::string _pMnetwork,
                std::string _pMcomment,
                QVector<MessageParameter*> _pMparameters,
                std::string _pMmodified,
                std::string _pMaccepted)
  {
      pMname = _pMname;
      pMnetwork = _pMnetwork;
      pMcomment = _pMcomment;
      v_pPubMparameters = _pMparameters;
      pMmodified = _pMmodified;
      pMaccepted = _pMaccepted;
  }

};

struct EnumValue{
  std::string eNValname;
  std::string eNValvalue;
  std::string eNValcomment;
  std::string eNValmodified;
  std::string eNValaccepted;

  EnumValue(std::string _eNValname,
                std::string _eNValvalue,
                std::string _eNValcomment,
                std::string _eNValmodified,
                std::string _eNValaccepted)
  {
      eNValname = _eNValname;
      eNValvalue = _eNValvalue;
      eNValcomment = _eNValcomment;
      eNValmodified = _eNValmodified;
      eNValaccepted = _eNValaccepted;
  }
};

struct Enum{
    std::string eNname;
    std::string eNcomment;
    QVector<EnumValue*> v_pEnumValues;

    Enum(std::string _eNname,
        std::string _eNcomment,
        QVector<EnumValue*> _v_pEnumValues)
    {
        eNname = _eNname;
        eNcomment = _eNcomment;
        v_pEnumValues = _v_pEnumValues;
    }

};

struct StructParameter{
  std::string sPname;
  std::string sPtype;
  std::string sPunit;
  std::string sPdefault;
  std::string sPmin;
  std::string sPmax;
  std::string sPcomment;
  std::string sPmodified;
  std::string sPaccepted;


  StructParameter(std::string _sPname,
                    std::string _sPtype,
                    std::string _sPunit,
                    std::string _sPdefault,
                    std::string _sPmin,
                    std::string _sPmax,
                    std::string _sPcomment,
                    std::string _sPmodified,
                    std::string _sPaccepted)
  {
      sPname = _sPname;
      sPtype = _sPtype;
      sPunit = _sPunit;
      sPdefault = _sPdefault;
      sPmin = _sPmin;
      sPmax = _sPmax;
      sPcomment = _sPcomment;
      sPmodified = _sPmodified;
      sPaccepted = _sPaccepted;

  }
};

struct Struct{
  std::string sTname;
  std::string sTnetwork;
  std::string sTcomment;
  QVector<StructParameter*> v_pStructParameters;

  Struct(std::string _sTname,
                std::string _sTnetwork,
                std::string _sTcomment,
                QVector<StructParameter*> _sTparameters)
  {
      sTname = _sTname;
      sTnetwork = _sTnetwork;
      sTcomment = _sTcomment;
      v_pStructParameters = _sTparameters;
  }
};

struct ICD{

    QVector<SubMessage*> v_pSubscribedMessages;
    QVector<PubMessage*> v_pPublishedMessages;
    QVector<Enum*> v_pEnums;
    QVector<Struct*> v_pStructs;
    std::string name;
    std::string parsedName;
    QDateTime recentIcdDate;
};

struct DrawnModelObject{

    std::string name;
    std::string parsedName;
    QRectF rect;
    QLineF line;
    QLabel* label;
    QPointF midpoint;
    int linkedDataObjects;
};

struct DrawnDataObject{

    std::string name;
    DrawnModelObject* model;
    QLineF line;
    QPolygonF arrowPoly;
    QPushButton* label;
    std::string pubOrSub;
    std::string modified;
    std::string accepted;
    int posX;
    int posY;
};

struct DrawnTaskFunction{

    QString name;
    QString dir;
    QLineF line;
    QPolygonF arrowPoly;
    QPushButton* label;
    std::string modified;
    std::string accepted;
};

//const char* aAcceptedStatus[] ={"eToBeAssessed", "eAccepted","eRejected"};
//const char* aModifiedStatus[] ={"eNotModified","eRemoved", "eAdded","eModified"};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void ParseStockIcds();
    void ParseStdIcds();
    ICD ParseSingleStdIcd(QString _filePath);
    void SaveAllXMLs();
    void SaveOneXML(ICD _icd);

    void SetupFileDirectories();
    void SetupIcdMenu();
    void SetupDrawingArea();
    void SetupMessageBrowser();
    void SetupFunctionBrowser();
    void SetupTaskflowScene();
    void SetupContextMenu();
    void ResetScroll();

    void DrawUnsortedDiagram(ICD _icd);
    QPolygonF CreateArrowHead(QLineF arrowLine, bool rightPointing);
    QPolygonF CreateTaskArrowHead(QLineF arrowLine);

    void CheckFunctionSceneResize();
    void CheckUnsortedSceneResize();
    void CheckTaskflowSceneResize();

    void RedrawFunctionScene();
    void SaveFunctionScene();
    void LoadFunctionScene();
    void LoadCustomFunctionScene(QString _fileName);

    void RedrawTaskflowScene();

    Ui::MainWindow *ui;

public slots:
    void onListIcdClicked(QListWidgetItem *_item);
    void onScrollEvent(int value);
    void closeEvent(QCloseEvent *event);
    void onAddDataExchangeClicked();
    void onHomeTriggered();
    void onAddPilotInteraction();
    void onFunctionDropdownMenuClicked(QAction*_action);
    void onDataObjectClicked();
    void functionContextMenuRequested(const QPoint& point);
    void taskflowContextMenuRequested(const QPoint& point);
    void onRenameFunction();
    void onAddFunctionToTask();
    void onFunctionObjectClicked();
    void onTaskflowDropdownMenuClicked(QAction*_action);
    void onSaveTaskflowScene();
    void onLoadTaskflowScene();
    void onNewTaskflowScene()   { SetupTaskflowScene();         }
    void onTabChange(int _tab);
    void onCheckforChanges();
    void onHomeAllTriggered()   { ResetScroll();                }
    void onSaveFunction()       { SaveFunctionScene();          }
    void onLoadFunction()       { LoadFunctionScene();          }
    void onParseStdXmls()       { ParseStdIcds();
                                  SetupMessageBrowser();
                                  SetupIcdMenu();               }
    void onParseStockXmls()     { ParseStockIcds();
                                  SetupMessageBrowser();
                                  SetupIcdMenu();               }
    void onSaveXmlChanges()     { SaveAllXMLs();                }

    void onSaveToJPG();
    void onRevertToStockIcds()  { ParseStockIcds();
                                  SaveAllXMLs();
                                  onParseStdXmls();             }
    void onToggleFunctionStyle(){ toggleFunctionStyleOn =
                                    !toggleFunctionStyleOn;
                                  RedrawFunctionScene();        }
    void onToggleTaskStyle()    { toggleTaskStyleOn =
                                    !toggleTaskStyleOn;

                                  RedrawTaskflowScene();        }

    void onHorizontalSpacing();
    void onVerticalSpacing();
    void onFunctionComments();
    void onTaskComments();

private:
    QVector<ICD> v_ICDs;
    ICD selectedICD;

    QGraphicsScene* icdScene;
    QGraphicsView*  icdView;

    int horizontalSpacing;
    int verticalSpacing;
    int sceneUnsortedHorizontalSizing;
    int sceneUnsortedVerticalSizing;

    QGraphicsScene* functionScene;
    QGraphicsView*  functionView;

    QTreeWidget* icdBrowser;
    QVector<DrawnModelObject*> functionDrawnModels;
    QVector<DrawnDataObject*> functionDrawnData;
    int functionHorizontalSpacing;
    int functionVerticalSpacing;
    int sceneFunctionHorizontalSizing;
    int sceneFunctionVerticalSizing;
    QList<QTreeWidgetItem *> subItems;
    QList<QTreeWidgetItem *> pubItems;
    QList<QTreeWidgetItem *> pubOrSub;
    QMenu* functionDropdownMenu;
    QPushButton* functionSelectedButton;
    DrawnDataObject* functionSelectedDataObject;
    QString functionTitle;
    DrawnModelObject* pilotModelObject;
    DrawnModelObject* targetModelObject;
    bool toggleFunctionStyleOn;
    QString functionComments;

    QGraphicsScene* taskflowScene;
    QVector<DrawnTaskFunction*> taskflowDrawnFunctions;
    int taskflowVerticalSpacing;
    int sceneTaskflowVerticalSizing;
    QMenu* taskflowDropdownMenu;
    QPushButton* taskflowSelectedButton;
    DrawnTaskFunction* taskflowSelectedFunctionObject;
    QString taskTitle;
    bool toggleTaskStyleOn;
    QString taskComments;
};

#endif // MAINWINDOW_H
