#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString head_addr, QString name, QString msg)
{
    _user_info = std::make_shared<UserInfo>(0, 0, name, name, head_addr, msg);
    // 设置头像
    QPixmap head_icon(head_addr);  // 加载头像图片
    ui->icon_lb->setPixmap(head_icon.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 截断文本，保留前面部分并添加省略号
    QFontMetrics fm(ui->user_name_lb->font());
    QString name_text = fm.elidedText(name, Qt::ElideRight, ui->user_name_lb->width());
    QFontMetrics fm1(ui->user_chat_lb->font());
    QString msg_text = fm1.elidedText(msg, Qt::ElideRight, ui->user_chat_lb->width());
    // 设置用户名和最近聊天记录
    ui->user_name_lb->setText(name_text);
    ui->user_chat_lb->setText(msg_text);
}

void ChatUserWid::SetInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    // 设置头像
    QPixmap head_icon(_user_info->icon);  // 加载头像图片
    ui->icon_lb->setPixmap(head_icon.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 截断文本，保留前面部分并添加省略号
    QFontMetrics fm(ui->user_name_lb->font());
    QString name_text = fm.elidedText(_user_info->name, Qt::ElideRight, ui->user_name_lb->width());
    QFontMetrics fm1(ui->user_chat_lb->font());
    QString msg_text = fm1.elidedText(_user_info->last_msg, Qt::ElideRight, ui->user_chat_lb->width());
    // 设置用户名和最近聊天记录
    ui->user_name_lb->setText(_user_info->name);
    ui->user_chat_lb->setText(_user_info->last_msg);
}

void ChatUserWid::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    SetInfo(std::make_shared<UserInfo>(friend_info));
}

std::shared_ptr<UserInfo> ChatUserWid::GetUserInfo()
{
    return _user_info;
}

void ChatUserWid::ShowRedPoint(bool show)
{
    if (show)
    {
        // ui->red_point->show();
    }
    else
    {
        // ui->red_point->hide();
    }
}

/**
 * @brief ChatUserWid::UpdateLastMsg    更新最后一条消息记录
 * @param msgs  保存在内存中的历史消息记录
 */
void ChatUserWid::UpdateLastMsg(std::vector<std::shared_ptr<TextChatData> > msgs)
{
    for (const auto& msg : msgs)
    {
        _user_info->chat_msgs.push_back(msg);
    }
    _user_info->last_msg = _user_info->chat_msgs.back()->msg_content;
    ui->user_chat_lb->setText(_user_info->last_msg);
    QFontMetrics fm1(ui->user_chat_lb->font());
    QString msg_text = fm1.elidedText(_user_info->last_msg, Qt::ElideRight, ui->user_chat_lb->width());
}
