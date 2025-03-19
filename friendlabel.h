#ifndef FRIENDLABEL_H
#define FRIENDLABEL_H

#include <QFrame>

namespace Ui {
class FriendLabel;
}

class FriendLabel : public QFrame
{
    Q_OBJECT

public:
    explicit FriendLabel(QWidget *parent = nullptr);
    ~FriendLabel();

    void SetText(QString text);
    int Width();
    int Height();
    QString Text();

private:
    Ui::FriendLabel *ui;
    int _width;
    int _height;
    QString _text;

signals:
    void sig_close(QString);

public slots:
    void slot_close();
};

#endif // FRIENDLABEL_H
