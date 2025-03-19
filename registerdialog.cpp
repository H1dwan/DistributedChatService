#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QRandomGenerator>
#include "global.h"
#include "httpmgr.h"
#include "timerbtn.h"


RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog),
    _shutdown_timer(new QTimer(this)), _count_shutdown(5)
{
    ui->setupUi(this);

    // 设置为密码模式
    ui->passwd_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    // 设置眼睛图标可见
    ui->pwd_visible->SetState("unvisible","unvisible_hover","","visible",
                              "visible_hover","");
    ui->confirm_visible->SetState("unvisible","unvisible_hover","","visible",
                                  "visible_hover","");
    // 连接点击事件（切换密码可见与不可见）
    connect(ui->pwd_visible, &ClickedLabel::clicked, this, [this]() {
        if (this->ui->pwd_visible->GetCurState() == ClickLbState::Normal) {
            ui->passwd_edit->setEchoMode(QLineEdit::Password);
        } else {
            ui->passwd_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "label was clicked";
    });
    connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
        if (this->ui->confirm_visible->GetCurState() == ClickLbState::Normal) {
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
        } else {
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
        }
        qDebug() << "label was clicked";
    });

    // 设置错误提示的初始状态（后续可用repolish根据状态进行刷新）
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);

    // 初始化
    initHttpHandlers();
    // 连接 sig_reg_mod_finish信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish,
            this, &RegisterDialog::slot_reg_mod_finish);

    // 用于检查输入是否合法
    ui->err_tip->clear();
    // 添加 editingFinished信号和处理逻辑
    connect(ui->user_edit, &QLineEdit::editingFinished, this, [this]() {
        this->checkUserValid();
    });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() {
        this->checkEmailValid();
    });
    connect(ui->passwd_edit, &QLineEdit::editingFinished, this, [this]() {
        this->checkPasswdValid();
    });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this]() {
        this->checkConfirmValid();
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this]() {
        this->checkVarifyValid();
    });

    // 连接注册成功后返回登录界面的倒计时 timer的槽函数
    connect(_shutdown_timer, &QTimer::timeout, this, [this]() {
        this->_count_shutdown--;
        if (this->_count_shutdown <= 0) {
            this->_count_shutdown = 5;
            this->_shutdown_timer->stop();
            // 发射返回登录界面的信号
            emit sig_switch_to_login();
            return;
        }
        // 显示剩余时间
        auto tip_str = QString("注册成功，%1 s后自动返回登录...").arg(this->_count_shutdown);
        ui->tip_lb->setText(tip_str);
    });

}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "destruct RegisterDialog!";
    delete ui;
}

/**
 * @brief RegisterDialog::on_get_code_clicked
 * UI中获取验证码按钮的回调函数
 * 通过 HttpMgr向 GateServer发送获取验证的 POST请求
 */
void RegisterDialog::on_get_code_clicked()
{
    // 获取邮箱
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    // 执行正则表达式匹配
    bool match = regex.match(email).hasMatch();
    if (match) {
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_verifycode"),
                                            json_obj, ReqID::ID_GET_VERIFY_CODE, Modules::REGISTERMOD);
        // 通知倒计时功能，邮箱正确
        this->ui->get_code->_email_ready = true;
    } else {
        showTip(tr("邮箱地址不正确！"), false);
    }
}


/**
 * @brief RegisterDialog::slot_reg_mod_finish
 * @param id
 * @param res   GateServer的回报数据
 * @param err
 */
void RegisterDialog::slot_reg_mod_finish(ReqID id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        qDebug()<< "error code is " << err;
        showTip(tr("网络请求错误！"), false);
        return;
    }
    // 解析 JSON字符串，将 res响应转化为 QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json解析失败
    if (jsonDoc.isNull()) {
        showTip(tr("json解析失败！"), false);
        return;
    }
    // json解析错误
    if (!jsonDoc.isObject()) {
        showTip(tr("json解析失败！"), false);
        return;
    }

    // json解析成功，根据请求类型 RepID调用对应的处理逻辑
    _handlers[id](jsonDoc.object());
    return;
}

/**
 * @brief RegisterDialog::initHttpHandlers  处理 HTTP 回复的具体逻辑
 * 1. 获取验证码的回包逻辑
 * 2. 用户注册的回包逻辑
 */
void RegisterDialog::initHttpHandlers()
{
    // 注册注册界面获取验证码回包的逻辑
    _handlers.insert(ReqID::ID_GET_VERIFY_CODE, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            qDebug()<< "error code is " << error;
            showTip(tr("参数错误！"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱，注意查收！"), true);
        qDebug() << "email is " << email;
    });

    // 注册用户注册回包逻辑
    _handlers.insert(ReqID::ID_REG_USER, [this](const QJsonObject& jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            qDebug()<< "error code is " << error;
            showTip(tr("参数错误！"), false);
            return;
        }

        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug()<< "user id is " << jsonObj["uid"].toString() ;
        qDebug()<< "email is " << email ;
        ChangeRegPage();
    });
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::_AddTipErr(TipError te, QString tips)
{
    this->_tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::_DelTipErr(TipError te)
{
    this->_tip_errs.remove(te);
    if (this->_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }
    // 展示未解决TIP中的第一个；
    showTip(_tip_errs.first(), false);
}

bool RegisterDialog::checkUserValid()
{
    if (ui->user_edit->text() == "") {
        _AddTipErr(TipError::TIP_USER_ERROR, tr("用户名不能为空"));
        return false;
    }
    _DelTipErr(TipError::TIP_USER_ERROR);
    return true;
}

bool RegisterDialog::checkEmailValid()
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

bool RegisterDialog::checkPasswdValid()
{
    auto passwd = ui->passwd_edit->text();

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

bool RegisterDialog::checkConfirmValid()
{
    auto passwd = ui->passwd_edit->text();
    auto confirm = ui->confirm_edit->text();

    if (confirm != passwd) {
        _AddTipErr(TipError::TIP_PWD_CONFIRM, tr("密码和确认密码不匹配"));
        return false;
    }

    _DelTipErr(TipError::TIP_PWD_CONFIRM);
    return true;
}

bool RegisterDialog::checkVarifyValid()
{
    auto verify_code = ui->verify_edit->text();

    if (verify_code == "") {
        _AddTipErr(TipError::TIP_VARIFY_ERROR, tr("验证码不能为空"));
        return false;
    }

    _DelTipErr(TipError::TIP_VARIFY_ERROR);
    return true;
}

/**
 * @brief RegisterDialog::ChangeRegPage 注册成功后，从注册界面跳转到过渡界面（注册与登录的过渡界面）
 */
void RegisterDialog::ChangeRegPage()
{
    this->_shutdown_timer->stop();
    // 切换到过渡界面
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    // 启动定时器，设置间隔为1000毫秒（1秒）
    this->_shutdown_timer->start(1000);
}

// 注册界面确认按钮的槽函数
void RegisterDialog::on_confirm_btn_clicked()
{

    if (ui->user_edit->text() == "")
    {
        showTip(tr("用户名不能为空！"), false);
        return;
    }

    if (ui->email_edit->text() == "")
    {
        showTip(tr("邮箱不能为空！"), false);
        return;
    }

    if (ui->passwd_edit->text() == "")
    {
        showTip(tr("密码不能为空！"), false);
        return;
    }

    if (ui->confirm_edit->text() == "")
    {
        showTip(tr("确认密码不能为空！"), false);
        return;
    }

    if (ui->passwd_edit->text() != ui->confirm_edit->text())
    {
        showTip(tr("密码和确认密码不匹配！"), false);
        return;
    }

    if (ui->verify_edit->text() == "")
    {
        showTip(tr("验证码不能为空！"), false);
        return;
    }

    // 发送 HTTP 请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xoString(ui->passwd_edit->text());
    json_obj["confirm"] = xoString(ui->confirm_edit->text());
    json_obj["varifycode"] = ui->verify_edit->text();

    // TODO: 测试用代码
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成 0 到 99 之间的随机整数
    int head_i = randomValue % heads.size();
    json_obj["icon"] = heads[head_i];
    json_obj["nickname"] = ui->user_edit->text();


    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_register"), json_obj,
                                        ReqID::ID_REG_USER, Modules::REGISTERMOD);
}

/**
 * @brief RegisterDialog::on_return_btn_clicked
 * 所属 UI: 过渡界面（注册成功后的）
 * 功能：返回登录界面
 */
void RegisterDialog::on_return_btn_clicked()
{
    this->_shutdown_timer->stop();
    emit sig_switch_to_login();
}

/**
 * @brief RegisterDialog::on_cancel_btn_clicked
 * 所属 UI: 注册界面
 * 功能：返回登录界面
 */
void RegisterDialog::on_cancel_btn_clicked()
{
    this->_shutdown_timer->stop();
    emit sig_switch_to_login();
}

