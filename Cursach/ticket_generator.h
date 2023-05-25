#ifndef TICKET_GENERATOR_H
#define TICKET_GENERATOR_H
#include <string>
#include <QStringList>

class TicketGenerator
{
public:
    TicketGenerator(std::string name, std::string surname, std::string fathername, std::string insurance,
                     std::string date, std::string time, QStringList doctor);
    int createTicket();
private:
    std::string _name;
    std::string _surname;
    std::string _fathername;
    std::string _insurance;

    std::string _doctorType;
    std::string _doctorName;
    std::string _doctorSurname;
    std::string _doctorFathername;
    std::string _date;
    std::string _time;

    //date
    std::string _day;
    std::string _month;
    std::string _year;

    //time
    std::string _hours;
    std::string _minutes;

    std::string  readTemplate();
};

#endif // TICKET_GENERATOR_H
