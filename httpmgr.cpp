#include "httpmgr.h"

HttpMgr::~HttpMgr()
{}

HttpMgr::HttpMgr() {
    // 连接 http请求和完成信号，信号槽机制保证队列消费
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

/**
 * @brief HttpMgr::PostHttpReq 用于发送 POST请求，并对响应数据通过信号进行通知
 * @param url: 资源定位符（位于 http包的请求行）
 * @param json: POST要传递的 JSON数据（http包中的 body）
 * @param req_id: 请求类型
 * @param mod: 发出请求的模块
 */
void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqID req_id, Modules mod)
{
    // 将 JSON数据转换为字节流（data是 request中的 body）
    QByteArray data = QJsonDocument(json).toJson();
    // 创建一个 request对象，通过 url构造请求
    QNetworkRequest request(url);
    // 设置请求头部信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    // 获取当前对象 HttpMgr的 shared_ptr
    auto self = shared_from_this();
    // 发送一个 POST请求
    QNetworkReply* reply = _manager.post(request, data);

    // 设置信号和槽等待发送完成
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
        // 处理错误情况
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        // 无错误则读回响应
        QString res = reply->readAll();

        // 发送信号通知完成
        emit self->sig_http_finish(req_id, res, ErrorCodes::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

/**
 * @brief HttpMgr::slot_http_finish     发送信号通知指定模块其 HTTP的响应结束了（一级分发到模块）
 * @param id    请求的类型
 * @param res   服务器回包的结果
 * @param err   错误码
 * @param mod   发出请求的模块
 */
void HttpMgr::slot_http_finish(ReqID id, QString res, ErrorCodes err, Modules mod)
{
    // 通知注册模块
    if (mod == Modules::REGISTERMOD) {
        emit sig_reg_mod_finish(id, res, err);
        return;
    }
    // 通知重置密码模块
    if (mod == Modules::RESETMOD) {
        emit sig_reset_mod_finish(id, res, err);
        return;
    }
    // 通知登录模块
    if (mod == Modules::LOGINMOD) {
        emit sig_login_mod_finish(id, res, err);
        return;
    }
}
