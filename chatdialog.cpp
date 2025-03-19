#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include <QRandomGenerator>
#include <QMovie>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include "chatuserwid.h"
#include "tcpmgr.h"
#include "usermgr.h"


ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), _b_loading(false),
    _mode(ChatUIMode::ChatMode), _state(ChatUIMode::ChatMode), _cur_chat_uid(0), _last_widget(ui->background_page)
{
    ui->setupUi(this);
    // 初始化上次浏览页面
    _last_page[ChatUIMode::ChatMode] = ui->background_page;
    _last_page[ChatUIMode::ContactMode] = ui->background_page;
    // 设置初始页的背景图片
    QLabel* bg_label = new QLabel();
    bg_label->setPixmap(QPixmap(":res/wechat_background.png"));
    bg_label->adjustSize();
    QVBoxLayout *layout = new QVBoxLayout(ui->background_page);
    layout->addStretch(); // 添加伸缩项
    layout->addWidget(bg_label, 0, Qt::AlignCenter); // 添加 QLabel 并居中
    layout->addStretch(); // 添加伸缩项
    ui->background_page->setLayout(layout);

    ui->add_btn->SetState("normal", "hover", "press");
    // 设置搜索栏最大长度
    ui->search_edit->SetMaxLength(30);
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));
    _ShowSearchPage(false);
    // 创建 search动作并设置图标（放大镜）
    QAction* search_action = new QAction(ui->search_edit);
    search_action->setIcon(QIcon(":/res/search.png"));
    // 将动作添加到搜索栏头部
    ui->search_edit->addAction(search_action, QLineEdit::LeadingPosition);
    // 创建一个清除动作并设置图标，默认空白（×）
    QAction* clear_action = new QAction(ui->search_edit);
    clear_action->setIcon(QIcon(":res/close_transparent.png"));
    ui->search_edit->addAction(clear_action, QLineEdit::TrailingPosition);

    // 当输入文本时，更改为实际的清除图标
    connect(ui->search_edit, &QLineEdit::textChanged, [clear_action](const QString text) {
        if (!text.isEmpty())
        {
            clear_action->setIcon(QIcon(":/res/close_search.png"));
        }
        else
        {
            // 文本为空时，切换回透明图标
            clear_action->setIcon(QIcon(":res/close_transparent.png"));
        }
    });

    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(clear_action, &QAction::triggered, [this, clear_action]() {
        ui->search_edit->clear();
        clear_action->setIcon(QIcon(":res/close_transparent.png"));
        ui->search_edit->clearFocus();
        //清除按钮被按下则不显示搜索框
        _ShowSearchPage(false);
    });

    // 连接加载更多最近聊天条目信号
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_users,
            this, &ChatDialog::slot_loading_chat_users);
    // 连接加载更多联系人信号
    connect(ui->conn_user_list, &ContactUserList::sig_loading_contact_users, this,
            &ChatDialog::slot_loading_contact_users);


    // TODO: 测试用户
    _addChatUserList();
    // 加载自己的头像
    QPixmap pixmap(UserMgr::GetInstance()->GetUserInfo()->icon);
    QPixmap scaledPixmap = pixmap.scaled( ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->side_head_lb->setPixmap(scaledPixmap);
    ui->side_head_lb->setScaledContents(true);

    // 初始化侧边栏图标的状态
    ui->widget->layout()->setAlignment(Qt::AlignCenter);
    ui->side_chat_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_contact_lb->SetState("normal","hover","pressed","selected_normal","selected_hover","selected_pressed");
    ui->side_chat_lb->SetSelected(true);    // 初始化 side_chat_lb 为选中状态
    // 实现侧边栏图标高亮切换
    _AddLBGroup(ui->side_chat_lb);
    _AddLBGroup(ui->side_contact_lb);
    // 连接 侧边栏中 chat_lb 和 contact_lb 的点击信号
    connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    // 连接搜索框输入变化
    connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);

    //检测鼠标点击位置判断是否要清空搜索框
    this->installEventFilter(this); // 安装事件过滤器

    // 为 searchlist 设置 search edit
    ui->search_list->SetSearchEdit(ui->search_edit);

    // 连接申请添加好友通知信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_apply_friend);

    // 连接认证添加好友信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);

    // 连接自己认证回复信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ChatDialog::slot_auth_rsp);

    // 连接 SearchList 跳转聊天界面信号
    connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    // 连接点击联系人item发出的信号和用户信息展示槽函数
    connect(ui->conn_user_list, &ContactUserList::sig_switch_friend_info_page, this, &ChatDialog::slot_friend_info_page);

    // 连接联系人页面点击好友申请条目的信号
    connect(ui->conn_user_list, &ContactUserList::sig_switch_apply_friend_page, this,
            &ChatDialog::slot_switch_apply_friend_page);

    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_page, this,
            &ChatDialog::slot_jump_chat_page_from_friend_info_page);

    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this, &ChatDialog::slot_append_send_chat_msg);

    // 连接对端消息通知
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);
    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this, &ChatDialog::slot_append_send_chat_msg);

    // 连接聊天列表点击信号
    connect(ui->chat_user_list, &QListWidget::itemClicked, this, &ChatDialog::slot_item_clicked);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

/**
 * @brief ChatDialog::_addChatUserList  随机添加 13 个好友（测试用）
 */
void ChatDialog::_addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(heads[head_i], names[name_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);
        ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
}

/**
 * @brief ChatDialog::eventFilter
 * @param watched
 * @param event
 * @return
 */
bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        qDebug() << "mouse press!";
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        _HandleGlobalMousePress(mouseEvent);
        return true;
    }
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::_ShowSearchPage(bool b_show)
{
    if (b_show)
    {
        ui->chat_user_list->hide();
        ui->conn_user_list->hide();
        ui->search_list->show();
        _mode = ChatUIMode::SearchMode;
    }
    else if (_state == ChatUIMode::ChatMode)
    {
        ui->chat_user_list->show();
        ui->conn_user_list->hide();
        ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
    }
    else if (_state == ChatUIMode::ContactMode)
    {
        ui->chat_user_list->hide();
        ui->conn_user_list->show();
        ui->search_list->hide();
        _mode = ChatUIMode::ContactMode;
    }
}

// 将 StateWidget 添加到 list中统一管理
void ChatDialog::_AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}

// 清除 _lb_list中除了 lb的选中状态
void ChatDialog::_ClearLabelState(StateWidget *lb)
{
    for (auto& elem : _lb_list)
    {
        if (elem == lb)
        {
            continue;
        }
        elem->ClearState();
    }
}

/**
 * @brief ChatDialog::_HandleGlobalMousePress   根据鼠标点击的位置，判断是否清除搜索框中的内容
 * @param event
 */
void ChatDialog::_HandleGlobalMousePress(QMouseEvent *event)
{
    if (_mode != ChatUIMode::SearchMode)
    {
        return;
    }
    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPointF posInSearchList = ui->search_list->mapFromGlobal(event->globalPosition());
    // 判断点击位置是否超出 search_list 的右边界
    if (posInSearchList.x() > ui->search_list->rect().x() + ui->search_list->rect().width())
    {
        ui->search_edit->clear();
        _ShowSearchPage(false);
    }
}

/**
 * @brief ChatDialog::SetSelectChatItem 根据用户 ID（uid）选择并设置当前的聊天项（设置为选中）
 * @param uid
 */
void ChatDialog::SetSelectChatItem(int uid)
{
    // 如果列表为空，直接返回
    if (ui->chat_user_list->count() <= 0)
    {
        return;
    }

    // 如果 uid 为 0，选择列表中的第一项
    if (uid == 0)
    {
        ui->chat_user_list->setCurrentRow(0);
        auto* first_item = ui->chat_user_list->item(0);
        if (!first_item)
        {
            qDebug() << "First item is null";
            return;
        }
        auto* widget = ui->chat_user_list->itemWidget(first_item);
        if (!widget)
        {
            qDebug() << "Item widget is null";
            return;
        }
        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item)
        {
            qDebug() << "Failed to cast widget to ChatUserWid";
            return;
        }
        _cur_chat_uid = con_item->GetUserInfo()->uid;
        return;
    }

    // 在 _chat_items_added 中查找 uid
    auto find_it = _chat_items_added.find(uid);
    if (find_it == _chat_items_added.end())
    {
        qDebug() << "uid " <<uid<< " not found, set curent row 0";
        ui->chat_user_list->setCurrentRow(0);
        return;
    }
    // 如果找到，设置当前项为找到的项，并更新当前聊天用户的 UID
    ui->chat_user_list->setCurrentItem(find_it.value());
    _cur_chat_uid = uid;
}

/**
 * @brief ChatDialog::SetSelectChatPage 根据用户ID (uid) 设置聊天页面的用户信息
 * @param uid
 */
void ChatDialog::SetSelectChatPage(int uid)
{
    // 如果列表为空，直接返回
    if (ui->chat_user_list->count() <= 0)
    {
        qDebug() << "chat_user_list is empty";
        return;
    }

    // 如果 uid 为 0，则将聊天页面设置为列表中的第一项
    if (uid == 0)
    {
        auto item = ui->chat_user_list->item(0);
        auto* widget = ui->chat_user_list->itemWidget(item);
        if (!widget)
        {
            return;
        }
        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item)
        {
            return;
        }
        //设置信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }

    // 如果 uid 不为 0，查找 uid 对应的项
    auto find_it = _chat_items_added.find(uid);
    if (find_it == _chat_items_added.end())
    {
        qDebug() << "uid " << uid << " not found in _chat_items_added";
        return;
    }
    // 获取项的自定义控件
    auto* widget = ui->chat_user_list->itemWidget(find_it.value());
    if (!widget)
    {
        qDebug() << "itemWidget is null for uid " << uid;
        return;
    }
    // 将 widget 转换为 ListItemBase 类型
    auto* custom_item = qobject_cast<ListItemBase*>(widget);
    if(!custom_item)
    {
        qDebug() << "Failed to cast widget to ListItemBase for uid " << uid;
        return;
    }
    // 检查项的类型
    auto item_type = custom_item->GetItemType();
    if (item_type == ListItemType::CHAT_USER_ITEM)
    {
        auto con_item = qobject_cast<ChatUserWid*>(custom_item);
        if (!con_item)
        {
            qDebug() << "Failed to cast custom_item to ChatUserWid for uid " << uid;
            return;
        }
        // 获取用户信息并设置到 chat_page 聊天页面
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }
}

/**
 * @brief ChatDialog::UpdateChatMsg
 * @param msgdata
 */
void ChatDialog::UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata)
{
    for(auto & msg : msgdata)
    {
        if(msg->from_uid != _cur_chat_uid){
            break;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}

/**
 * @brief ChatDialog::_LoadMoreChatUser 加载更多最近聊天条目
 */
void ChatDialog::_LoadMoreChatUser()
{
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (!friend_list.empty())
    {
        for (const auto& friend_ele : friend_list)
        {
            auto find_iter = _chat_items_added.find(friend_ele->uid);
            if(find_iter != _chat_items_added.end()){
                continue;
            }
            auto* chat_user_wid = new ChatUserWid();
            chat_user_wid->SetInfo(friend_ele);
            auto* item = new QListWidgetItem();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->conn_user_list->addItem(item);
            ui->conn_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->uid, item);
        }
        //更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }
}

/**
 * @brief ChatDialog::_LoadMoreConUser  加载更多联系人
 */
void ChatDialog::_LoadMoreConUser()
{
    auto friend_list = UserMgr::GetInstance()->GetConListPerPage();
    if (!friend_list.empty())
    {
        for (const auto& friend_ele : friend_list)
        {
            auto* chat_user_wid = new ChatUserWid();
            chat_user_wid->SetInfo(friend_ele);
            auto* item = new QListWidgetItem();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->conn_user_list->addItem(item);
            ui->conn_user_list->setItemWidget(item, chat_user_wid);
        }
        //更新已加载条目
        UserMgr::GetInstance()->UpdateContactLoadedCount();
    }
}

void ChatDialog::slot_loading_chat_users()
{
    if (_b_loading)
    {
        return;
    }

    _b_loading = true;
    // 加载等待动画资源
    QMovie* movie = new QMovie(":/res/loading.gif");
    movie->setScaledSize(QSize(50, 50));    // 设置动画的大小
    // 创建一个 label，并将动画加载到 label上
    QLabel* loading_label = new QLabel(this);
    loading_label->setMovie(movie);
    loading_label->setFixedSize(250, 70);
    loading_label->setAlignment(Qt::AlignCenter);
    // 创建一个 list widget，并将 label绑定上
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(250, 70));
    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, loading_label);
    movie->start();
    // 500ms后执行一个一次性事件
    QTimer::singleShot(500, this, [this, item]() {
        qDebug() << "add new contact info to list...";
        _LoadMoreConUser();
        ui->chat_user_list->takeItem(ui->chat_user_list->row(item));
        ui->chat_user_list->update();
        _b_loading = false;
    });
}

void ChatDialog::slot_loading_contact_users()
{
    if (_b_loading)
    {
        return;
    }

    _b_loading = true;
    // 加载等待动画资源
    QMovie* movie = new QMovie(":/res/loading.gif");
    movie->setScaledSize(QSize(50, 50));    // 设置动画的大小
    // 创建一个 label，并将动画加载到 label上
    QLabel* loading_label = new QLabel(this);
    loading_label->setMovie(movie);
    loading_label->setFixedSize(250, 70);
    loading_label->setAlignment(Qt::AlignCenter);
    // 创建一个 list widget，并将 label绑定上
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(250, 70));
    ui->conn_user_list->addItem(item);
    ui->conn_user_list->setItemWidget(item, loading_label);
    movie->start();
    // 500ms后执行一个一次性事件
    QTimer::singleShot(500, this, [this, item]() {
        qDebug() << "add new contact info to list...";
        _LoadMoreConUser();
        ui->conn_user_list->takeItem(ui->chat_user_list->row(item));
        ui->conn_user_list->update();
        _b_loading = false;
    });
}

/**
 * @brief ChatDialog::slot_side_chat 处理侧边栏 chat_lb 的点击事件槽函数
 */
void ChatDialog::slot_side_chat()
{
    // qDebug()<< "receive side chat clicked";
    _ClearLabelState(ui->side_chat_lb);
    ui->stackedWidget->setCurrentWidget(_last_page[ChatUIMode::ChatMode]);
    _state = ChatUIMode::ChatMode;
    _ShowSearchPage(false);
}

/**
 * @brief ChatDialog::slot_side_contact 处理侧边栏 contact_lb 的点击事件槽函数
 */
void ChatDialog::slot_side_contact()
{
    // qDebug()<< "receive side chat clicked";
    _ClearLabelState(ui->side_contact_lb);
    ui->stackedWidget->setCurrentWidget(_last_page[ChatUIMode::ContactMode]);
    _state = ChatUIMode::ContactMode;
    _ShowSearchPage(false);
}

/**
 * @brief ChatDialog::slot_text_changed     处理搜索框中有内容时，跳转搜索界面
 * @param str   搜索框中的字符串
 */
void ChatDialog::slot_text_changed(const QString &str)
{
    qDebug()<< "receive slot text changed str is " << str;
    if (str.isEmpty())
    {
        return;
    }
    _ShowSearchPage(true);
}

/**
 * @brief ChatDialog::slot_item_clicked 点击最近聊天用户条目后，跳转到聊天界面
 * @param item  被点击的条目
 */
void ChatDialog::slot_item_clicked(QListWidgetItem *item)
{
    auto* widget = ui->chat_user_list->itemWidget(item);
    if (!widget)
    {
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    // 对自定义 widget 进行操作， 将 item 转化为基类 ListItemBase
    auto* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    // 获取 item 的 type
    auto itemType = customItem->GetItemType();
    if (itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM)
    {
        qDebug()<< "slot invalid item clicked ";
        return;
    }
    if (itemType == ListItemType::CHAT_USER_ITEM)
    {
        qDebug()<< "chat user item clicked ";
        auto chat_wid = qobject_cast<ChatUserWid*>(customItem);
        auto user_info = chat_wid->GetUserInfo();
        // 跳转到聊天界面
        ui->chat_page->SetUserInfo(user_info);
        _cur_chat_uid = user_info->uid;
        ui->stackedWidget->setCurrentWidget(ui->chat_page);
        _last_page[ChatUIMode::ChatMode] = ui->chat_page;
        _last_widget = ui->chat_page;
        return;
    }
}

/**
 * @brief ChatDialog::slot_apply_friend     处理好友添加通知（receiver）
 * @param apply
 */
void ChatDialog::slot_apply_friend(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "receive apply friend slot, applyuid is " << apply->from_uid << " name is "
             << apply->name << " desc is " << apply->description;

    // 已收到过该好友申请通知，则直接返回
    if (UserMgr::GetInstance()->AlreadyApply(apply->from_uid))
    {
        return;
    }
    // 未收到过该好友申请通知，则将该申请信息添加到列表中，并在界面中显示未处理的好友申请
    UserMgr::GetInstance()->AddApplyList(std::make_shared<ApplyInfo>(apply));
    ui->side_contact_lb->ShowRedPoint();
    ui->conn_user_list->ShowRedPoint();
    ui->friend_apply_page->AddNewApply(apply);
}

/**
 * @brief ChatDialog::slot_add_auth_friend
 */
void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info)
{
    qDebug() << "receive slot_add_auth__friend uid is " << auth_info->uid
             << " name is " << auth_info->name;
    // 将新的好友信息添加到 UserMgr 中管理
    UserMgr::GetInstance()->AddFriend(auth_info);
    // 将新添加的好友聊天条目添加到聊天列表中
    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(std::make_shared<UserInfo>(auth_info));
    auto* item = new QListWidgetItem();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->uid, item);
}

/**
 * @brief ChatDialog::slot_auth_rsp
 */
void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->uid
             << " name is " << auth_rsp->name;
    // 将新的好友信息添加到 UserMgr 中管理
    UserMgr::GetInstance()->AddFriend(auth_rsp);
    // 将新添加的好友聊天条目添加到聊天列表中
    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(std::make_shared<UserInfo>(auth_rsp));
    auto* item = new QListWidgetItem();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->uid, item);
}

/**
 * @brief ChatDialog::slot_jump_chat_item
 * @param si
 */
void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{
    qDebug() << "slot jump chat item";
    auto find_it = _chat_items_added.find(si->uid);
    // 搜索到的用户在历史聊天列表中
    if (find_it != _chat_items_added.end())
    {
        qDebug() << "jump to chat item , uid is " << si->uid;
        // 将滚动条滚动到合适位置，使得该 item 进入可视区域
        ui->chat_user_list->scrollToItem(find_it.value());
        // 侧边栏选中为聊天
        ui->side_chat_lb->SetSelected(true);
        // 将聊天用户设置为选中
        SetSelectChatItem(si->uid);
        // 更新聊天界面信息
        SetSelectChatPage(si->uid);
        slot_side_chat();
        return;
    }
    // 不在，则新建一个聊天条目并插入到聊天列表中
    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(std::make_shared<UserInfo>(si));
    auto* item = new QListWidgetItem();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    // 用于管理历史聊天列表
    _chat_items_added.insert(si->uid, item);
    // 侧边栏选中为聊天
    ui->side_chat_lb->SetSelected(true);
    // 将聊天用户设置为选中
    SetSelectChatItem(si->uid);
    // 更新聊天页面信息
    SetSelectChatPage(si->uid);
    slot_side_chat();
}

/**
 * @brief ChatDialog::slot_friend_info_page
 * @param user_info
 */
void ChatDialog::slot_friend_info_page(std::shared_ptr<UserInfo> user_info)
{
    qDebug()<<"receive switch friend info page sig";
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    _last_page[ChatUIMode::ContactMode] = ui->friend_info_page;
    auto info = std::make_shared<FriendInfo>(user_info->uid, user_info->gender, user_info->name,
                                             user_info->nickname, "", user_info->icon, "");
    ui->friend_info_page->SetInfo(info);
}

/**
 * @brief ChatDialog::slot_switch_apply_friend_page
 */
void ChatDialog::slot_switch_apply_friend_page()
{
    qDebug()<<"receive switch apply friend page sig";
    _last_widget = ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _last_page[ChatUIMode::ContactMode] = ui->friend_apply_page;
}

void ChatDialog::slot_jump_chat_page_from_friend_info_page(std::shared_ptr<FriendInfo> friend_info)
{
    qDebug() << "slot jump chat page";
    auto find_it = _chat_items_added.find(friend_info->uid);
    if (find_it != _chat_items_added.end())
    {
        qDebug() << "jump to chat item , uid is " << friend_info->uid;
        ui->chat_user_list->scrollToItem(find_it.value());
        ui->side_chat_lb->SetSelected(true);
        SetSelectChatItem(friend_info->uid);
        //更新聊天界面信息
        SetSelectChatPage(friend_info->uid);
        slot_side_chat();
        return;
    }
    // 不在，则新建一个聊天条目并插入到聊天列表中
    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(friend_info);
    auto* item = new QListWidgetItem();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    // 用于管理历史聊天列表
    _chat_items_added.insert(friend_info->uid, item);
    // 侧边栏选中为聊天
    ui->side_chat_lb->SetSelected(true);
    // 将聊天用户设置为选中
    SetSelectChatItem(friend_info->uid);
    // 更新聊天页面信息
    SetSelectChatPage(friend_info->uid);
    slot_side_chat();
}

/**
 * @brief ChatDialog::slot_text_chat_msg
 * @param msg
 */
void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg)
{
    auto find_iter = _chat_items_added.find(msg->from_uid);
    // 消息的发送者位于好友聊天条目列表中
    if (find_iter != _chat_items_added.end())
    {
        qDebug() << "receive message from uid " << msg->from_uid;
        auto* widget = ui->chat_user_list->itemWidget(find_iter.value());
        auto chat_wid = qobject_cast<ChatUserWid*>(widget);
        if (!chat_wid)
        {
            return;
        }
        // 更新好友条目中的最近聊天记录
        chat_wid->UpdateLastMsg(msg->chat_msgs);
        // TODO: 条目置顶
        // 更新聊天页面的聊天记录
        UpdateChatMsg(msg->chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->from_uid, msg->chat_msgs);
        return;
    }

    // 消息的发送者不在好友聊天条目列表中，则创建新的插入列表里
    auto* chat_user_wid = new ChatUserWid();
    auto p_friend_info = UserMgr::GetInstance()->GetFriendByUID(msg->from_uid);
    chat_user_wid->SetInfo(p_friend_info);
    chat_user_wid->UpdateLastMsg(msg->chat_msgs);
    UserMgr::GetInstance()->AppendFriendChatMsg(msg->from_uid, msg->chat_msgs);
    auto* item = new QListWidgetItem();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->from_uid, item);
}

void ChatDialog::slot_append_send_chat_msg(std::shared_ptr<TextChatData> msg)
{
    if (_cur_chat_uid == 0) {
        return;
    }

    auto find_iter = _chat_items_added.find(_cur_chat_uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    //转为widget
    QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    // 判断转化为自定义的widget
    // 对自定义widget进行操作， 将 item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == ListItemType::CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if (!con_item) {
            return;
        }

        //设置信息
        auto user_info = con_item->GetUserInfo();
        user_info->chat_msgs.push_back(msg);
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.push_back(msg);
        UserMgr::GetInstance()->AppendFriendChatMsg(_cur_chat_uid,msg_vec);
        return;
    }
}
