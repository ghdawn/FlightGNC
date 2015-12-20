#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "IOControl.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QPainter>

SSPSend sspSend;
SSPReceivefuc sspRec;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->imagewidget->installEventFilter(this);

    sspRec.setpara(&mode,&pos_x,&pos_y,&Area,&fps);
    sspUdp.Init(0xA5,0x5A,&sspSend);
    sspUdp.AddDataRecFunc(&sspRec,0);

    connect(&recThread,SIGNAL(OnDataReceive(char*,int)),this,SLOT(processRecData(char*,int)));
    recThread.Init();
    recThread.start();

    avcodec_register_all();
    codec = avcodec_find_decoder(CODEC_ID_H264);
    dec = avcodec_alloc_context3(codec);
    if (avcodec_open2(dec, codec,NULL) < 0) {
        fprintf(stderr, "ERR: open H264 decoder err\n");
        exit(-1);
    }

    frame = avcodec_alloc_frame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processRecData(char* buffer, int length)
{
    const int SSPLength=18+6;
    static int count=0;
    char filename[]="x264/x2640000000.x264";
    {
        sspUdp.ProcessRawByte((U8*)buffer,SSPLength);
        QString str=QString("x:%1,y:%2\nfps:%3\n").arg(pos_x).arg(pos_y).arg(fps);
        ui->lbInfo->setText(str);
    }

    {
        int got;
        AVPacket pkt;
        pkt.data = (U8*)buffer+SSPLength;
        pkt.size = length-SSPLength;
        int ret = avcodec_decode_video2(dec, frame, &got, &pkt);
        if(got<=0)
            return;

        int k=0;
        int rgb[320*240];
        //        yuv2rgb(frame->data[],rgb,320,240);
        //        char *p =(char*) rgb;
        for(int j=0;j<240;j++)
            for(int i=0;i<320;i++)
            {
                imgbuffer[4*k  ]=frame->data[0][(i+j*352)];
                imgbuffer[4*k+1]=frame->data[0][(i+j*352)];
                imgbuffer[4*k+2]=frame->data[0][(i+j*352)];
                imgbuffer[4*k+3]=frame->data[0][(i+j*352)];
                //                imgbuffer[4*k  ]=*p++;
                //                imgbuffer[4*k+1]=*p++;
                //                imgbuffer[4*k+2]=*p++;
                //                imgbuffer[4*k+3]=*p++;
                k++;
            }
        ui->imagewidget->update();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    char filename[30];
    const int radius=20;
    const int crosslength=8;
    const int width=320;
    const int height=240;

    if(obj==ui->imagewidget)
    {
        if(e->type()==QEvent::Paint)
        {
            QImage img=QImage(imgbuffer,width,height,QImage::Format_RGB32);
            QPainter painter(ui->imagewidget);
            painter.drawImage(QPoint(0,0),img);
            painter.setPen(Qt::red);
            if(mode==2||mode==3)
            {
                painter.drawRect(pos_x-radius,pos_y-radius,radius+radius,radius+radius);
                painter.drawLine(pos_x-crosslength,pos_y,pos_x+crosslength,pos_y);
                painter.drawLine(pos_x,pos_y-crosslength,pos_x,pos_y+crosslength);
                painter.drawLine(pos_x,pos_y,width/2,height/2);

            }
            else
                painter.drawRect(140,100,40,40);
            return true;
        }
    }

}

void MainWindow::on_btStop_clicked()
{
    U8 buffer[2]={0x41,0};
    sspUdp.SSPSendPackage(0,buffer,2);
}

void MainWindow::on_btCapture_clicked()
{
    U8 buffer[2]={0x41,1};
    sspUdp.SSPSendPackage(0,buffer,2);
}

void MainWindow::on_btTrack_clicked()
{
    U8 buffer[2]={0x41,2};
    sspUdp.SSPSendPackage(0,buffer,2);
}

void MainWindow::on_btExit_clicked()
{
    U8 buffer[2]={0x41,3};
    sspUdp.SSPSendPackage(0,buffer,2);
}

void MainWindow::on_btIP_clicked()
{
    sspSend.udppackage.IP=ui->tbIP->toPlainText().toStdString();
}
