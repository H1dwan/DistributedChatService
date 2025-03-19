#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <QByteArray>
#include <QRegularExpression>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include <functional>
#include <memory>
#include <iostream>
#include <mutex>
#include "QStyle"


extern QString gate_url_prefix;

extern std::function<void(QWidget*)> repolish;

extern std::function<QString(QString)> xoString;

const std::vector<QString>  strs ={"hello world !",
                                   "nice to meet u",
                                   "New year, new life",
                                   "You have to love yourself",
                                   "My love is written in the wind ever since the whole world is you"};

const std::vector<QString> heads = {
    ":/res/head_1.jpg",
    ":/res/head_2.jpg",
    ":/res/head_3.jpg",
    ":/res/head_4.jpg",
    ":/res/head_5.jpg"
};

const std::vector<QString> names = {
    "HanMeiMei",
    "Lily",
    "Ben",
    "Androw",
    "Max",
    "Summer",
    "Candy",
    "Hunter"
};

// TODO: TRY TO UNDERSTAND CODE
const int  tip_offset = 5;
const int MIN_APPLY_LABEL_ED_LEN = 40;  //申请好友标签输入框最低长度
const QString add_prefix = "添加标签 ";

/**
 * @brief The ReqID enum        请求类型（用于 HTTP回包的二级分发定位到具体的请求，从而调用对应的处理逻辑）
 */
enum ReqID {
    ID_GET_VERIFY_CODE = 1001,  // 获取验证码
    ID_REG_USER = 1002,         // 注册用户
    ID_RESET_PWD = 1003,        // 重置密码
    ID_USER_LOGIN = 1004,       // 用户登录
    ID_CHAT_LOGIN = 1005,       // 登录聊天服务器
    ID_CHAT_LOGIN_RSP = 1006,   // 登录聊天服务器回包
    ID_SEARCH_USER_REQ = 1007,  // 用户搜索请求
    ID_SEARCH_USER_RSP = 1008,  // 搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,   // 添加好友申请
    ID_ADD_FRIEND_RSP = 1010,   // 申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,    // 通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,          // 认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,          // 认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015,   // 通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ  = 1017,       // 文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,       // 本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, // 通知用户文本聊天信息
};

/**
 * @brief The Modules enum      发出请求的模块（用于 HTTP回包的一级分发定位到具体的模块）
 */
enum Modules {
    REGISTERMOD = 0,
    RESETMOD,
    LOGINMOD,
};

/**
 * @brief The ErrorCodes enum   HTTP通信的错误码
 */
enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1,       // json解析失败
    ERR_NETWORK = 2,    // 网络错误
};

/**
 * @brief The TipError enum     注册或重置密码时的错误提示类型
 */
enum TipError {
    TIP_SUCCESS = 0,
    TIP_USER_ERROR,     // 用户名为空
    TIP_EMAIL_ERROR,    // 邮箱地址不正确
    TIP_PWD_ERROR,      // 密码为空或者密码不合法
    TIP_PWD_CONFIRM,    // 密码与确认密码不匹配（包含了确认密码为空）
    TIP_VARIFY_ERROR,   // 验证码为空
};

enum ClickLbState{
    Normal = 0,     // 闭眼
    Selected = 1    // 睁眼
};

/**
 * @brief The ServerInfo class  服务器返回给客户端的参数信息（用于 tcp登录）
 */
struct ServerInfo {
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

enum class ChatRole
{
    Self,
    Other,
};

struct MsgInfo
{
    QString msgFlag;    // "text,image,file"
    QString content;    // 表示文件和图像的 url ,文本信息
    QPixmap pixmap;     // 文件和图片的缩略图
};

#endif // GLOBAL_H
