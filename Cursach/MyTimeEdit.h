#ifndef MYTIMEEDIT_H
#define MYTIMEEDIT_H

#include <QTimeEdit>

class MyTimeEdit : public QTimeEdit {
    Q_OBJECT
public:
    MyTimeEdit(QWidget* parent = 0) {
    }

    virtual void stepBy(int steps)
    {
        if (this->time().minute() == 30 && steps > 0){
            setTime(QTime(time().hour() + 1, 0, time().second(), time().msec()));
        } else if(this->time().minute() == 0 && steps > 0){
            setTime(QTime(time().hour(), 30, time().second(), time().msec()));
        } else if (this->time().minute() == 30 && steps < 0){
            setTime(QTime(time().hour(), 0, time().second(), time().msec()));
        } else if(this->time().minute() == 0 && steps < 0){
            setTime(QTime(time().hour() - 1, 30, time().second(), time().msec()));
        } else {
            //QTimeEdit::stepBy(steps);
        }
    }
};

#endif // MYTIMEEDIT_H
