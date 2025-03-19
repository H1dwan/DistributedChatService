#include "tcpmgr.h"
#include <QAbstractSocket>
#include <QJsonDocument>
#include "usermgr.h"

TcpMgr::~TcpMgr()
{

}

TcpMgr::TcpMgr() : _host(""), _port(0), b_recv_pending(false), _msg_id(0), _msg_len(0)
{
    // socket连接
    QObject::connect(&_socket, &QTcpSocket::connected, [this]() {
        qDebug() << "Connected to server!";
        emit sig_conn_success(true);
    });

    // 读取并解析 socket数据
    QObject::connect(&_socket, &QTcpSocket::readyRead, [this]() {
        // 当有数据可读时，读取所有数据并追加到缓冲区
        this->_buffer.append(this->_socket.readAll());
        QDataStream stream(&this->_buffer, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_6_8);

        forever
        {
            // 先解析 head
            if (!this->b_recv_pending)
            {
                 // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
                if (this->_buffer.size() < static_cast<qsizetype>(sizeof(qint16) * 2))
                {
                    return; // 数据不够，则等待更多数据
                }
                // 预读取消息ID和消息长度，但不从缓冲区中移除
                stream >> this->_msg_id >> this->_msg_len;
                // 输出读取的数据
                qDebug() << "Message ID:" << _msg_id << ", Length:" << _msg_len;
                 //将 buffer中的前四个字节移除
                this->_buffer.remove(0, sizeof(qint16) * 2);
            }

            // buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
            if (this->_buffer.size() < this->_msg_len)
            {
                this->b_recv_pending = true;
                return;
            }

            // 消息体完整，则读取后交由上层处理
            this->b_recv_pending = false;
            auto msg_body = this->_buffer.mid(0, this->_msg_len);
            this->_buffer.remove(0, this->_msg_len);
            handleMsg(ReqID(this->_msg_id), this->_msg_len, msg_body);
        }
    });

    // 错误捕获
    QObject::connect(&_socket, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError socketError) {
        qDebug() << "Socket error occurred:" << socketError;
        qDebug() << "Error:" << this->_socket.errorString();
    });

    // 处理断开连接
    QObject::connect(&_socket, &QTcpSocket::disconnected, []() {
         qDebug() << "Disconnected from server.";
    });

    // 连接信号用于发送数据
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);

    // 注册处理函数
    initHandlers();
}


void TcpMgr::initHandlers()
{
     // 注册获取聊天服务器登录的回包逻辑
    this->_handlers.insert(ReqID::ID_CHAT_LOGIN_RSP, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON 中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login failed, error is JSON parse error: " << err;
            emit sig_login_failed(err);
            return;
        }
        // 登录失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Login failed, error is: " << err;
            emit sig_login_failed(err);
            return;
        }

        // 登录成功
        qDebug() << "Login successed.";
        // 从回包中读取并保存用户信息
        auto uid = json_obj["uid"].toInt();
        auto gender = json_obj["gender"].toInt();
        auto name = json_obj["name"].toString();
        auto nickname = json_obj["nickname"].toString();
        auto icon = json_obj["icon"].toString();
        auto user_info = std::make_shared<UserInfo>(uid, gender, name, nickname, icon);
        UserMgr::GetInstance()->SetUserInfo(user_info);
        UserMgr::GetInstance()->SetToken(json_obj["token"].toString());
        // 保存好友申请列表信息（好友申请通知列表）
        if (json_obj.contains("apply_list"))
        {
            UserMgr::GetInstance()->AppendApplyList(json_obj["apply_list"].toArray());
        }
        // 保存好友列表信息
        if (json_obj.contains("friend_list")) {
            UserMgr::GetInstance()->AppendFriendList(json_obj["friend_list"].toArray());
        }
        // 跳转到聊天界面
        emit sig_switch_to_chat();
    });

    // 注册搜索用户信息的回包逻辑
    this->_handlers.insert(ReqID::ID_SEARCH_USER_RSP, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Search info failed, error is JSON parse error: " << err;
            emit sig_user_search(nullptr);
            return;
        }
        // 搜索用户失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Search info failed, error is: " << err;
            emit sig_user_search(nullptr);
            return;
        }
        // 搜索用户成功
        // 提取并封装好用户信息
        auto search_info = std::make_shared<SearchInfo>(json_obj["uid"].toInt(), json_obj["gender"].toInt(),
                                                        json_obj["name"].toString(), json_obj["nickname"].toString(),
                                                        json_obj["description"].toString(), json_obj["icon"].toString());
        emit sig_user_search(search_info);
    });

    // 注册添加好友的回包逻辑（添加别人为好友的回复即该申请是否送达到对方，sender）
    this->_handlers.insert(ReqID::ID_ADD_FRIEND_RSP, [](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Add Friend Failed, error is JSON parse error: " << err;
            return;
        }
        // 申请添加好友失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Add Friend Failed failed, error is: " << err;
            return;
        }
        qDebug() << "Add Friend REQ is sent!" ;
    });

    // 注册收到添加好友通知的请求逻辑（有人添加你为好友，receiver）
    this->_handlers.insert(ReqID::ID_NOTIFY_ADD_FRIEND_REQ, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify add friend failed, error is JSON parse error: " << err;
            return;
        }
        // 通知好友申请失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Notify add friend failed, error is: " << err;
            return;
        }

        // 通知成功，则取出好友申请数据，发送好友申请通知的信号
        int from_uid = json_obj["applyuid"].toInt();
        int gender = json_obj["gender"].toInt();
        QString name = json_obj["name"].toString();
        QString desc = json_obj["desc"].toString();
        QString icon = json_obj["icon"].toString();
        QString nick = json_obj["nickname"].toString();
        auto apply_info = std::make_shared<AddFriendApply>(from_uid, gender, name, nick, desc, icon);
        emit sig_friend_apply(apply_info);
    });

    // 注册收到添加好友同意的请求逻辑（对方同意添加你为好友，sender）
    this->_handlers.insert(ReqID::ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend failed, error is JSON parse error: " << err;
            return;
        }
        // 通知好友申请失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Auth Friend failed, error is: " << err;
            return;
        }

        // 添加好友同意成功，则取出数据，并通知各个界面
        int from_uid = json_obj["fromuid"].toInt();
        int gender = json_obj["gender"].toInt();
        QString name = json_obj["name"].toString();
        QString icon = json_obj["icon"].toString();
        QString nick = json_obj["nickname"].toString();
        auto auth_info = std::make_shared<AuthInfo>(from_uid, gender, name, nick, icon);
        emit sig_add_auth_friend(auth_info);
    });

    // 注册发送添加好友同意的回包逻辑
    this->_handlers.insert(ReqID::ID_AUTH_FRIEND_RSP, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Auth Friend Failed, error is JSON parse error: " << err;
            return;
        }
        // 通知好友申请失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Auth Friend Failed, error is: " << err;
            return;
        }

        // 添加好友成功，则取出数据，并通知各个界面
        int from_uid = json_obj["uid"].toInt();
        int gender = json_obj["gender"].toInt();
        QString name = json_obj["name"].toString();
        QString icon = json_obj["icon"].toString();
        QString nick = json_obj["nickname"].toString();
        auto rsp = std::make_shared<AuthRsp>(from_uid, gender, name, nick, icon);
        emit sig_auth_rsp(rsp);
        qDebug() << "Auth Friend Success " ;
    });

    // 注册发送文本消息成功的回包逻辑
    this->_handlers.insert(ReqID::ID_TEXT_CHAT_MSG_RSP, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Chat Msg Rsp Failed, error is JSON parse error: " << err;
            return;
        }
        // 消息发送失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Chat Msg Rsp Failed, error is: " << err;
            return;
        }
        // TODO: 设置消息已送达等标记
        qDebug() << "Receive Text Chat Rsp Success " ;
    });

    // 注册有新的文本消息待接收的逻辑
    this->_handlers.insert(ReqID::ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqID id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将数据转换成 JsonDocument
        QJsonDocument json_doc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (json_doc.isNull())
        {
            qDebug() << "Failed to create JSON document.";
            return;
        }
        // 取出 JSON对象
        auto json_obj = json_doc.object();
        // JSON中不包含 error字段
        if (!json_obj.contains("error"))
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Notify Chat Msg Failed, error is JSON parse error: " << err;
            return;
        }
        // 消息发送失败
        int err = json_obj["error"].toInt();
        if (err != ErrorCodes::SUCCESS)
        {
            qDebug() << "Notify Chat Msg Failed, error is: " << err;
            return;
        }

        qDebug() << "Receive Text Chat Notify Success " ;
        auto p_msg = std::make_shared<TextChatMsg>(json_obj["fromuid"].toInt(), json_obj["touid"].toInt(),
                                                   json_obj["text_array"].toArray());
        emit sig_text_chat_msg(p_msg);
    });
}


void TcpMgr::handleMsg(ReqID id, int len, QByteArray data)
{
    auto iter = this->_handlers.find(id);
    if (iter == this->_handlers.end())
    {
        qDebug() << "no matched handler for ReqID " << id;
        return;
    }

    iter.value()(id, len, data);
}


// 连接对端服务器
void TcpMgr::slot_tcp_connect(ServerInfo info)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到分配搭配的聊天服务器
    qDebug() << "Connecting to ChatServer...";
    this->_host = info.Host;
    this->_port = static_cast<uint16_t>(info.Port.toInt());
    this->_socket.connectToHost(this->_host, this->_port);
}


/**
 * @brief TcpMgr::slot_send_data
 * @param id    TCP的请求类型
 * @param data  需要发送的数据
 */
void TcpMgr::slot_send_data(ReqID id, QByteArray data)
{
    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(data.size());
    // 创建一个  QByteArray 用于存储要发送的所有数据
    QByteArray chunk;
    QDataStream out(&chunk, QIODevice::WriteOnly);
    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << static_cast<quint16>(id) << len;
    // 添加字节数据
    chunk.append(data);
    // 发送数据
    this->_socket.write(chunk);
    qDebug() << "tcp mgr send byte data is " << chunk;
}
