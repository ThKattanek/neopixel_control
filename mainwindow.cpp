#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPushButton>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("STM32 - NeoPixel Control");

    ui->disconnect->setEnabled(false);

    serial = new QSerialPort(0);
    port_info = new QSerialPortInfo();

    for(int i=0; i<port_info->availablePorts().length(); i++)
    {
        ui->ports->addItem(port_info->availablePorts()[i].portName());
    }

    ui->baudrate->addItems(QStringList()<<"9600"<<"19200"<<"38400"<<"115200");
    ui->baudrate->setCurrentIndex(3);
    ui->data_bits->addItems(QStringList()<<"5"<<"6"<<"7"<<"8");
    ui->data_bits->setCurrentIndex(3);
    ui->parity->addItems(QStringList()<<"None"<<"Even"<<"Odd"<<"Mark"<<"Space");
    ui->parity->setCurrentIndex(0);
    ui->stop_bits->addItems(QStringList()<<"1"<<"2");
    ui->stop_bits->setCurrentIndex(0);
    ui->flow_control->addItems(QStringList()<<"None"<<"RTS/CTS"<<"XON/XOFF");
    ui->flow_control->setCurrentIndex(0);

    ui->led_count->setValue(8);

    ui->serial_reciver_out->setStyleSheet("color: rgb(30, 230, 0);background-color: rgb(0, 25, 0);");

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();

    for(int i=0; i<data.length();i++)
    {
        unsigned char byte = data.at(i);

        if(byte != '\n')
        {
            SerialInText += byte;
        }
        else
        {
            ui->serial_reciver_out->setText(SerialInText);
            SerialInText.clear();
        }
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        on_disconnect_clicked();
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
    }
}

void MainWindow::on_connect_clicked()
{
    /// Werte holem
    bool isOK;

    int baud = QVariant(ui->baudrate->currentText()).toInt(&isOK);
    if(!isOK)
    {
        QMessageBox::information(this,"Fehler !","Fehlerhafte Eingabe der Baudrate !");
    }

    serial->setPortName(ui->ports->currentText());
    if(serial->open(QIODevice::ReadWrite))
    {
        if(serial->setBaudRate(baud)
                && serial->setDataBits((QSerialPort::DataBits)(ui->data_bits->currentIndex()+5))
                && serial->setParity((QSerialPort::Parity)ui->parity->currentIndex())
                && serial->setStopBits((QSerialPort::StopBits)(ui->stop_bits->currentIndex()+1))
                && serial->setFlowControl(QSerialPort::FlowControl(0))
                )
        {
            ui->serial_reciver_out->setText("Verbunden...");
            ui->disconnect->setEnabled(true);
            ui->connect->setEnabled(false);
            ui->led_count->setEnabled(false);

            led_count = ui->led_count->value();

            InitLedTable(ui->led_table, led_count);

        }
        else
        {
            serial->close();
            ui->serial_reciver_out->setText("Fehler: " + serial->errorString());
        }
    }
    else
    {
        serial->close();
        ui->serial_reciver_out->setText("Fehler: " + serial->errorString());
    }
}

void MainWindow::on_disconnect_clicked()
{
    serial->close();
    ui->serial_reciver_out->clear();
    ui->disconnect->setEnabled(false);
    ui->connect->setEnabled(true);
    ui->led_count->setEnabled(true);

    ClearLedTable(ui->led_table);
}

void MainWindow::SerialPackageSend(SERIAL_PACKAGE* ser_package)
{
    /* Package send begin */


    // claculate checksum //
    uint8_t* package = (uint8_t*)ser_package;
    ser_package->checksum = 0;

    for(int i=sizeof(ser_package->checksum); i<SERIAL_PACKAGE_LEN; i++)
    {
        ser_package->checksum += package[i];
    }

    serial->write((const char*)ser_package, SERIAL_PACKAGE_LEN);
    serial->waitForBytesWritten();

    /* Package send end */
}

void MainWindow::InitLedTable(QTableWidget *tbl, int led_count)
{
    tbl->setColumnCount(5);
    tbl->setRowCount(led_count);

    tbl->setHorizontalHeaderLabels(QStringList() << "LED" << "Red" << "Green" << "Blue" << "Color Dialog");

    for(int i=0; i<led_count; i++)
    {
        tbl->setCellWidget(i,0,new QLabel(QString::number(i)));
        tbl->setCellWidget(i,4,new QPushButton("..."));

        connect((QPushButton*)tbl->cellWidget(i,4),SIGNAL(clicked()),this, SLOT(on_color_dialog_clicked()));
    }
}

void MainWindow::ClearLedTable(QTableWidget *tbl)
{
    tbl->setColumnCount(0);
    tbl->setRowCount(0);
}

void MainWindow::on_TestButton_clicked()
{
    ser_package.command = SET_LED_COLOR;

    ser_package.led_number = 0;
    ser_package.color_r = 0x9f;
    ser_package.color_g = 0x00;
    ser_package.color_b = 0x00;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 1;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 2;
    ser_package.color_r = 0x00;
    ser_package.color_g = 0x9f;
    ser_package.color_b = 0x00;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 3;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 4;
    ser_package.color_r = 0x00;
    ser_package.color_g = 0x00;
    ser_package.color_b = 0x9f;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 5;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 6;
    ser_package.color_r = 0x9f;
    ser_package.color_g = 0x00;
    ser_package.color_b = 0x9f;
    SerialPackageSend(&ser_package);

    ser_package.led_number = 7;
    SerialPackageSend(&ser_package);

    ser_package.command = LEDS_UPDATE;
    SerialPackageSend(&ser_package);
}

void MainWindow::on_all_leds_off_clicked()
{
    SERIAL_PACKAGE ser_package;

    for(int i=0; i<8; i++)
    {
        ser_package.command = SET_LED_COLOR;
        ser_package.led_number = i;
        ser_package.color_r = 0x00;
        ser_package.color_g = 0x00;
        ser_package.color_b = 0x00;
        SerialPackageSend(&ser_package);
    }

    ser_package.command = LEDS_UPDATE;
    SerialPackageSend(&ser_package);
}

void MainWindow::on_color_dialog_clicked()
{
    current_select_led = ui->led_table->currentRow();

    QColorDialog *dlg = new QColorDialog(this);

    connect(dlg, SIGNAL(currentColorChanged(QColor)), this, SLOT(on_currentColorChanged(QColor)));

    dlg->exec();

    delete dlg;
}

void MainWindow::on_currentColorChanged(const QColor color)
{
    SERIAL_PACKAGE ser_package;

    int r,g,b;

    color.getRgb(&r,&g,&b);

    ser_package.command = SET_LED_COLOR;
    ser_package.color_r = r;
    ser_package.color_g = g;
    ser_package.color_b = b;

    if(current_select_led >= 3 && current_select_led <= 7)
    {
        for(int i=4; i<8; i++)
        {
            ser_package.led_number = i;
            SerialPackageSend(&ser_package);
        }
    }
    else
    {
        ser_package.led_number = current_select_led;
        SerialPackageSend(&ser_package);
    }

    ser_package.command = LEDS_UPDATE;
    SerialPackageSend(&ser_package);
}
