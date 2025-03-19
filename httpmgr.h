#ifndef HTTPMGR_H
#define HTTPMGR_H

/******************************************************************************
 *
 * @file       httpmgr.h
 * @brief      XXXX Function
 *
 * @author     Heesoo
 * @date       2024/12/17
 * @history
 *****************************************************************************/

#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>

#include "singleton.h"

// 继承 QObject 是为了使用QT的信号和槽函数
// 继承 Singleton<HttpMgr>用到了CRTP技术
// 继承 std::enable_shared_from_this<HttpMgr>能够安全地将 this指针转换为 shared_ptr
class HttpMgr : public QObject, public Singleton<HttpMgr>,
                public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
    // 声明 Singleton<HttpMgr>为友元类，从而可以访问 HttpMgr的私有成员和保护成员（用于单例模式中 shared_ptr的创建）；
    friend class Singleton<HttpMgr>;

public:
    ~HttpMgr();
    // 发送 HTTP的 POST请求
    void PostHttpReq(QUrl url, QJsonObject json, ReqID req_id, Modules mod);

private:
    HttpMgr();
    QNetworkAccessManager _manager;

signals:
    // http请求结束信号，res表示接收到的响应数据
    void sig_http_finish(ReqID id, QString res, ErrorCodes err, Modules mod);
    // 注册模块结束信号
    void sig_reg_mod_finish(ReqID id, QString res, ErrorCodes err);
    // 重置密码模块结束信号
    void sig_reset_mod_finish(ReqID id, QString res, ErrorCodes err);
    // 登录模块结束信号
    void sig_login_mod_finish(ReqID id, QString res, ErrorCodes err);

private slots:
    void slot_http_finish(ReqID id, QString res, ErrorCodes err, Modules mod);
};

#endif // HTTPMGR_H
