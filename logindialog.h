#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void initHead();

    void _AddTipErr(TipError te, QString tips);
    void _DelTipErr(TipError te);
    void showTip(QString str, bool b_ok = false);
    bool checkEmailValid();
    bool checkPasswdValid();

    void initHttpHandlers();
    // 使能登录按钮
    bool enableBtn(bool);


private:
    Ui::LoginDialog *ui;
    // 不同请求类型对应着不同的处理逻辑（函数）
    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
    // 存储登录界面所有未解决的 errors
    QMap<TipError, QString> _tip_errs;
    // 用户的 UID
    int _uid;
    // 用于跟 ChatServer认证的令牌
    QString _token;

signals:
    void switchRegister();
    // 切换到重置密码界面信号
    void sig_switch_to_reset();
    // 发送 tcp长连接的信号
    void sig_tcp_connect(ServerInfo info);

private slots:
    void slot_forget_pwd();
    void slot_login_mod_finish(ReqID id, QString res, ErrorCodes err);
    void slot_tcp_conn_finish(bool b_successed);
    void slot_login_failed(int err);
    void on_login_btn_clicked();
    void slot_email_edit_changed(int index);
};

#endif // LOGINDIALOG_H
