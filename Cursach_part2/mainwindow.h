#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QVector>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonAppointmentFinished_clicked();
    void on_buttonDeleteAppointment_clicked();
    void on_buttonUpdateAppointmentInfo_clicked();
    void on_change_doctor_in_doctor_selector();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QVector<QStringList> docArray; //doctor: 0)doc_id 1)name 2)surname 3)fathername

    void addDoctorsToDoctorSelector();
    void readDoctorsFromDB();
    void clearTabs();
};
#endif // MAINWINDOW_H
