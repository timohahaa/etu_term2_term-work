#ifndef TICKET_GENERATOR_H
#define TICKET_GENERATOR_H
#include <string>
#include <QStringList>

class Ticket_generator
{
public:
    Ticket_generator(std::string name, std::string surname, std::string fathername, std::string insurance,
                     std::string date, std::string time, QStringList doctor);
    int create_ticket();
private:
    std::string _name;
    std::string _surname;
    std::string _fathername;
    std::string _insurance;

    std::string _doctor_type;
    std::string _doctor_name;
    std::string _doctor_surname;
    std::string _doctor_fathername;
    std::string _date;
    std::string _time;

    //date
    std::string _day;
    std::string _month;
    std::string _year;

    //time
    std::string _hours;
    std::string _minutes;

    std::string  read_template();
};

#endif // TICKET_GENERATOR_H
