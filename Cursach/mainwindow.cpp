#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include "ticket_generator.h"
#include <QSqlError>
#include <QSqlQuery>
#include "filedownloader.h"
#include <QAxObject>
#include <QMovie>
#include <QScrollArea>
#include <QScrollBar>
/*
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //configure ui elements
    this->setFixedSize(QSize(1000, 550));
    ui->setupUi(this);
    ui->dateEdit->setMinimumDate(QDateTime(QDateTime::currentDateTime()).date());
    ui->timeEdit->setTime(QTime(8,0));
    ui->timeEdit->setMinimumTime(QTime(8,0));
    ui->timeEdit->setMaximumTime(QTime(19,0));
    delete ui->tabWidget->widget(1);
    delete ui->tabWidget->widget(0);
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(QRegularExpression("[0-9]*"), this);
    ui->insuranceLE->setValidator(validator);
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

void MainWindow::on_change_doctor_in_doctor_selector() {
    //delete all previous tabs
    int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        delete ui->tabWidget->widget(0);
    }
    //query for all doctors where doc_type = selected (in QComboBox) doc_type
    QSqlQuery query(db);
    query.prepare("SELECT doc_id, doc_name, doc_surname, doc_fathername, doc_type, university, certificate FROM doctors WHERE doc_type = :doc_type");
    query.bindValue(":doc_type", ui->doctorSelector->currentText());
    if (!query.exec()) {
            qDebug() << query.lastError().text();
    } else {
        //query will return:
        // 0) doc_id 1)doc_name 2)doc_surname 3)doc_fathername 4)doc_type 5)university 6)certificate
        //also save doctors in the docArray, if a doctor is chosen - pass needed doctor to TicketGenerator
        //because they will appear in docArray in the same order as in the tabs - it'll be easy, and no additional search is needed
        int i = 1;
        docArray.clear();
        while (query.next()) {
            //save the doctor to array
            QStringList temp;
            for (int j = 0; j < 7; j++) {
                temp.append(query.value(j).toString());
            }
            docArray.append(temp);
            //create a new tab
            QScrollArea *scrollWidget = new QScrollArea();
            QWidget* doctorWidget = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout();
            QLabel* img = new QLabel();
            QString text;
            //make a paragraph containing doctor information
            QTextStream ts(&text);
            QString university = query.value(5).toString();
            QString certificate = query.value(6).toString();
            ts << query.value(2).toString() << " " << query.value(1).toString() << " " << query.value(3).toString() << "\n" << query.value(4).toString()
               << "\n\nОбразование:\n" << university.section("\\", 0, 0) << "\n" << university.section("\\", 1, 1)<< "\n" << university.section("\\", 2, 2)
               << "\n\nСертификат:\n" << certificate.section("\\", 0, 0) << "\n" << certificate.section("\\", 1, 1)<< "\n" << certificate.section("\\", 2, 2);
            QPixmap* pixmap = new QPixmap();
            QString fathername = query.value(3).toString();
            if (fathername.endsWith("а")) {
                //load woman image
                pixmap->load(":/doctors/woman.png");
            } else {
                //load man image
                pixmap->load(":/doctors/man.png");
            }
            img->setPixmap(pixmap->scaledToHeight(200));
            layout->addWidget(img);
            QLabel *textLabel = new QLabel(text);
            textLabel->setWordWrap(true);
            layout->addWidget(textLabel);
            doctorWidget->setLayout(layout);
            doctorWidget->setFixedWidth(500);
            doctorWidget->setStyleSheet(
                "margin: 0px 25px 0px 0px;"
                );
            scrollWidget->setWidget(doctorWidget);
            scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            scrollWidget->setStyleSheet(
                "QScrollBar:vertical {"
                    "border: 2px solid #2B7081;"
                    "background: white;"
                    "width: 22px;"
                    "margin: 22px 0px 22px 0px;}"
                "QScrollBar::handle:vertical {"
                    "background: #9AD0DD;"
                    "min-height: 50px;}"
                "QScrollBar::add-line:vertical {"
                    "border: 2px solid #2B7081;"
                    "border-bottom-left-radius: 5px;"
                    //"border-bottom-right-radius: 5px;"
                    "background: #9AD0DD;"
                    "height: 20px;"
                    "subcontrol-position: bottom;"
                    "subcontrol-origin: margin;}"
                "QScrollBar::sub-line:vertical {"
                    "border: 2px solid #2B7081;"
                    "background: #9AD0DD;"
                    "border-top-left-radius: 5px;"
                    //"border-top-right-radius: 5px;"
                    "height: 20px;"
                    "subcontrol-position: top;"
                    "subcontrol-origin: margin;}"
                "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
                    "border: 1px solid #2B7081;"
                    "width: 7px;"
                    "height: 7px;"
                    "border-radius: 4px;"
                    "background: #62B6CB;}"
                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                    "background: none;}"
                );
            ui->tabWidget->addTab(scrollWidget, "Врач " + QString::number(i));
            //next doctor
            i++;
        }
    }
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

void MainWindow::addPatientToDB(QString name, QString surname, QString fathername, QString insurance) {
    //check if patient was put in DB from previous appointment
    QSqlQuery query(db);
    query.prepare("SELECT * FROM patients WHERE name = :name AND surname = :surname AND fathername = :fathername AND insurance = :insurance");
    query.bindValue(":name", name);
    query.bindValue(":surname", surname);
    query.bindValue(":fathername", fathername);
    query.bindValue(":insurance", insurance);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return;
    }

    //if there is no such patient, add them to DB
    if (!query.next()) {
        qDebug() << "empty query, adding patient to DB";
        query.prepare("INSERT INTO patients (name, surname, fathername, insurance) VALUES (:name, :surname, :fathername, :insurance)");
        query.bindValue(":name", name);
        query.bindValue(":surname", surname);
        query.bindValue(":fathername", fathername);
        query.bindValue(":insurance", insurance);
        if (!query.exec()) {
            qDebug() << query.lastError().text();
        }
    }
}

void MainWindow::addAppointmentToDB(QString name, QString surname, QString fathername, QString insurance, QString docId, QString date, QString time) {
    //addPatientToDB() gets called first always, so we assume that we 100% will find patient in DB already
    QString patientId;
    QSqlQuery query(db);
    query.prepare("SELECT patient_id FROM patients WHERE name = :name AND surname = :surname AND fathername = :fathername AND insurance = :insurance");
    query.bindValue(":name", name);
    query.bindValue(":surname", surname);
    query.bindValue(":fathername", fathername);
    query.bindValue(":insurance", insurance);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return;
    } else {
        query.next();
        patientId = query.value(0).toString();
    }

    //insert new appointment into DB;
    query.prepare("INSERT INTO appointments (doc_id, patient_id, date, time) VALUES (:doc_id, :patient_id, :date, :time)");
    query.bindValue(":doc_id", docId);
    query.bindValue(":patient_id", patientId);
    query.bindValue(":date", date);
    query.bindValue(":time", time);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
}

//add insurance check later if needed
void MainWindow::insuranceExists(QString insurance) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM patients WHERE insurance = :insurance");
    query.bindValue(":insurance", insurance);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return;
    }

    //if there is no such patient, everything is ok!
    if (!query.next()) {
        //add stuff to do if there is no such insurance
    }
}

void MainWindow::on_buttonMakeAppointment_clicked() {
    QString name = ui->nameLE->text();
    QString surname = ui->surnameLE->text();
    QString fathername = ui->fathernameLE->text();
    QString insurance = ui->insuranceLE->text();
    //retrieve doctor QStringList and pass it on to TicketGenerator
    QStringList doctor = docArray[ui->tabWidget->currentIndex()];
    //doctor: 0)doc_id 1)name 2)surname 3)fathername 4)doc_type 5)img_id
    //replace . with / to make queries easier later
    QString date = ui->dateEdit->date().toString("dd.MM.yy").replace(".","/");
    QString time = ui->timeEdit->time().toString("hh:mm");
    if(name.isEmpty() || surname.isEmpty() || fathername.isEmpty()) {
        QMessageBox::critical(this, "Ошибка!",
                              "Вы заполнили не все поля!\nПожалуйста, заполните все поля и нажмите \"Записаться на приём\".");
    } else if (insurance.size() != 16) {
        QMessageBox::critical(this, "Ошибка!",
                              "Неправильно указан полис!\nПожалуйста, заполните поле \"Полис\" и нажмите \"Записаться на приём\".");
    } else {
        addPatientToDB(name, surname, fathername, insurance);
        addAppointmentToDB(name, surname, fathername, insurance, doctor[0], date, time);
        TicketGenerator tg = TicketGenerator(name.toStdString(), surname.toStdString(), fathername.toStdString(),
                                               insurance.toStdString(), date.toStdString(), time.toStdString(), doctor);
        int err = tg.createTicket();
        qDebug() << err;
        QMessageBox::about(this, " ", "Вы успешно записаны!");
    }
}

bool MainWindow::canMakeAppointment(QString docType) {
    //check if a doctor type exists in DB - thus an appointment to this doctor can be made and we add/update doctor info in DB
    //check void MainWindow::on_buttonUpdateDocInfo_clicked() fow usage of this function fow the context
    QSqlQuery query(db);
    if (!query.exec("SELECT DISTINCT doc_type FROM doctors")) {
        qDebug() << query.lastError().text();
        return false;
    }
    while (query.next()) {
        if (query.value(0).toString() == docType) {
            return true;
        }
    }
    return false;
}

int MainWindow::doctorExistsInDB(QString name, QString surname, QString fathername, QString docType, QString university) {
    //return docs id if it exists, else return -1 (doc id can't be negative)
    QSqlQuery query(db);
    query.prepare("SELECT doc_id FROM doctors WHERE doc_name = :name AND doc_surname = :surname AND doc_fathername = :fathername "
                  "AND doc_type = :doc_type AND university = :university");
    query.bindValue(":name", name);
    query.bindValue(":surname", surname);
    query.bindValue(":fathername", fathername);
    query.bindValue(":doc_type", docType);
    query.bindValue(":university", university);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
    if (!query.next()) {
        return -1;
    } else {
        return query.value(0).toInt();
    }
}

void MainWindow::addDoctorToDB(QString name, QString surname, QString fathername, QString docType, QString university, QString certificate) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO doctors (doc_name, doc_surname, doc_fathername, doc_type, university, certificate) "
                  "VALUES (:doc_name, :doc_surname, :doc_fathername, :doc_type, :university, :certificate)");
    query.bindValue(":doc_name", name);
    query.bindValue(":doc_surname", surname);
    query.bindValue(":doc_fathername", fathername);
    query.bindValue(":doc_type", docType);
    query.bindValue(":university", university);
    query.bindValue(":certificate", certificate);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
}

void MainWindow::updateDocInfo(QString docId, QString certificate, QString university) {
    QSqlQuery query(db);
    query.prepare("UPDATE doctors SET certificate = :certificate, university = :university WHERE doc_id = :doc_id");
    query.bindValue(":certificate", certificate);
    query.bindValue(":university", university);
    query.bindValue(":doc_id", docId);
    if (!query.exec()) {
        qDebug() << query.lastError().text();
    }
}

void MainWindow::on_buttonUpdateDocInfo_clicked() {
    QDialog *dialog = new QDialog();
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    //QMovie *gif = new QMovie(":/gifs/loading.gif");
    //QLabel *GIFlabel = new QLabel();
    //GIFlabel->setMovie(gif);
    //gif->start();
    QLabel *text = new QLabel("Загрузка...");
    text->setAlignment(Qt::AlignCenter);
    text->setFixedSize(150,100);
    QFont font = text->font();
    font.setPointSize(20);
    text->setFont(font);
    //layout->addWidget(GIFlabel);
    layout->addWidget(text);
    dialog->setWindowTitle(" ");
    dialog->open();

    //download the file from the internet first
    //https://hosp15.com/doc/medrabotniki.xlsx
    //https://hospital20.spb.ru/data/editor/Spisok-dlya-sayta-ot-25.11.2019.pdf
    //it'll contain up to date doctor information
    FileDownloader downloader(this);
    downloader.download("https://hosp15.com/doc/medrabotniki.xlsx", "C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\doctors.xlsx");
    if (downloader.getLastError()) {
        //error occured
        dialog->done(0);
        QMessageBox::critical(this, "Ошибка!", "Не удалось выполнить http запрос для обновления информации.\n"
                                               "Возможно, вы не подключены к интернету или возникли проблемы на сервере.");
        return;
    }

    /*
    QSqlDatabase excel = QSqlDatabase::addDatabase("QODBC", "excel");
    excel.setDatabaseName("Driver={Microsoft Excel Driver (*.xls)}; DriverID=22; DBQ=" +
                          QString("C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\doctors.xlsx"));
                          //+ QString("C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\doctors.xlsx"));
    qDebug() << excel.isValid();
    if (!excel.open()) {
        QMessageBox::critical(this, "Ошибка!", "Не удается открыть файл с данными для обновления информации.");
        qDebug() << excel.lastError().text();
        qDebug() << excel.drivers();
        return;
    } else {
        qDebug() << "success";
    }

    [Microsoft][Диспетчер драйверов ODBC] Источник данных не найден и не указан драйвер, используемый по умолчанию QODBC: Unable to connect
    */
       /*
    QXlsx::Document excel("C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\testing.xlsx");
    if (excel.load()) {
        qDebug() << "succ";

    } else {
        QMessageBox::critical(this, "Ошибка!", "Не удается открыть файл с данными для обновления информации.");
        //return;
    }
    QXlsx::Cell* cell = excel.cellAt(1, 1); // get cell pointer.
    if (cell == nullptr) {
        qDebug() << "NULL";
    }
*/

    //QXlsx doesnt work because it cant load an excel file with 200+ rows :)
    //so use qaxcontainer :) (im crying...)
    QAxObject* excel = new QAxObject("Excel.Application", 0);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", "C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\doctors.xlsx");
    QAxObject* sheets = workbook->querySubObject("Worksheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);
    //get the number of rows and columns, standart stuff
    //if number of columns != 7, then additional info was added, so we can't parse a file
    QAxObject* usedRange = sheet->querySubObject("UsedRange");
    QAxObject* columns = usedRange->querySubObject("Columns");
    int columnCount = columns->property("Count").toInt();
    QAxObject* rows = usedRange->querySubObject("Rows");
    int rowCount = rows->property("Count").toInt();
    qDebug() << "ROWS " << rowCount;
    qDebug() << "COLS " << columnCount;
    if (columnCount != 7) {
        dialog->done(0);
        QMessageBox::critical(this, "Ошибка!", "Не удалось выполнить http запрос для обновления информации.\n"
                                               "Возможно, вы не подключены к интернету или возникли проблемы на сервере.");
        return;
        //show the same error cause why not lmao :)
    }

    //now iterate through spreadsheet
    //skip the first row, because it contains just column names
    for (int row = 1; row < rowCount; row++ ){
        QAxObject* position = sheet->querySubObject("Cells(int,int)", row + 1, 2);
        QString fullDocType = position->property("Value").toString().toLower();
        QString docType;

        //not all rows contain doctor info - some are about other hospital workers - laboratory workers/nurses/etc
        //also not all doctors can see patients - some of them are prescribed therapists and you can't make an appointment to them right away
        //thus check if a doctor can see patients with canMakeAppointment(QString docType)
        if (fullDocType.contains("врач-")) {
            docType = fullDocType.sliced(5);
        } else if (fullDocType.contains("врач - ")) {
            docType = fullDocType.sliced(7);
        } else {
            //row contained other persons info, not a doctor
            continue;
        }

        //update docs info
        if (canMakeAppointment(docType)) {
            QAxObject *FIO = sheet->querySubObject("Cells(int,int)", row + 1, 1);
            QString fullFIO = FIO->property("Value").toString();
            QString surname = fullFIO.section(' ', 0, 0);
            QString name = fullFIO.section(' ', 1, 1);
            QString fathername = fullFIO.section(' ', 2, 2);
            //get the university - can also update that (update the void MainWindow::updateDocInfo(QString docId, QString certificate) function)
            //but we assume that the university cant change
            QAxObject *uni = sheet->querySubObject("Cells(int,int)", row + 1, 3);
            QString university = uni->property("Value").toString();
            //get the certificate
            QAxObject *cert = sheet->querySubObject("Cells(int,int)", row + 1, 6);
            QString certificate = cert->property("Value").toString();

            int docId = doctorExistsInDB(name, surname, fathername, docType, university);
            if (docId == -1) {
                //doc doesnt exist, so add them
                //basicaly a new employee with the docType, such that an appointment can be made to them
                addDoctorToDB(name, surname, fathername, docType, university, certificate);
            } else {
                //just update the certificate info
                updateDocInfo(QString::number(docId), certificate, university);
            }
        } else {
            //cant make an appointment - so just skip the row
            continue;
        }
    }
    //close the excel
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    addDoctorsToDoctorSelector();
    ui->doctorSelector->setCurrentIndex(0);
    dialog->done(0);
}

