/******************************************************************************
 *
 * @file       registerdialog.h
 * @brief      注册界面
 *
 * @author     Heesoo
 * @date       2025/01/16
 * @history
 *****************************************************************************/
#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private:
    void initHttpHandlers();

    void showTip(QString str, bool b_ok);
    void _AddTipErr(TipError te, QString tips);
    void _DelTipErr(TipError te);
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPasswdValid();
    bool checkConfirmValid();
    bool checkVarifyValid();

    void ChangeRegPage();

private:
    Ui::RegisterDialog *ui;
    // 不同请求类型对应着不同的处理逻辑（函数）
    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
    // 存储注册界面所有未解决的 errors
    QMap<TipError, QString> _tip_errs;
    // 注册成功后返回登录界面的倒计时 timer
    QTimer* _shutdown_timer;
    // 倒计时 （5s）
    int _count_shutdown;

signals:
    void sig_switch_to_login();

private slots:
    void on_get_code_clicked();

    void slot_reg_mod_finish(ReqID id, QString res, ErrorCodes err);

    void on_confirm_btn_clicked();

    void on_return_btn_clicked();

    void on_cancel_btn_clicked();
};

#endif // REGISTERDIALOG_H
