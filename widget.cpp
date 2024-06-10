#include "widget.h"
#include "ui_widget.h"
#include "my_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("磁场强度采集设备");
    myStatus=toBeConnected;
    socket=new QTcpSocket;
    socket2=new QTcpSocket;
    setChart();
    //定时器
    timer = new QTimer(this);

    readTimer=new QTimer;
    //读取传感器的数据
    connect(socket,&QTcpSocket::readyRead,this,&Widget::readData);
    //读取经纬度信息
    connect(socket2,&QTcpSocket::readyRead,this,&Widget::readData2);



    connect(timer, &QTimer::timeout, this, &Widget::my_timeOut);
    //连接之前先读取数据
    connect(readTimer,&QTimer::timeout,this,&Widget::beforeRead);

    //断开
    connect(socket,&QTcpSocket::disconnected,this,&Widget::my_disconnect);

    connect(ui->confirmPushtButton,&QPushButton::clicked,this,&Widget::onButtonClicked);
    connect(ui->confirmPushtButton,&QPushButton::clicked,this,&Widget::onButtonClicked);
    connect(ui->pathselect,&QPushButton::clicked,this,&Widget::onButtonClicked);
    connect(ui->start,&QPushButton::clicked,this,&Widget::onButtonClicked);
    connect(ui->start_2,&QPushButton::clicked,this,&Widget::onButtonClicked);
    connect(ui->clear,&QPushButton::clicked,this,&Widget::onButtonClicked);


     QDir currentDir = QDir::currentPath();
     QString img1Path = currentDir.filePath("no.jpg");
     QString img2Path = currentDir.filePath("yes.jpg");
     img1.load(img1Path);
     img2.load(img2Path);
    ui->status1->setPixmap(QPixmap::fromImage(img1));
    ui->status1_2->setPixmap(QPixmap::fromImage(img1));
    ui->gps->setPixmap(QPixmap::fromImage(img1));
    ui->rtk->setPixmap(QPixmap::fromImage(img1));


}

void Widget::onButtonClicked() {
    // 获取触发槽函数的按钮对象
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    // 遍历所有按钮
    QList<QPushButton*> allButtons = findChildren<QPushButton*>();
    qDebug()<<allButtons;
    foreach (QPushButton *button, allButtons) {
        if (button != clickedButton) {
            // 如果按钮不是当前被点击的按钮，重置其属性和颜色
            button->setProperty("isPressed", false);
            button->setStyleSheet("");
        } else {
            // 如果按钮是当前被点击的按钮，更新其属性和颜色
            button->setProperty("isPressed", true);
            button->setStyleSheet("background-color: red");
        }
    }
}

Widget::~Widget()
{
    delete ui;
    socket->close();
    socket2->close();
}

void Widget::beforeRead(){
    sendData();
}
void Widget::my_timeOut(){
    sendData();
    updateChart();
    saveFile();
}

void Widget::my_disconnect(){
    timer->stop();
 QMessageBox::critical(this, "连接提示", "连接断开");
 ui->status1->setPixmap(QPixmap::fromImage(img1));
 ui->status1_2->setPixmap(QPixmap::fromImage(img1));
}

void Widget::setChart(){
    chart =new QChart();
//    chart->setTitle("磁场强度曲线");

    chart2 =new QChart();
//    chart2->setTitle("磁场强度曲线");

    seriesX=new QLineSeries;
    seriesY=new QLineSeries;
    seriesZ=new QLineSeries;
    seriesX->setName("X1");
    seriesY->setName("Y1");
    seriesZ->setName("Z1");

    seriesX2=new QLineSeries;
    seriesY2=new QLineSeries;
    seriesZ2=new QLineSeries;
    seriesX2->setName("X2");
    seriesY2->setName("Y2");
    seriesZ2->setName("Z2");
    //X轴
    axisX=new QValueAxis;
    axisX->setTickCount(10);
    axisX->setRange(0,num+10);
    axisX2=new QValueAxis;
    axisX2->setTickCount(10);
    axisX2->setRange(0,num+10);

    chart->addAxis(axisX,Qt::AlignBottom);
    chart2->addAxis(axisX2,Qt::AlignBottom);

    //Y轴
    axisY = new QValueAxis;
    axisY->setRange(-50000,60000);
    axisY2 = new QValueAxis;
    axisY2->setRange(-50000,60000);

    axisX->setGridLineVisible(false);
    axisY->setGridLineVisible(false);
    axisX2->setGridLineVisible(false);
    axisY2->setGridLineVisible(false);
    chart->addAxis(axisY,Qt::AlignLeft);
    chart2->addAxis(axisY2,Qt::AlignLeft);


    //加动画，更丝滑
//    chart->setAnimationOptions(QChart::SeriesAnimations);
//    chart2->setAnimationOptions(QChart::SeriesAnimations);
    //将曲线添加到图表上·
    chart->addSeries(seriesX);
    chart->addSeries(seriesY);
    chart->addSeries(seriesZ);
    chart2->addSeries(seriesX2);
    chart2->addSeries(seriesY2);
    chart2->addSeries(seriesZ2);
    //曲线对象关联X轴
    seriesX->attachAxis(axisX);
    seriesY->attachAxis(axisX);
    seriesZ->attachAxis(axisX);
    //曲线对象关联Y轴
    seriesX->attachAxis(axisY);
    seriesZ->attachAxis(axisY);
    seriesY->attachAxis(axisY);

    //曲线对象关联X轴
    seriesX2->attachAxis(axisX2);
    seriesY2->attachAxis(axisX2);
    seriesZ2->attachAxis(axisX2);
    //曲线对象关联Y轴
    seriesX2->attachAxis(axisY2);
    seriesZ2->attachAxis(axisY2);
    seriesY2->attachAxis(axisY2);
    ui->graphicsView->setChart(chart);
    ui->graphicsView_2->setChart(chart2);

}

//连接设备
void Widget::on_confirmPushtButton_clicked()
{
    QString myIP=ui->IPlineEdit->text();
    QString hostport=ui->hostlineEdit->text();
    QString myIP2=ui->IPlineEdit_2->text();
    QString hostport2=ui->hostlineEdit_2->text();
    //连接传感器
    socket->connectToHost(QHostAddress(myIP),hostport.toShort());
    //连接gps
    socket2->connectToHost(QHostAddress(myIP2),hostport2.toShort());


    if (socket->waitForConnected(3000)&&socket2->waitForConnected(3000)) {
            // 连接成功
            QMessageBox::information(this, "连接提示", "连接成功");
             ui->status1->setPixmap(QPixmap::fromImage(img2));
             ui->status1_2->setPixmap(QPixmap::fromImage(img2));

             readTimer->start(WAITIME);
             if(ui->path->toPlainText().isEmpty()){
                myStatus=toBeSavePath;
             }
             else{
                myStatus=toBeStart;
             }
        } else if(socket->waitForConnected(3000)||socket2->waitForConnected(3000)) {
            if(!socket->waitForConnected(3000)){
                // 连接失败，显示错误信息
                QMessageBox::critical(this, "连接错误","无法连接到传感器设备，请检查IP和端口是否正确。");
                ui->status1_2->setPixmap(QPixmap::fromImage(img2));
            }else{
                // 连接失败，显示错误信息
                QMessageBox::critical(this, "连接错误", "无法连接到定位设备，请检查IP和端口是否正确。");
                ui->status1->setPixmap(QPixmap::fromImage(img2));
            }


        }else{
            // 连接失败，显示错误信息
            QMessageBox::critical(this, "连接错误", "无法连接到传感器设备和定位设备，请检查IP和端口是否正确。");
        }


}

//改为正常顺序
QString swapHexHalfBytes(const QString &hexString) {
    if (hexString.length() != 8) {
        // Handle error, input string length should be 8
        qDebug() << "Error: Input string length should be 8";
        return hexString;
    }

    QString firstHalf = hexString.mid(0, 4); // Get first four characters
    QString secondHalf = hexString.mid(4, 4); // Get last four characters

    return secondHalf + firstHalf; // Swap and concatenate
}

void Widget::on_start_clicked()
{

    switch(myStatus){
    case toBeConnected:
         QMessageBox::critical(nullptr, "error", "请先连接设备");
        break;
    case toBeSavePath:
        QMessageBox::critical(nullptr, "error", "请设置好保存路径");
        break;
    case toBeStart:
        readTimer->stop();
        timer->start(WAITIME);
        break;

    }

}
//发送请求数据-传感器
void Widget::sendData(){
    QByteArray hexData = QByteArray::fromHex("0103000000104406");
    if (socket->state() == QAbstractSocket::ConnectedState){
        qint64 bytesWritten=socket->write(hexData);
        if (bytesWritten == -1) {
                qDebug() << "Failed to write to socket:" << socket->errorString();
                socket->close();
                return ;
            } else if (bytesWritten != hexData.size()) {
                qDebug() << "Could not write all the data to socket";
                socket->close();
                return;
            }

            qDebug() << "Modbus request sent successfully";

            // 等待数据返回
            if (!socket->waitForReadyRead(3000)) {
                qDebug() << "Modbus server response timeout";
                socket->close();
                return ;
            }
    }
}
//转为传感器的真实值
double toTrueValue(const QString &hexString){
    QString value=swapHexHalfBytes(hexString);
    bool ok;
    //解析到数码值
    double numValue = value.toInt(&ok, 16);
    qDebug()<<"numvalue is"<<numValue;
    //解析到真实电压值
    double volValue=(numValue/MAX_VALUE)*MY_RANGE_VALUE+MY_MIN_VALUE;
    //换算为传感器的真实值
    double trueValue=(SEN_RANGE_VALUE/MY_RANGE_VALUE)*(volValue-MY_MIN_VALUE)+SEN_MIN_VALUE;
    qDebug()<<"truevalue is"<<trueValue;
    return trueValue;
}
//读取数据
void Widget::readData(){

     QByteArray data=socket->readAll();
        //x1
     QString valueX1=data.mid(11,4).toHex();
     double x1=toTrueValue(valueX1);
        //y1
     QByteArray valueY1=data.mid(15,4).toHex();
     double y1=toTrueValue(valueY1);
        //z1
     QByteArray valueZ1=data.mid(19,4).toHex();
     double z1=toTrueValue(valueZ1);
        //x2
     QByteArray valueX2=data.mid(23,4).toHex();
     double x2=toTrueValue(valueX2);
        //y2
     QByteArray valueY2=data.mid(27,4).toHex();
     double y2=toTrueValue(valueY2);
        //z2
     QByteArray valueZ2=data.mid(31,4).toHex();
     double z2=toTrueValue(valueZ2);
     outputDatas.X1=x1;
     outputDatas.Y1=y1;
     outputDatas.Z1=z1;
     outputDatas.X2=x2;
     outputDatas.Y2=y2;
     outputDatas.Z2=z2;

}
float Widget::HEX_to_Float(QByteArray bytes,bool mode){
    unsigned char i;
    UF4 yy;
    UF8 xx;
    double d=0;
    if(bytes.size()==4){
        if(mode==true){
            for(i=0;i<4;i++)
                yy.U1[i]=bytes[i];
            d=yy.f;
        }
        else{
            unsigned char j=3;
            for(i=0;i<4;i++){
                yy.U1[j]=bytes[i];
                j--;
            }
            d=yy.f;
        }
    }
    return d;
}
double Widget::HEX_to_double(QByteArray bytes,bool mode){
    unsigned char i;
    UF8 xx;
    double d=0;
    if(bytes.size()==8){
        if(mode==true){
            for(i=0;i<8;i++)
                xx.U1[i]=bytes[i];
            d=xx.f;
        }
        else{
            unsigned char j=7;
            for(i=0;i<8;i++){
                xx.U1[j]=bytes[i];
                j--;
            }
            d=xx.f;
        }
    }
    return d;
}
void Widget::toDisplayStatus(uint8_t status){
    switch (status) {
    case 0:
        QMessageBox::critical(nullptr, "error", "无GPS模块连接或GPS故障");
        break;
    //GPS无信号
    case 1:
        ui->gps->setPixmap(QPixmap::fromImage(img1));
        ui->rtk->setPixmap(QPixmap::fromImage(img1));
        break;
    //2D定位
    case 2:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img1));
        break;
    //3D定位
    case 3:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img1));
        break;
    //DGPS/SBAS 辅助
    case 4:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img1));
        break;
    //RTK浮点解模式
    case 5:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img2));
        break;
    //RTK固定解模式
    case 6:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img2));
        break;
    //静态定点模式，通常用于基站
    case 7:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img2));
        break;
    //精密单点定位
    case 8:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img2));
        break;
    //双天线均为RTK固定解模式
    case 9:
        ui->gps->setPixmap(QPixmap::fromImage(img2));
        ui->rtk->setPixmap(QPixmap::fromImage(img2));
        break;
    }
}
void Widget:: readData2(){
    QByteArray responseData = socket2->readAll();
    uint8_t First_Receive;
    uint8_t Receive_1;
    uint8_t Receive_2;
    uint8_t Receive_3;
    First_Receive = responseData[0];
    QByteArray FdArray = responseData.mid(0, STATE_TYPE_LEN);
    if(First_Receive == 252){
        for(int i = 0; i < STATE_TYPE_LEN; i++){
            FdArray[i] = responseData[i];
        }
        Receive_1=FdArray[1];
        Receive_2=FdArray[2];
        Receive_3=FdArray[STATE_TYPE_LEN-1];
        if(Receive_1==TYPE_STATE&&Receive_2==STATE_LEN&&Receive_3==FRAME_END)
        {
            uint32_t combinedTime=0;
            uint8_t parttime;

            uint8_t Gnns_status=FdArray[9];
            Gnns_status=Gnns_status>>4;
            toDisplayStatus(Gnns_status);

            //时间
            for(int i=3;i>=0;i--){
                parttime=FdArray[11+i];
                combinedTime = combinedTime<<8 |parttime;
            }
            outputDatas.Unix_time=combinedTime;
            for(int i=3;i>=0;i--){
                parttime=FdArray[15+i];
                combinedTime = combinedTime<<8 |parttime;
            }
            outputDatas.Microseconds=combinedTime;
            //经纬度
            QByteArray temp_8 = responseData.mid(19, 8);
            double result = HEX_to_double(temp_8,true) * 180 / PI;
//
            QString outputString1;
            outputString1.setNum(result, 'f', 10);

            outputDatas.latitude = outputString1;
            ui->jingdu->setPlainText(outputString1);
            temp_8 = responseData.mid(27, 8);
            result = HEX_to_double(temp_8,true)* 180 / PI;
            QString outputString2;
            outputString2.setNum(result, 'f', 10);
            outputDatas.longitude = outputString2;
            ui->weidu->setPlainText(outputString2);
            //海拔
            temp_8 = responseData.mid(35, 8);
            outputDatas.height = HEX_to_double(temp_8,true);
            QString high=QString::number(outputDatas.height);
            ui->gaocheng->setPlainText(high);
            //速度
            QByteArray temp_4 = responseData.mid(43, 4);
            outputDatas.Velocity_north = HEX_to_Float(temp_4,true);
            temp_4 = responseData.mid(47, 4);
            outputDatas.Velocity_east = HEX_to_Float(temp_4,true);
            temp_4 = responseData.mid(51, 4);
            outputDatas.Velocity_down = HEX_to_Float(temp_4,true);
            //角度
            temp_4 = responseData.mid(71, 4);
            outputDatas.Roll = HEX_to_Float(temp_4,true);
            temp_4 = responseData.mid(75, 4);
            outputDatas.Pitch = HEX_to_Float(temp_4,true);
            temp_4 = responseData.mid(79, 4);
            outputDatas.Heading = HEX_to_Float(temp_4,true);
        }
    }
};
void Widget::updateChart(){


        seriesX->append(num, outputDatas.X1);
        seriesY->append(num, outputDatas.Y1);
        seriesZ->append(num, outputDatas.Z1);
        seriesX2->append(num,outputDatas.X2);
        seriesY2->append(num,outputDatas.Y2);
        seriesZ2->append(num,outputDatas.Z2);

        axisX->setMin(num-((60000/WAITIME)));
        axisX->setMax(num);
        axisX2->setMin(num-((60000/WAITIME)));
        axisX2->setMax(num);
        num++;

        qreal maxValue = std::numeric_limits<qreal>::lowest();
        qreal minValue = std::numeric_limits<qreal>::max();
        // 1遍历系列中的数据点，找到最大值
        foreach (const QPointF &point, seriesX->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        foreach (const QPointF &point, seriesY->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        foreach (const QPointF &point, seriesZ->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        int intMax = static_cast<int>(maxValue) + 10000;
        int intMin = static_cast<int>(minValue) - 10000;
        axisY->setMin(intMin);
        axisY->setMax(intMax);


        // 2遍历系列中的数据点，找到最大值
        foreach (const QPointF &point, seriesX2->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        foreach (const QPointF &point, seriesY2->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        foreach (const QPointF &point, seriesZ2->points()) {
            minValue = std::min(minValue, std::min(point.x(), point.y()));
            maxValue = std::max(maxValue, std::max(point.x(), point.y()));
        }
        int intMax2 = static_cast<int>(maxValue) + 10000;
        int intMin2 = static_cast<int>(minValue) - 10000;
        axisY2->setMin(intMin2);
        axisY2->setMax(intMax2);

}
void Widget::saveFile(){
    QString fileName = ui->path->toPlainText();

    if(fileName.isEmpty())
    {
        return;
    }
    else{
        QFile file(fileName);
        if (!file.open(QIODevice::Append | QIODevice::Text)) {
            QMessageBox::warning(this,"警告","无法保存为该文件，请检查！");
        }



        QTextStream out(&file);
        out<<outputDatas.Unix_time<<","<<outputDatas.Microseconds<<",";
        out<<outputDatas.X1<<","<<outputDatas.Y1<<","<<outputDatas.Z1<<","<<outputDatas.X2<<","<<outputDatas.Y2<<","<<outputDatas.Z2<<",";
        out<<outputDatas.longitude<<","<<outputDatas.latitude<<","<<outputDatas.height<<","<<outputDatas.Roll<<","<<outputDatas.Pitch<<",";
        out<<outputDatas.Heading<<","<<outputDatas.Velocity_north<<","<<outputDatas.Velocity_east<<","<<outputDatas.Velocity_down;
        out<<"\n";

        file.close();
        return;
    }
}


//选择保存路径
void Widget::on_pathselect_clicked()
{
    if(myStatus==toBeSavePath||myStatus==toBeStart){
        QString defaultFileFormat = "csv";
        QString defaultFileName = "output";
        QString defaultFileFullName = defaultFileName + "." + defaultFileFormat;

        QString fileName = QFileDialog::getSaveFileName(
            nullptr,
            "选择一个文件",
            defaultFileFullName,
            "CSV Files (*.csv);;All Files (*)"
            );

        ui->path->setPlainText(fileName);
        if(!ui->path->toPlainText().isEmpty()){
            myStatus=toBeStart;
        }

//        if(fileName.isEmpty())
//        {

//            QMessageBox::warning(this,"警告","请选择一个文件");
//        }
//        else
//        {
//            ui->path->setPlainText(fileName);
//            myStatus=toBeStart;

//        }
    }
    else{
        QMessageBox::critical(this, "error", "请设置好参数");
    }

}

//清除图表数据
void Widget::on_clear_clicked()
{
    seriesX->clear();
    seriesY->clear();
    seriesZ->clear();
    seriesX2->clear();
    seriesY2->clear();
    seriesZ2->clear();
}

//void Widget::on_pushButton_clicked()
//{
//    QString s1="D57C0001";
//    double s=toTrueValue(s1);
//    qSetRealNumberPrecision(10);
//    qDebug()<<s;
//}
//停止检测
void Widget::on_start_2_clicked()
{
    timer->stop();
}

