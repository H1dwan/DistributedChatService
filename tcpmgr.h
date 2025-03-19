#ifndef TCPMGR_H
#define TCPMGR_H

#include <QObject>
#include <QTcpSocket>
#include "singleton.h"
#include "global.h"
#include "userdata.h"

struct SearchInfo;

class TcpMgr : public QObject, public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT;
    friend class Singleton<TcpMgr>;
public:
    ~TcpMgr();

private:
    TcpMgr();
    void initHandlers();
    void handleMsg(ReqID id, int len, QByteArray data);

private:
    QString _host;
    uint16_t _port;
    QTcpSocket _socket;
    QByteArray _buffer;
    bool b_recv_pending;
    qint16 _msg_id;
    qint16 _msg_len;
    QMap<ReqID, std::function<void(ReqID, int, QByteArray)>> _handlers;

signals:
    void sig_conn_success(bool b_successed);
    void sig_login_failed(int err);
    void sig_send_data(ReqID id, QByteArray data);
    void sig_switch_to_chat();

    void sig_user_search(std::shared_ptr<SearchInfo>);  // 查找用户

    void sig_friend_apply(std::shared_ptr<AddFriendApply>); // 收到添加好友通知
    void sig_add_auth_friend(std::shared_ptr<AuthInfo>);
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);
    void sig_text_chat_msg(std::shared_ptr<TextChatMsg> msg);

public slots:
    void slot_tcp_connect(ServerInfo info);
    void slot_send_data(ReqID id, QByteArray data);
};

#endif // TCPMGR_H
