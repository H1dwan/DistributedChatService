/******************************************************************************
 *
 * @file       resetdialog.h
 * @brief      重置密码界面
 *
 * @author     Heesoo
 * @date       2025/01/16
 * @history
 *****************************************************************************/
#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

private:
    void _AddTipErr(TipError te, QString tips);
    void _DelTipErr(TipError te);
    void showTip(QString str, bool b_ok = false);
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPasswdValid();
    bool checkVarifyValid();

    void initHttpHandlers();

private:
    Ui::ResetDialog *ui;
    // 不同请求类型对应着不同的处理逻辑（函数）
    QMap<ReqID, std::function<void(const QJsonObject&)>> _handlers;
    // 存储重置密码界面所有未解决的 errors
    QMap<TipError, QString> _tip_errs;

signals:
    void sig_switch_to_login();

private slots:
    void slot_reset_mod_finish(ReqID id, QString res, ErrorCodes err);
    void on_sure_btn_clicked();
    void on_varify_btn_clicked();
    void on_return_btn_clicked();
};

#endif // RESETDIALOG_H
