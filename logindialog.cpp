#include "logindialog.h"
#include "ui_logindialog.h"
#include "clickedlabel.h"
#include "httpmgr.h"
#include "tcpmgr.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QComboBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    // TODO: 下拉
    ui->email_edit->setEditable(true);
    ui->email_edit->addItem("147258@163.com");
    ui->email_edit->addItem("123456@163.com");
    connect(ui->email_edit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &LoginDialog::slot_email_edit_changed);

    // 设置密码不可见
    this->ui->passwd_edit->setEchoMode(QLineEdit::Password);
     // 连接切换到注册界面信号和槽函数
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    // 设置 label的状态
    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    // 连接切换到忘记密码界面信号和槽函数
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    // 初始化头像
    this->initHead();

    // 注册处理回包的逻辑
    initHttpHandlers();
    // 连接登录回包信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this ,
            &LoginDialog::slot_login_mod_finish);
    // 连接 TCP连接请求的信号和槽
    connect(this, &LoginDialog::sig_tcp_connect, TcpMgr::GetInstance().get(),
            &TcpMgr::slot_tcp_connect);
    // 连接 tcp管理者发出的 TCP连接成功信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_conn_success, this,
            &LoginDialog::slot_tcp_conn_finish);
    // 连接 tcp管理者发出的登录失败信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this,
            &LoginDialog::slot_login_failed);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDialog!";
    delete ui;
}

/**
 * @brief LoginDialog::initHead     在 head_label上显示圆角头像
 */
void LoginDialog::initHead()
{
    // TODO: 理解这段代码
    // 加载图片
    QPixmap originalPixmap(":/res/husky.jpg");
    // 设置图片自动缩放
    qDebug() << originalPixmap.size() << ui->head_label->size();
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // 创建一个和原始图片大小相同透明的 Pixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent);
    //
    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);  // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    // 使用 QPainterPath设置圆角或圆形
    QPainterPath path;
    // path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10);
    int diameter = qMin(originalPixmap.width(), originalPixmap.height());
    path.addEllipse((originalPixmap.width() - diameter) / 2, (originalPixmap.height() - diameter) / 2,
                    diameter, diameter);
    painter.setClipPath(path);
    // 将原始图片绘制到 roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);
    // 将绘制好的圆角图片贴到 head_label上
    ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::_AddTipErr(TipError te, QString tips)
{
    this->_tip_errs[te] = tips;
    showTip(tips);
}

void LoginDialog::_DelTipErr(TipError te)
{
    this->_tip_errs.remove(te);
    // 如果没有待解决的错误，则清空错误提示信息
    if (this->_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }
    showTip(this->_tip_errs.first());
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

bool LoginDialog::checkEmailValid()
{
    auto email = ui->email_edit->currentText();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    // 执行正则表达式匹配
    bool match = regex.match(email).hasMatch();
    if (!match) {
        _AddTipErr(TipError::TIP_EMAIL_ERROR, tr("邮箱地址不正确"));
        return false;
    }
    _DelTipErr(TipError::TIP_EMAIL_ERROR);
    return true;
}

bool LoginDialog::checkPasswdValid()
{
    auto passwd = ui->passwd_edit->text();

    // 1.检查密码长度（是否已经包含在了正则表达式中呢）
    if (passwd.length() < 6 || passwd.length() > 15) {
        _AddTipErr(TipError::TIP_PWD_ERROR, tr("密码长度应为6~15"));
        return false;
    }

    _DelTipErr(TipError::TIP_PWD_ERROR);
    return true;
}

void LoginDialog::initHttpHandlers()
{
    // 注册登录界面登录回包的逻辑
    this->_handlers.insert(ReqID::ID_USER_LOGIN, [this](const QJsonObject& jsonObj) {
        // 错误处理
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            qDebug()<< "error code is " << error;
            showTip(tr("参数错误"));
            enableBtn(true);
            return;
        }

        qDebug() << "user is " << jsonObj["user"].toString();
        showTip(tr("登录成功"), true);

        // 获取得到的 uid 和 token令牌
        this->_uid = jsonObj["uid"].toInt();
        this->_token = jsonObj["token"].toString();

        qDebug() << jsonObj;

        // 发送信号通知 TcpMgr发送长连接
        ServerInfo info {jsonObj["host"].toString(), jsonObj["port"].toString(),
                        this->_token, this->_uid};
        qDebug()<< "user is " <<  jsonObj["user"].toString() << " uid is " << info.Uid <<" host is "
                 << info.Host << " Port is " << info.Port << " Token is " << info.Token;
        emit sig_tcp_connect(info);
    });
}

bool LoginDialog::enableBtn(bool enabled)
{
    this->ui->login_btn->setEnabled(enabled);
    this->ui->reg_btn->setEnabled(enabled);
    return true;
}

void LoginDialog::slot_forget_pwd()
{
    qDebug() << "slot forget pwd";
    emit sig_switch_to_reset();
}

void LoginDialog::slot_login_mod_finish(ReqID id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        qDebug()<< "error code is " << err;
        showTip(tr("网络请求错误！"));
        return;
    }

    // 解析 JSON字符串，将 res响应转化为 QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json解析失败
    if (jsonDoc.isNull()) {
        showTip(tr("JSON解析失败！"));
        return;
    }
    // json解析错误
    if (!jsonDoc.isObject()) {
        showTip(tr("JSON解析失败！"));
        return;
    }

    // json解析成功，根据请求类型 RepID调用对应的处理逻辑
    _handlers[id](jsonDoc.object());
    return;
}

/**
 * @brief LoginDialog::slot_tcp_conn_finish     TCP建立成功后，便执行与 ChatServer的登录
 * @param b_successed
 */
void LoginDialog::slot_tcp_conn_finish(bool b_successed)
{
    if (!b_successed)
    {
        showTip(tr("网络异常"),false);
        enableBtn(true);
        return;
    }

    // tcp连接成功后，根据 uid 和 token和 ChatServer进行连接
    showTip(tr("聊天服务连接成功，正在登录..."),true);
    QJsonObject json_obj {
        {"uid", this->_uid},
        {"token", this->_token}
    };
    QJsonDocument json_doc(json_obj);
    QByteArray json_data = json_doc.toJson();
    //发送 tcp请求给 ChatServer
    emit TcpMgr::GetInstance()->sig_send_data(ReqID::ID_CHAT_LOGIN, json_data);
}


void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登录失败，err is %1").arg(err);
    showTip(result,false);
    enableBtn(true);
}


void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"login btn clicked";
    // 检查邮箱，密码合法性
    if (!checkEmailValid()) {
        return;
    }
    if (!checkPasswdValid()) {
        return;
    }

    enableBtn(false);

    // 发送 HTTP请求登录
    QJsonObject json_obj {
        {"email", ui->email_edit->currentText()},
        {"passwd", xoString(ui->passwd_edit->text())},
    };
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json_obj, ReqID::ID_USER_LOGIN, Modules::LOGINMOD);
}

void LoginDialog::slot_email_edit_changed(int index)
{
    ui->passwd_edit->setText("123456");
}

