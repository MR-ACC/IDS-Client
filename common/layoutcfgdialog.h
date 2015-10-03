#ifndef LAYOUTCFGDIALOG_H
#define LAYOUTCFGDIALOG_H

#include <QDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QtAlgorithms>
#include "app_amp.h"

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
namespace Ui {
class layoutCfgDialog;
}

class layoutCfgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit layoutCfgDialog(QWidget *parent = 0);
    ~layoutCfgDialog();
    void paintEvent(QPaintEvent *event);
    void drawGrid(QPainter *painter);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    int idsUpdate(gpointer);
    gpointer                    mIdsEndpoint;
    IdsLayoutAll                mlayout;
    int                         mMsgRet;

private slots:
    void on_btnNewLayout_clicked();

    void on_btnDoneNew_clicked();

    void on_btnCancelNew_clicked();

    void on_comboBoxLayoutList_currentIndexChanged(int index);

    void on_comboBoxCameraType_currentIndexChanged(int index);

    void on_btnDel_clicked();

    void on_comboBoxChannel_currentIndexChanged(int index);

    void msgslot(int i);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_btnModify_clicked(bool checked);

private:
    Ui::layoutCfgDialog *ui;
    QPoint lastPoint;
    QPoint endPoint;
    QPixmap pix;
    QPixmap tempPix; //临时画布
    bool isDrawing;   //标志是否正在绘图
    int mMargin;//边缘
    int mWinCount;
    int mlayoutMatrix[IDS_LAYOUT_WIN_H][IDS_LAYOUT_WIN_W];
    bool isClick;
    bool isLayoutSwitch;
    bool isNewLayout;
    bool isChangeWinChannel;
    int mCurSelectedWin;
    int mCurSelectedLayout;
    QString mVidList[70];
    QBrush *mBrushEmpty;
    QBrush *mBrushFull;
    QPen *mPenNotSelected;
    QPen *mPenSelected;
    QPen *mPenGrid;
    QPen *mPenGridCenter;

signals:
    void sig(int i);
};

#endif // LAYOUTCFGDIALOG_H
