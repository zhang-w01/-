#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QTcpSocket>
#include<QHostAddress>
#include<QString>
#include<QMessageBox>
#include <QTimer>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include<QDebug>
#include <QFile>
#include <QtGlobal>
#include <QtMath>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTimeAxis>
#include <QAxObject>

//最大数码值
#define MAX_VALUE 262143
//量程范围
#define MY_RANGE_VALUE 20.00
//最小量程值
#define MY_MIN_VALUE -10.00
//传感器量程范围
#define SEN_RANGE_VALUE 200000
//传感器最小值
#define SEN_MIN_VALUE -100000
#define WAITIME 50 //在这里设置采样时间
#define STATE_LEN 0x64 //STATE数据长度
#define FRAME_HEAD 0xFC //数据帧头
#define FRAME_END 0xFD //数据帧尾
#define TYPE_STATE 0x50 //STATE数据类别
#define STATE_TYPE_LEN 108 //当数据类型为STATE时，该数组的总长度
#define PI 3.1415926

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE



class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_confirmPushtButton_clicked();
    void readData();
    void readData2();
    void on_start_clicked();
    void updateChart();

    void on_pathselect_clicked();
    void setChart();

    void on_clear_clicked();
    double HEX_to_double(QByteArray bytes,bool mode);
    float HEX_to_Float(QByteArray bytes,bool mode);
    void sendData();
    void my_timeOut();
    void saveFile();
    void my_disconnect();
    void beforeRead();
    void toDisplayStatus(uint8_t status);
//    void on_pushButton_clicked();
//    bool check();

    void on_start_2_clicked();
    void onButtonClicked();



private:
    Ui::Widget *ui;
    QTcpSocket* socket;
    QTcpSocket* socket2;
    QHostAddress* IP;
    QTimer *timer;
    QTimer *readTimer;
    QChart* chart;
    QChart* chart2;
    QLineSeries *seriesX;
    QLineSeries *seriesY;
    QLineSeries *seriesZ;
    QLineSeries *seriesX2;
    QLineSeries *seriesY2;
    QLineSeries *seriesZ2;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisX2;
    QValueAxis *axisY2;

    QImage img1;
    QImage img2;
    qreal num=0;
    struct opData{
        double X1=0,Y1=0,Z1=0;
        double X2=0,Y2=0,Z2=0;
        QString longitude=0,latitude=0;
        double height=0;
        float Roll=0,Pitch=0,Heading=0;
        float Velocity_north=0,Velocity_east=0,Velocity_down=0;
        uint32_t Unix_time=0,Microseconds=0;
        uint8_t GNSS_state=0;
    }outputDatas;
    enum status {
            //待连接状态
            toBeConnected,

            //待保存状态
            toBeSavePath,
            //待开始状态
            toBeStart
        };
    status myStatus;
};
//传感器浮点数解析类型
typedef union UF4
{
    unsigned char U1[4];
    float f;
}UF4;

typedef union UF8
{
    unsigned char U1[8];
    double f;
}UF8;
#endif // WIDGET_H
