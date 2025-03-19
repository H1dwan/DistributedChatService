#ifndef USERDATA_H
#define USERDATA_H

#include <QString>
#include <QJsonArray>
#include <QJsonObject>



// 搜索好友返回的好友信息
struct SearchInfo
{
    int uid;
    int gender;
    QString name;           // 用户名
    QString nickname;       // 昵称
    QString description;    // 描述
    QString icon;           // 头像的 url

    SearchInfo(int uid, int gender, QString name, QString nickname, QString desc, QString icon)
        : uid(uid), gender(gender), name(name), nickname(nickname), description(desc), icon(icon) {}
};


struct AddFriendApply
{
    int from_uid;   // 对方的 uid
    int gender;
    QString name;
    QString nickname;
    QString description;
    QString icon;   // 头像的 url

    AddFriendApply(int from_uid, int gender, QString name, QString nickname, QString desc, QString icon)
        :from_uid(from_uid), gender(gender), name(name), nickname(nickname), description(desc), icon(icon) {}
};

struct ApplyInfo
{
    int uid;
    int gender;
    QString name;
    QString nickname;
    QString description;
    QString icon;   // 头像的 url
    int status;

    ApplyInfo(int uid, int gender, QString name, QString nickname, QString desc, QString icon, int status)
        : uid(uid), gender(gender), name(name), nickname(nickname), description(desc), icon(icon), status(status) {}

    ApplyInfo(std::shared_ptr<AddFriendApply> addinfo)
        : uid(addinfo->from_uid), name(addinfo->name), nickname(addinfo->nickname),
        description(addinfo->description), icon(addinfo->icon), status(0) {}

    void SetIcon(QString head)
    {
        icon = head;
    }
};

struct AuthInfo
{
    int uid;
    int gender;
    QString name;
    QString nickname;
    QString icon;

    AuthInfo(int uid, int gender, QString name, QString nickname, QString icon)
        : uid(uid), gender(gender), name(name), nickname(nickname), icon(icon) {}
};

struct AuthRsp
{
    int uid;
    int gender;
    QString name;
    QString nickname;
    QString icon;

    AuthRsp(int peer_uid, int peer_gender, QString peer_name, QString peer_nick, QString peer_icon)
        : uid(peer_uid), gender(peer_gender), name(peer_name), nickname(peer_nick), icon(peer_icon) {}
};

struct TextChatData
{
    int from_uid;
    int to_uid;
    QString msg_id;
    QString msg_content;

    TextChatData(int from_uid, int to_uid, QString msg_id, QString msg_content)
        : from_uid(from_uid), to_uid(to_uid), msg_id(msg_id), msg_content(msg_content) {}
};

struct TextChatMsg
{
    int from_uid;
    int to_uid;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;

    TextChatMsg(int from_uid, int to_uid, QJsonArray arrays)
        : from_uid(from_uid), to_uid(to_uid)
    {
        for (auto msg_data : arrays)
        {
            auto msg_obj = msg_data.toObject();
            auto content = msg_obj["content"].toString();
            auto msgid = msg_obj["msgid"].toString();
            auto msg_ptr = std::make_shared<TextChatData>(from_uid, to_uid, msgid, content);
            chat_msgs.push_back(msg_ptr);
        }
    }
};

struct FriendInfo
{
    int uid;
    int gender;
    QString name;
    QString nickname;
    QString description;
    QString icon;           // 头像
    QString back;           // 备注
    QString last_msg;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;   // 历史消息

    FriendInfo(int uid, int gender, QString name, QString nickname, QString desc,
               QString icon, QString back, QString last_msg="")
        : uid(uid), gender(gender), name(name), nickname(nickname), description(desc),
        icon(icon), back(back), last_msg(last_msg) {}

    FriendInfo(std::shared_ptr<AuthInfo> auth_info)
        : uid(auth_info->uid), gender(auth_info->gender), name(auth_info->name),
        nickname(auth_info->nickname), description(""),
        icon(auth_info->icon), back(""), last_msg("") {}

    FriendInfo(std::shared_ptr<AuthRsp> auth_rsp)
        : uid(auth_rsp->uid), gender(auth_rsp->gender), name(auth_rsp->name),
        nickname(auth_rsp->nickname), description(""),
        icon(auth_rsp->icon), back(""), last_msg("") {}

    void AppendChatMsgs(const std::vector<std::shared_ptr<TextChatData>> text_vec)
    {
        for(const auto & text: text_vec){
            chat_msgs.push_back(text);
        }
    }
};

struct UserInfo
{
    int uid;
    int gender;
    QString name;
    QString nickname;
    QString icon;
    QString last_msg;
    std::vector<std::shared_ptr<TextChatData>> chat_msgs;

    UserInfo(int uid, int gender, QString name, QString nickname,
               QString icon, QString last_msg="")
        : uid(uid), gender(gender), name(name), nickname(nickname), icon(icon), last_msg(last_msg) {}

    UserInfo(std::shared_ptr<AuthInfo> auth)
        : uid(auth->uid), gender(auth->gender), name(auth->name), nickname(auth->nickname),
        icon(auth->icon), last_msg("") {}

    UserInfo(std::shared_ptr<AuthRsp> auth)
        : uid(auth->uid), gender(auth->gender), name(auth->name), nickname(auth->nickname),
        icon(auth->icon), last_msg("") {}

    UserInfo(int uid, QString name, QString icon, QString last_msg="")
        : uid(uid), gender(0), name(name), nickname(""), icon(icon), last_msg(last_msg) {}

    UserInfo(std::shared_ptr<SearchInfo> info)
        : uid(info->uid), gender(info->gender), name(info->name), nickname(info->nickname),
        icon(info->icon), last_msg("") {}

    UserInfo(std::shared_ptr<FriendInfo> info)
        : uid(info->uid), gender(info->gender), name(info->name), nickname(info->nickname),
        icon(info->icon), last_msg("")
    {
        chat_msgs = info->chat_msgs;
    }
};

#endif // USERDATA_H
