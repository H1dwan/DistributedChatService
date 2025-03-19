#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);

    // 连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // 连接登录界面忘记密码信号
    connect(_login_dialog, &LoginDialog::sig_switch_to_reset, this, &MainWindow::SlotSwitchReset);
    // 连接 ChatServer登录成功跳转聊天界面信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_switch_to_chat, this, &MainWindow::SlotSwitchChat);
}

MainWindow::~MainWindow()
{
    delete ui;
    // if (_login_dialog) {
    //     delete _login_dialog;
    //     _login_dialog = nullptr;
    // }

    // if (_register_dialog) {
    //     delete _register_dialog;
    //     _register_dialog = nullptr;
    // }
}

void MainWindow::SlotSwitchReg()
{
    _register_dialog = new RegisterDialog(this);
    _register_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_register_dialog);
    _login_dialog->hide();
    _register_dialog->show();

    // 连接从注册界面返回登录界面的信号和槽函数
    connect(this->_register_dialog, &RegisterDialog::sig_switch_to_login, this, &MainWindow::SlotSwitchLoginFromReg);
}

/**
 * @brief MainWindow::SlotSwitchLoginFromReg    从注册界面返回登录界面
 */
void MainWindow::SlotSwitchLoginFromReg()
{
    // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);
    _register_dialog->hide();
    _login_dialog->show();

    // 连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dialog, &LoginDialog::sig_switch_to_reset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    _reset_dialog = new ResetDialog(this);
    _reset_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_reset_dialog);
    _login_dialog->hide();
    _reset_dialog->show();

    // 连接从重置密码界面返回登录界面的信号和槽函数
    connect(this->_reset_dialog, &ResetDialog::sig_switch_to_login, this, &MainWindow::SlotSwitchLoginFromReset);
}

/**
 * @brief MainWindow::SlotSwitchLoginFromReset  从重置密码界面返回登录界面
 */
void MainWindow::SlotSwitchLoginFromReset()
{
    // 创建一个CentralWidget, 并将其设置为 MainWindow的中心部件
    _login_dialog = new LoginDialog(this);
    _login_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dialog);
    _reset_dialog->hide();
    _login_dialog->show();

    // 连接登录界面注册信号
    connect(_login_dialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    connect(_login_dialog, &LoginDialog::sig_switch_to_reset, this, &MainWindow::SlotSwitchReset);
}

/**
 * @brief MainWindow::SlotSwitchChat    从登录界面切换到聊天界面
 */
void MainWindow::SlotSwitchChat()
{
    // 创建一个 CentralWidget, 并将其设置为 MainWindow的中心部件
    _chat_dialog = new ChatDialog(this);
    _chat_dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_chat_dialog);
    _login_dialog->hide();
    _chat_dialog->show();
    this->setMinimumSize(QSize(1050, 900));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
