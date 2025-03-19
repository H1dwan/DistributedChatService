#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QWidget>
#include <QLabel>
#include "global.h"

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void SetState(QString normal = "", QString hover = "", QString press = "",
                  QString select = "", QString select_hover = "", QString select_press = "");
    void SetSelected(bool be_selected);
    ClickLbState GetCurState();
    void ClearState();

    void AddRedPoint();
    void ShowRedPoint(bool show = true);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;
    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _cur_state;

    QLabel* _red_point;

signals:
    void clicked();
};

#endif // STATEWIDGET_H
