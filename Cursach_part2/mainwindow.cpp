#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidget>
#include <QDate>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->doctorSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(on_change_doctor_in_doctor_selector()));
    //connect to db
    db = QSqlDatabase::addDatabase("QPSQL", "mydb");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("qt_term_project");
    db.setUserName("timabd");
    db.setPassword("secure_password228"); //very secure indeed
    if (!db.open()) {
        qDebug() << db.lastError().text();
    } else {
        //name of the function says it all
        addDoctorsToDoctorSelector();
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addDoctorsToDoctorSelector() {
    QSqlQuery query(db);
    if (!query.exec("SELECT DISTINCT doc_type FROM doctors")) {
        qDebug() << query.lastError().text();
        return;
    }
    while (query.next()) {
        ui->doctorSelector->addItem(query.value(0).toString());
    }
}

void MainWindow::readDoctorsFromDB() {
    //read all doctors whose doc_type = the one in the doctorSelector
    QSqlQuery query(db);
    query.prepare("SELECT doc_id, doc_name, doc_surname, doc_fathername FROM doctors WHERE doc_type = :docType");
    query.bindValue(":docType", ui->doctorSelector->currentText());
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return;
    }
    //doctor: 0)doc_id 1)name 2)surname 3)fathername
    while (query.next()) {
        QStringList doctor;
        for (int i = 0; i < 4; i++) {
            doctor.append(query.value(i).toString());
        }
        docArray.append(doctor);
    }
}

void MainWindow::on_change_doctor_in_doctor_selector() {
    clearTabs();
    docArray.clear();
    readDoctorsFromDB(); // <- reads doctors into docArray
    //iterate through docArray - (create a doctor tab and add all of the appointments to it)
    //set tab label to be doc's name + surname + fathername (N.F.Surname)
    //querry for appointments by doc_id in docArray
    //these patient appointments are going to be rows in QTableWigdet sitting in a Tab in QTabWidget
    for (int i = 0; i < docArray.size(); i++) {
        //query first to determine the number of appointments
        QSqlQuery query(db);
        query.prepare("SELECT date, time, name, surname, fathername, insurance, attended, app_id "
                      "FROM appointments JOIN patients USING (patient_id) WHERE doc_id = :docId");
        query.bindValue(":docId", docArray[i][0]);
        if (!query.exec()) {
            qDebug() << query.lastError().text() << "here";
            qDebug() << query.executedQuery();
            return;
        }
        int numberOfAppointments = query.size();
        //qDebug() << "number of appointments is: " << numberOfAppointments;

        //doctor: 0)doc_id 1)name 2)surname 3)fathername
        QString tabLabel;
        QTextStream ts(&tabLabel);
        ts << docArray[i][1][0] << "." << docArray[i][3][0] << "." << docArray[i][2];

        //create a new table for i-th doctor
        QTableWidget *table = new QTableWidget;
        table->setColumnCount(7); // date, time, patient: name, surname, fathername, insurance, attended
        table->setHorizontalHeaderLabels(QStringList() << "Дата" << "Время" << "Имя" << "Фамилия" << "Отчество" << "Полис" << "Пациент пришел");
        table->setRowCount(numberOfAppointments);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        int table_index = 0;
        while (query.next()) {
            //appointment: 0)date 1)time 2)name 3)surname 4)fathername 5)insurance 6)attended 7)app_id
            QTableWidgetItem* date = new QTableWidgetItem();
            QTableWidgetItem* time = new QTableWidgetItem();
            QDate tempDate = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
            date->setText(tempDate.toString());
            QTime tempTime = QTime::fromString(query.value(1).toString(), Qt::ISODate);
            time->setText(tempTime.toString());
            QTableWidgetItem* name = new QTableWidgetItem(query.value(2).toString());
            QTableWidgetItem* surname = new QTableWidgetItem(query.value(3).toString());
            QTableWidgetItem* fathername = new QTableWidgetItem(query.value(4).toString());
            QTableWidgetItem* insurance = new QTableWidgetItem(query.value(5).toString());
            QTableWidgetItem* attended = new QTableWidgetItem((query.value(6).toString() == "false") ? "Нет" : "Да");
            //save the appointment id in the background
            //when updating the "attended" field use this id
            //when deleting an appointment use this id
            QVariant data(query.value(7).toString());
            attended->setData(Qt::UserRole, data);

            //populate table
            table->setItem(table_index, 0, date);
            table->setItem(table_index, 1, time);
            table->setItem(table_index, 2, name);
            table->setItem(table_index, 3, surname);
            table->setItem(table_index, 4, fathername);
            table->setItem(table_index, 5, insurance);
            table->setItem(table_index, 6, attended);
            table_index++;
        }
        ui->tabWidget->addTab(table, tabLabel);
    }
}

void MainWindow::clearTabs() {
    //empties all of the tabs in the ui
    int count = ui->tabWidget->count();
    qDebug() << count;
    for (int i = 0; i < count; i++) {
        delete ui->tabWidget->widget(0);
    }
}

void MainWindow::on_buttonAppointmentFinished_clicked() {
    QTableWidget* table = qobject_cast<QTableWidget*>(ui->tabWidget->currentWidget());
    int row = table->currentRow();
    QString appointmentId =  table->item(row, 6)->data(Qt::UserRole).toString();
    QSqlQuery query(db);
    query.prepare("UPDATE appointments SET attended = '1' WHERE app_id = :app_id");
    query.bindValue(":app_id", appointmentId);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
    table->item(row, 6)->setText("Да");
}

void MainWindow::on_buttonDeleteAppointment_clicked() {
    QTableWidget* table = qobject_cast<QTableWidget*>(ui->tabWidget->currentWidget());
    int row = table->currentRow();
    QString appointmentId =  table->item(row, 6)->data(Qt::UserRole).toString();
    QSqlQuery query(db);
    query.prepare("DELETE FROM appointments WHERE app_id = :app_id");
    query.bindValue(":app_id", appointmentId);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
    table->removeRow(row);
}

void MainWindow::on_buttonUpdateAppointmentInfo_clicked() {
    //remove litteraly everything
    clearTabs();
    docArray.clear();
    ui->doctorSelector->clear();
    //query again
    addDoctorsToDoctorSelector();
}
