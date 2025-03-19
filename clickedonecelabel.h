#ifndef CLICKEDONECELABEL_H
#define CLICKEDONECELABEL_H

#include <QLabel>

class ClickedOneceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedOneceLabel(QWidget *parent=nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent *ev) override;

signals:
    void clicked(QString);
};

#endif // CLICKEDONECELABEL_H
