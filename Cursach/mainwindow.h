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
    void on_buttonMakeAppointment_clicked();
    void on_change_doctor_in_doctor_selector();
    void on_buttonUpdateDocInfo_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QVector<QStringList> docArray;

    void addDoctorsToDoctorSelector();
    void addPatientToDB(QString name, QString surname, QString fathername, QString insurance);
    void addAppointmentToDB(QString name, QString surname, QString fathername, QString insurance, QString doc_id, QString date, QString time);
    void addDoctorToDB(QString name, QString surname, QString fathername, QString doc_type, QString university, QString certificate);
    void insuranceExists(QString insurance);
    bool canMakeAppointment(QString docType);
    int doctorExistsInDB(QString name, QString surname, QString fathername, QString doc_type, QString university);
    void updateDocInfo(QString doc_id, QString certificate, QString university);
};
#endif // MAINWINDOW_H
