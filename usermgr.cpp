#include "usermgr.h"

constexpr int CHAT_COUNT_PER_PAGE = 12;

int UserMgr::GetUID()
{
    return _user_info->uid;
}

QString UserMgr::GetName()
{
    return _user_info->name;
}

QString UserMgr::GetIcon()
{
    return _user_info->icon;
}

std::shared_ptr<UserInfo> UserMgr::GetUserInfo()
{
    return _user_info;
}


void UserMgr::SetToken(const QString &token)
{
    _token = token;
}

void UserMgr::SetUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
}

bool UserMgr::CheckFriendByUID(int uid)
{
    return _friend_map.contains(uid);
}


QMap<int, std::shared_ptr<ApplyInfo>> UserMgr::GetApplyList()
{
    return _apply_list;
}

// 判断是否已经收到过同一 UID 发来的好友申请
bool UserMgr::AlreadyApply(int uid)
{
    return _apply_list.contains(uid);
}

void UserMgr::AddApplyList(std::shared_ptr<ApplyInfo> info)
{
    _apply_list.insert(info->uid, info);
}

/**
 * @brief UserMgr::AppendApplyList  将好友申请信息保存到 _apply_list 中（方便后续的可视化）
 * @param array 从 ChatServer 收到的登录回包中携带的申请列表数据，以 QJsonArray 形式保存，其中的每个 QJsonValue 都为一个 QJsonObject
 */
void UserMgr::AppendApplyList(QJsonArray array)
{
    // 遍历 QJsonArray 并输出每个元素
    for (const auto& value : array)
    {
        QJsonObject obj = value.toObject();
        auto uid = obj["uid"].toInt();
        auto gender = obj["gender"].toInt();
        auto name = obj["name"].toString();
        auto nickname = obj["nickname"].toString();
        auto desc = obj["desc"].toString();
        auto icon = obj["icon"].toString();
        auto status = obj["status"].toInt();
        _apply_list.insert(uid, std::make_shared<ApplyInfo>(uid, gender, name, nickname, desc, icon, status));
    }
}

std::shared_ptr<FriendInfo> UserMgr::GetFriendByUID(int uid)
{
    auto find_it = _friend_map.find(uid);
    if (find_it == _friend_map.end())
    {
        return nullptr;
    }
    return *find_it;
}

void UserMgr::AddFriend(std::shared_ptr<AuthRsp> auth_rsp)
{
    _friend_map.insert(auth_rsp->uid, std::make_shared<FriendInfo>(auth_rsp));
}

void UserMgr::AddFriend(std::shared_ptr<AuthInfo> auth_info)
{
    _friend_map.insert(auth_info->uid, std::make_shared<FriendInfo>(auth_info));
}

/**
 * @brief UserMgr::AppendFriendList 将好友信息保存到 _friend_list 中（方便后续的可视化）
 * @param array
 */
void UserMgr::AppendFriendList(QJsonArray array)
{
    // 遍历 QJsonArray 并输出每个元素
    for (const auto& value : array)
    {
        QJsonObject obj = value.toObject();
        auto uid = obj["uid"].toInt();
        auto gender = obj["gender"].toInt();
        auto name = obj["name"].toString();
        auto nickname = obj["nickname"].toString();
        auto desc = obj["desc"].toString();
        auto icon = obj["icon"].toString();
        auto back = obj["back"].toString();
        auto info = std::make_shared<FriendInfo>(uid, gender, name, nickname, desc, icon, back);
        _friend_list.push_back(info);
        _friend_map.insert(uid, info);
    }
}

/**
 * @brief UserMgr::GetChatListPerPage   获取一页聊天条目
 * @return
 */
std::vector<std::shared_ptr<FriendInfo>> UserMgr::GetChatListPerPage()
{
    size_t begin = _chat_loaded;
    size_t end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return {};
    }

    if (end > _friend_list.size())
    {
        return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin() + begin, _friend_list.end());
    }

    return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin() + begin, _friend_list.begin()+ end);
}

bool UserMgr::IsLoadChatFin()
{
    return _chat_loaded >= _friend_list.size();
}

void UserMgr::UpdateChatLoadedCount()
{
    size_t begin = _chat_loaded;
    size_t end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size())
    {
        return;
    }

    if (end > _friend_list.size())
    {
        _chat_loaded = _friend_list.size();
        return ;
    }

    _chat_loaded = end;
}

/**
 * @brief UserMgr::GetConListPerPage    获取一页联系人条目
 * @return
 */
std::vector<std::shared_ptr<FriendInfo> > UserMgr::GetConListPerPage()
{
    size_t begin = _contact_loaded;
    size_t end = begin + CHAT_COUNT_PER_PAGE;
    if (begin >= _friend_list.size())
    {
        return {};
    }

    if (end > _friend_list.size())
    {
        return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin() + begin, _friend_list.end());
    }

    return std::vector<std::shared_ptr<FriendInfo>>(_friend_list.begin() + begin, _friend_list.begin()+ end);
}

void UserMgr::UpdateContactLoadedCount()
{
    size_t begin = _contact_loaded;
    size_t end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= _friend_list.size())
    {
        return;
    }

    if (end > _friend_list.size())
    {
        _chat_loaded = _friend_list.size();
        return ;
    }

    _contact_loaded = end;
}

bool UserMgr::IsLoadConFin()
{
    return _contact_loaded >= _friend_list.size();
}

void UserMgr::AppendFriendChatMsg(int friend_id, std::vector<std::shared_ptr<TextChatData>> msgs)
{
    auto find_iter = _friend_map.find(friend_id);
    if(find_iter == _friend_map.end()){
        qDebug()<<"append friend uid  " << friend_id << " not found";
        return;
    }

    find_iter.value()->AppendChatMsgs(msgs);
}

UserMgr::UserMgr() : _user_info(nullptr), _chat_loaded(0), _contact_loaded(0)
{
}
