#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum {SET_LED_COLOR = 0, LEDS_UPDATE};

struct SERIAL_PACKAGE
{
    uint32_t checksum;
    uint8_t command;
    uint8_t led_number;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
}typedef SERIAL_PACKAGE;

#define SERIAL_PACKAGE_LEN sizeof(SERIAL_PACKAGE)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readData();
    void handleError(QSerialPort::SerialPortError);

    void on_connect_clicked();
    void on_disconnect_clicked();
    void on_TestButton_clicked();
    void on_all_leds_off_clicked();
    void on_color_dialog_clicked();
    void on_currentColorChanged(const QColor);


private:
    Ui::MainWindow *ui;

    void SerialPackageSend(SERIAL_PACKAGE *ser_package);
    void InitLedTable(QTableWidget *tbl, int led_count);
    void ClearLedTable(QTableWidget *tbl);

    QSerialPortInfo *port_info;
    QSerialPort *serial;
    QString SerialInText;

    SERIAL_PACKAGE ser_package;
    uint16_t led_count;
    uint16_t current_select_led;
};
#endif // MAINWINDOW_H
