#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "itrbase.h"
#include <QUdpSocket>
#include "IOControl.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void on_btStop_clicked();

    void on_btCapture_clicked();

    void on_btTrack_clicked();

    void on_btExit_clicked();

    void on_btIP_clicked();

    void processRecData(char* buffer,int length);
private:
    Ui::MainWindow *ui;
    itr_protocol::StandSerialProtocol sspUdp;
    RecThread recThread;
    float pos_x,pos_y,Area,fps;
    int mode;

    U8 imgbuffer[352*240*4];
    AVCodec *codec;
    AVCodecContext *dec;
    AVFrame *frame;
};

#endif // MAINWINDOW_H
