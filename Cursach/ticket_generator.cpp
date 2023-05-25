#include "ticket_generator.h"
#include <fstream>
#include <string>
#include <sstream>
#include <QFile>

TicketGenerator::TicketGenerator(std::string name, std::string surname, std::string fathername, std::string insurance,
                                   std::string date, std::string time, QStringList doctor) {
    this->_name = name;
    this->_surname = surname;
    this->_fathername = fathername;
    this->_insurance = insurance;
    //doctor: 0)doc_id 1)name 2)surname 3)fathername 4)doc_type
    this->_doctorType = doctor[4].toStdString();
    this->_doctorName = doctor[1].toStdString();
    this->_doctorSurname = doctor[2].toStdString();
    this->_doctorFathername = doctor[3].toStdString();
    this->_date = date;
    this->_time = time;
    //date
    this->_day = date.substr(0, 2);
    this->_month = date.substr(3, 2);
    this->_year = date.substr(6, 2);
    //time
    this->_hours = time.substr(0, 2);
    this->_minutes = time.substr(3, 2);
}

std::string TicketGenerator::readTemplate() {
    //read appointment ticket template and return it as a string to fill out
    std::ifstream file;
    file.open("C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\ticket_template.txt");
    if (!file) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (!file.good()) {
        return "";
    }
    std::string str = buffer.str();
    file.close();
    return str;
}

int Ticket_generator::createTicket() {
    std::string ticketTemplate = TicketGenerator::readTemplate();
    if (ticketTemplate.empty()) {
        return 0;
    }

    //fill ticket template with name, surname, fathername, date and time, doctor
    int pos = ticketTemplate.find("ФИО:") + 7;
    ticketTemplate.insert(pos, " " + _surname + " " + _name.substr(0,2) + "." + _fathername.substr(0,2) + ".");
    pos = ticketTemplate.find("Полис:") + 11;
    ticketTemplate.insert(pos, " " + _insurance);
    pos = ticketTemplate.find("Дата:") + 9;
    ticketTemplate.insert(pos, " " + _date);
    pos = ticketTemplate.find("Время:") + 11;
    ticketTemplate.insert(pos, " " + _time);
    pos = ticketTemplate.find("Врач:") + 9;
    ticketTemplate.insert(pos, " " + _doctor_type + "\n      " + _doctorSurname + " " + _doctorName + " " + _doctorFathername );
    pos = ticketTemplate.find("Кабинет:") + 15;
    ticketTemplate.insert(pos, " 2281337");

    //make new file name based on persons name, surname, fathername, date and time of appointment, doctor info
    std::stringstream ss;
    ss <<"C:\\Users\\timab\\QtProjects\\Cursach\\resourses\\" << _surname << "_" << _name.substr(0,2) << "_" << _fathername.substr(0,2) <<
         "_"<< _doctorType << "_" << _doctorSurname << "_" << _doctorName.substr(0,2) << "_" << _doctorFathername.substr(0,2) <<
         "_" << _day << "_" << _month << "_" << _year << "_" << _hours << "_" << _minutes <<".txt";
    QString filename = QString::fromStdString(ss.str());

    //write new ticket to a file
    QFile outTicket(filename);
    outTicket.open(QIODevice::ReadWrite);
    QTextStream stream(&out_ticket);
    stream << QString::fromStdString(ticketTemplate);
    return 1;
}
