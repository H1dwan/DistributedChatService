#include "chatpage.h"
#include "ui_chatpage.h"
#include <QStyleOption>
#include <QPainter>
#include <QUuid>
#include <QJsonDocument>
#include <chatitembase.h>
#include <textbubble.h>
#include <picturebubble.h>
#include "usermgr.h"
#include "tcpmgr.h"

ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);
    // 设置按钮样式
    ui->recv_btn->SetState("normal", "hover", "press");
    ui->send_btn->SetState("normal", "hover", "press");
    // 设置工具栏图标样式（emoji、file）
    ui->emoji_lb->SetState("normal", "hover", "press", "normal", "hover", "press");
    ui->file_lb->SetState("normal", "hover", "press", "normal", "hover", "press");
}

ChatPage::~ChatPage()
{
    delete ui;
}

/**
 * @brief ChatPage::SetUserInfo
 * @param user_info
 */
void ChatPage::SetUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    // 在聊天视图中加载对方用户名、最近聊天记录
    ui->title_lb->setText(_user_info->name);
    ui->chat_display_wid->RemoveAllItem();
    for (const auto& msg : _user_info->chat_msgs)
    {
        AppendChatMsg(msg);
    }
}

/**
 * @brief ChatPage::AppendChatMsg
 * @param msg
 */
void ChatPage::AppendChatMsg(std::shared_ptr<TextChatData> msg)
{
    // TODO: 在聊天视图中加载聊天记录
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role = msg->from_uid == self_info->uid ? ChatRole::Self : ChatRole::Other;

    // TODO:... 添加聊天显示
    if (role == ChatRole::Self)
    {
        auto* pChatItem = new ChatItemBase(role);
        pChatItem->SetUserName(self_info->name);
        pChatItem->SetUserIcon(QPixmap(self_info->icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->msg_content);
        pChatItem->SetWidget(pBubble);
        ui->chat_display_wid->AppendChatItem(pChatItem);
    }
    else
    {
        auto* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::GetInstance()->GetFriendByUID(msg->from_uid);
        if (!friend_info)
        {
            return;
        }
        pChatItem->SetUserName(friend_info->name);
        pChatItem->SetUserIcon(QPixmap(friend_info->icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->msg_content);
        pChatItem->SetWidget(pBubble);
        ui->chat_display_wid->AppendChatItem(pChatItem);
    }
}

// 让样式表在 ChatPage生效（自定义的，继承自 QWidget的类必须重写该函数）
void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    QWidget::paintEvent(event);
}

/**
 * @brief ChatPage::on_send_btn_clicked
 *        点击发送按钮的槽函数，会从 QTextEdit 中获取消息列表，并根据消息类型（文本、图片、文件）创建相应的聊天项，并将其添加到聊天显示区域
 */
void ChatPage::on_send_btn_clicked()
{
    if (_user_info == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }

    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role = ChatRole::Self;
    QString userName = self_info->name;
    QString userIcon = self_info->icon;
    // 从消息框获取消息列表
    auto pTextEdit = ui->chat_edit;
    const QVector<MsgInfo>& msgList = pTextEdit->GetMsgList();
    // 将消息发送给 ChatServer 并显示在聊天界面中
    QJsonObject textObj;
    QJsonArray textArray;
    qsizetype txt_size = 0;
    // 遍历消息列表
    for (qsizetype i = 0; i < msgList.size(); ++i)
    {
        // 消息内容长度不合规就跳过
        if (msgList[i].content.length() > 1024){
            continue;
        }

        // 创建 ChatItemBase 对象，并设置用户名和用户头像
        ChatItemBase* pChatItem = new ChatItemBase(role);
        pChatItem->SetUserName(userName);
        pChatItem->SetUserIcon(QPixmap(userIcon));
        // 根据消息类型创建气泡
        QString type = msgList[i].msgFlag;
        QWidget* pBubble = nullptr; // 基类指针指向派生类

        // 遍历消息列表
        if (type == "text")
        {
            // 生成唯一id
            QUuid uuid = QUuid::createUuid();
            QString uuid_str = uuid.toString();
            pBubble = new TextBubble(role, msgList[i].content);

            if (txt_size + msgList[i].content.length() > 1024)
            {
                textObj["fromuid"] = self_info->uid;
                textObj["touid"] = _user_info->uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                // 发送并清空之前累计的文本列表
                txt_size = 0;
                textObj = QJsonObject();
                textArray = QJsonArray();
                // 发送tcp请求给 ChatServer
                emit TcpMgr::GetInstance()->sig_send_data(ReqID::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            // 将 bubble 和 uid 绑定，以后可以等网络返回消息后设置是否送达
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuid_str;
            textArray.append(obj);
            auto txt_msg = std::make_shared<TextChatData>(self_info->uid, _user_info->uid, uuid_str,
                                                          obj["content"].toString());
            emit sig_append_send_chat_msg(txt_msg);
        }
        else if (type == "image")
        {
            pBubble = new PictureBubble(role, QPixmap(msgList[i].content));
        }
        else if (type == "file")
        {
            // TODO: 处理文件类型
        }

        // 在聊天页面显示发送消息
        if (pBubble != nullptr)
        {
            pChatItem->SetWidget(pBubble);
            ui->chat_display_wid->AppendChatItem(pChatItem);
        }
    }

    // 发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromuid"] = self_info->uid;
    textObj["touid"] = _user_info->uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    // 发送并清空之前累计的文本列表
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqID::ID_TEXT_CHAT_MSG_REQ, jsonData);

    // 清空输入框
    pTextEdit->clear();
}

