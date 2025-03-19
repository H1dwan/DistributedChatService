#include "resetdialog.h"
#include "ui_resetdialog.h"
#include "httpmgr.h"

ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog)
{
    ui->setupUi(this);

     // 添加 editingFinished信号和处理逻辑，验证输入合法性
    connect(ui->user_edit, &QLineEdit::editingFinished, this, [this]() {
        checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
        checkEmailValid();
    });
    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this]() {
        checkPasswdValid();
    });
    connect(ui->varify_edit, &QLineEdit::editingFinished, this, [this]() {
        checkVarifyValid();
    });

    // 连接 reset相关信号和注册处理回调，处理 HTTP回包
    initHttpHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish,
            this, &ResetDialog::slot_reset_mod_finish);

}

ResetDialog::~ResetDialog()
{
    qDebug() << "destruct ResetDialog!";
    delete ui;
}

void ResetDialog::_AddTipErr(TipError te, QString tips)
{
    this->_tip_errs[te] = tips;
    showTip(tips);
}

void ResetDialog::_DelTipErr(TipError te)
{
    this->_tip_errs.remove(te);
    // 如果没有待解决的错误，则清空错误提示信息
    if (this->_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }
    showTip(this->_tip_errs.first());
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

bool ResetDialog::checkUserValid()
{
    if (ui->user_edit->text() == "") {
        _AddTipErr(TipError::TIP_USER_ERROR, tr("用户名不能为空"));
        return false;
    }
    _DelTipErr(TipError::TIP_USER_ERROR);
    return true;
}

bool ResetDialog::checkEmailValid()
{
    auto email = ui->email_edit->text();
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

bool ResetDialog::checkPasswdValid()
{
    auto passwd = ui->pwd_edit->text();

    // 1.检查密码长度（是否已经包含在了正则表达式中呢）
    if (passwd.length() < 6 || passwd.length() > 15) {
        _AddTipErr(TipError::TIP_PWD_ERROR, tr("密码长度应为6~15"));
        return false;
    }

    // 2.检查密码是否合法
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(passwd).hasMatch();
    if (!match) {
        _AddTipErr(TipError::TIP_PWD_ERROR, tr("不能包含非法字符"));
        return false;
    }

    _DelTipErr(TipError::TIP_PWD_ERROR);
    return true;
}

bool ResetDialog::checkVarifyValid()
{
    auto verify_code = ui->varify_edit->text();

    if (verify_code == "") {
        _AddTipErr(TipError::TIP_VARIFY_ERROR, tr("验证码不能为空"));
        return false;
    }

    _DelTipErr(TipError::TIP_VARIFY_ERROR);
    return true;
}

void ResetDialog::initHttpHandlers()
{
    // 注册重置密码界面获取验证码回包的逻辑
    this->_handlers.insert(ReqID::ID_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            qDebug()<< "error code is " << error;
            showTip(tr("参数错误"));
            return;
        }
        qDebug() << "email is " <<  jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱，注意查收"), true);
    });

    // 注册重置密码界面重置密码回包的逻辑
    this->_handlers.insert(ReqID::ID_RESET_PWD, [this](const QJsonObject& jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            qDebug()<< "error code is " << error;
            showTip(tr("参数错误"));
            return;
        }
        qDebug() << "email is " <<  jsonObj["email"].toString();
        qDebug()<< "user uid is " <<  jsonObj["uid"].toString();
        showTip(tr("重置成功，点击返回登录"), true);
    });
}

void ResetDialog::slot_reset_mod_finish(ReqID id, QString res, ErrorCodes err)
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

void ResetDialog::on_varify_btn_clicked()
{
    qDebug()<<"receive varify btn clicked";
    // 检查邮箱是否合法
    if (!checkEmailValid()) {
        return;
    }

    // 发送 HTTP请求获取验证码
    QJsonObject json_obj {{"email", ui->email_edit->text()}};
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_verifycode"), json_obj,
                      ReqID::ID_GET_VERIFY_CODE, Modules::RESETMOD);
}

void ResetDialog::on_sure_btn_clicked()
{
    qDebug()<<"sure btn clicked";
    // 再次检查输入是否合法
    if (!checkUserValid()) {
        return;
    }
    if (!checkEmailValid()) {
        return;
    }
    if (!checkPasswdValid()) {
        return;
    }
    if (!checkVarifyValid()) {
        return;
    }

    // 发送 HTTP重置密码
    QJsonObject json_obj {
        {"user", ui->user_edit->text()},
        {"email", ui->email_edit->text()},
        {"passwd", ui->pwd_edit->text()},
        {"varifycode", ui->varify_edit->text()},
    };
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/reset_pwd"), json_obj,
                                        ReqID::ID_RESET_PWD, Modules::RESETMOD);
}

void ResetDialog::on_return_btn_clicked()
{
    qDebug() << "sure btn clicked";
    emit sig_switch_to_login();
}

