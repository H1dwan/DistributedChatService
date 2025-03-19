#include "friendlabel.h"
#include "ui_friendlabel.h"

FriendLabel::FriendLabel(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::FriendLabel)
{
    ui->setupUi(this);

    ui->close_lb->SetState("normal", "hover", "pressed",
                           "selected_normal", "selected_hover", "selected_pressed");

    connect(ui->close_lb, &ClickedLabel::clicked, this, &FriendLabel::slot_close);
}

FriendLabel::~FriendLabel()
{
    delete ui;
}

/**
 * @brief FriendLabel::SetText
 *        为一个 FriendLabel 的 tip_lb 设置文本
 *        并根据文本内容和控件中的其他子控件（如 tip_lb 和 close_lb）调整控件的大小
 * @param text
 */
void FriendLabel::SetText(QString text)
{
    _text = text;
    ui->tip_lb->setText(_text);
    ui->tip_lb->adjustSize();

    // 确保布局更新
    // QApplication::processEvents();

    QFontMetrics fm(ui->tip_lb->font());    // 获取QLabel控件的字体信息
    auto textWidth = fm.horizontalAdvance(ui->tip_lb->text());  // 获取文本的宽度
    auto textHeight = fm.height();  // 获取文本的高度

    qDebug()<< " ui->tip_lb.width() is " << ui->tip_lb->width();
    qDebug()<< " ui->close_lb->width() is " << ui->close_lb->width();
    qDebug()<< " textWidth is " << textWidth;

    // 根据 tip_lb 和 close_lb 的宽度，以及文本高度，设置 FriendLabel 的固定宽度和高度
    this->setFixedWidth(ui->tip_lb->width() + ui->close_lb->width() + 5);
    this->setFixedHeight(textHeight + 2);

    qDebug()<< "  this->setFixedHeight " << this->height();
    _width = this->width();
    _height = this->height();
}

int FriendLabel::Width()
{
    return _width;
}

int FriendLabel::Height()
{
    return _height;
}

QString FriendLabel::Text()
{
    return _text;
}

void FriendLabel::slot_close()
{
    emit sig_close(_text);
}

