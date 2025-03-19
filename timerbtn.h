#ifndef TIMERBTN_H
#define TIMERBTN_H
#include <QPushButton>
#include <QTimer>


class TimerBtn : public QPushButton
{
    friend class RegisterDialog;
public:
    TimerBtn(QWidget *parent);
    ~TimerBtn();
     // 重写 mouseReleaseEvent
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QTimer* _timer;
    int _counter;
    bool _email_ready;
};

#endif // TIMERBTN_H
