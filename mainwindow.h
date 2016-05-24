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

namespace Ui {
class MainWindow;
}

struct SubMessage{
  std::string sMicd;
  std::string sMmessage;

  SubMessage(std::string _sMicd, std::string _sMmessage){sMicd = _sMicd; sMmessage = _sMmessage;}
//  std::string GetsMicd(){return sMicd;}
//  std::string sMmessage(){return sMmessage;}

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
    int pMranking;

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
  QVector<MessageParameter*> v_pPubMparameters;

  PubMessage(std::string _pMname,
                std::string _pMnetwork,
                std::string _pMcomment,
                QVector<MessageParameter*> _pMparameters)
  {
      pMname = _pMname;
      pMnetwork = _pMnetwork;
      pMcomment = _pMcomment;
      v_pPubMparameters = _pMparameters;
  }
//  std::string GetpMname(){return pMname;}

};

struct EnumValue{
  std::string eNValname;
  std::string eNValvalue;
  std::string eNValcomment;
  std::string eNValmodified;
  std::string eNValaccepted;

  EnumValue(std::string _eNValname,
                std::string _eNValvalue,
                std::string _eNValcomment)
  {
      eNValname = _eNValname;
      eNValvalue = _eNValvalue;
      eNValcomment = _eNValcomment;
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
    int posX;
    int posY;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void ParseXML();
    void SetupIcdMenu();
    void SetupDrawingArea();
    void SetupMessageBrowser();
    void SetupContextMenu();
    void ResetScroll();

    void SaveAllSequenceDiagrams();
    void SetupFileDirectories();

    void DrawUnsortedDiagram(ICD _icd);
    QPolygonF CreateArrowHead(QLineF arrowLine, bool rightPointing);

    void CheckFunctionSceneResize();
    void CheckUnsortedSceneResize();

    void RedrawFunctionScene();
    void SaveFunctionScene();
    void LoadFunctionScene();

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
    void contextMenuRequested(const QPoint& point);
    void onSaveFunction(){SaveFunctionScene();}
    void onLoadFunction(){LoadFunctionScene();}
    void onRenameFunction();

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

};

#endif // MAINWINDOW_H
