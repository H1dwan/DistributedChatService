#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      主窗口：负责所有子窗口的切换和调度（所有子窗口的切换的槽函数均在此实现）
 *
 * @author     Heesoo
 * @date       2024/12/16
 * @history
 *****************************************************************************/

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LoginDialog *_login_dialog;
    RegisterDialog *_register_dialog;
    ResetDialog* _reset_dialog;
    ChatDialog* _chat_dialog;

public slots:
    void SlotSwitchReg();
    void SlotSwitchLoginFromReg();
    void SlotSwitchReset();
    void SlotSwitchLoginFromReset();
    void SlotSwitchChat();
};
#endif // MAINWINDOW_H
