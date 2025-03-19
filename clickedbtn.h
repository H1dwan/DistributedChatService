#ifndef CLICKEDBTN_H
#define CLICKEDBTN_H

#include <QPushButton>

class ClickedBtn : public QPushButton
{
    Q_OBJECT
public:
    ClickedBtn(QWidget* parent = nullptr);
    ~ClickedBtn();
    void SetState(QString normal, QString hover, QString press);

protected:
    void enterEvent(QEnterEvent *event) override;       // 鼠标进入
    void leaveEvent(QEvent *event) override;            // 鼠标离开
    void mousePressEvent(QMouseEvent *e) override;      // 鼠标按下
    void mouseReleaseEvent(QMouseEvent *e) override;    // 鼠标释放

private:
    QString _normal;
    QString _hover;
    QString _press;
};

#endif // CLICKEDBTN_H
