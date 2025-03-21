#include "contactuserlist.h"

#include <QRandomGenerator>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include "contactuseritem.h"
#include "grouptipitem.h"
#include "tcpmgr.h"
#include "usermgr.h"

ContactUserList::ContactUserList(QWidget *parent)
    :_load_pending(false)
    , _add_friend_item(nullptr)
{
    Q_UNUSED(parent);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);

    // TODO: FOR TEST, LOADING DATA FROM DATABASE
    AddContactUserList();

    // 连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
    // 连接对端同意认证后通知的信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend,
            this, &ContactUserList::slot_add_auth_firend);
    // 连接自己点击同意认证后界面刷新
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp,
            this, &ContactUserList::slot_auth_rsp);
}

void ContactUserList::ShowRedPoint(bool show)
{
    this->_add_friend_item->ShowRedPoint(show);
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if (watched == this->viewport())
    {
        if (event->type() == QEvent::Enter)         // 鼠标悬浮，显示滚动条
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        else if (event->type() == QEvent::Leave)    // 鼠标离开，隐藏滚动条
        {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

    // 检查事件是否是鼠标滚轮事件
    if (watched == this->viewport() && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numDegrees = wheelEvent->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // 计算滚动步数

        // 设置滚动幅度
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);

        // 检查是否滚动到底部
        QScrollBar *scrollBar = this->verticalScrollBar();
        int maxScrollValue = scrollBar->maximum();
        int currentValue = scrollBar->value();
        if (maxScrollValue - currentValue <= 0)
        {
            if (UserMgr::GetInstance()->IsLoadConFin())
            {
                return true;
            }
            qDebug()<<"load more contact user";
            emit sig_loading_contact_users();
        }
        return true;    // 停止事件传递
    }

    return QListWidget::eventFilter(watched, event);
}

/**
 * @brief ContactUserList::AddContactUserList   初始化联系人列表
 */
void ContactUserList::AddContactUserList()
{
    auto* groupTip = new GroupTipItem();
    auto* item = new QListWidgetItem();
    item->setSizeHint(groupTip->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    this->addItem(item);
    this->setItemWidget(item, groupTip);

    _add_friend_item = new ContactUserItem();
    _add_friend_item->setObjectName("new_friend_item");
    _add_friend_item->SetInfo(0, "新的朋友", ":/res/add_friend.png");
    _add_friend_item->SetItemType(ListItemType::APPLY_FRIEND_ITEM);
    auto* add_item = new QListWidgetItem();
    add_item->setSizeHint(_add_friend_item->sizeHint());
    this->addItem(add_item);
    this->setItemWidget(add_item, _add_friend_item);
    // this->setCurrentItem(add_item); // 默认设置新的朋友申请条目被选中

    auto* groupContact = new GroupTipItem();
    groupContact->SetGroupTip("联系人");
    _groupitem = new QListWidgetItem();
    _groupitem->setSizeHint(groupContact->sizeHint());
    _groupitem->setFlags(_groupitem->flags() & ~Qt::ItemIsSelectable);
    this->addItem(_groupitem);
    this->setItemWidget(_groupitem, groupContact);

    // 加载数据库中的联系人列表（从后端获取的）
    auto contact_list = UserMgr::GetInstance()->GetConListPerPage();
    for (const auto& contact : contact_list)
    {
        auto* contact_item = new ContactUserItem();
        contact_item->SetInfo(contact->uid, contact->name, contact->icon);
        auto* item = new QListWidgetItem();
        item->setSizeHint(contact_item->sizeHint());
        this->addItem(item);    // 追加到列表尾
        this->setItemWidget(item, contact_item);
    }
    UserMgr::GetInstance()->UpdateContactLoadedCount();

    for (int i = 0; i < 13; ++i)
    {
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *con_user_wid = new ContactUserItem();
        con_user_wid->SetInfo(0, names[name_i], heads[head_i]);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);    // 追加到列表尾
        this->setItemWidget(item, con_user_wid);
    }
}

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
    auto* widget = this->itemWidget(item);
    if (!widget)
    {
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto* customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == ListItemType::INVALID_ITEM || itemType == ListItemType::GROUP_TIP_ITEM)
    {
        qDebug()<< "slot invalid item clicked ";
        return;
    }
    if (itemType == ListItemType::APPLY_FRIEND_ITEM)
    {
        qDebug()<< "apply friend item clicked ";
        // 跳转到好友申请界面
        emit sig_switch_apply_friend_page();
        return;
    }
    if (itemType == ListItemType::CONTACT_USER_ITEM)
    {
        qDebug()<< "contact user item clicked ";
        auto user_item = qobject_cast<ContactUserItem*>(customItem);
        emit sig_switch_friend_info_page(user_item->GetInfo());
        return;
    }
}

/**
 * @brief ContactUserList::slot_add_auth_firend
 */
void ContactUserList::slot_add_auth_firend(std::shared_ptr<AuthInfo>auth_info)
{
    qDebug() << "slot add auth friend ";

    // 在 groupitem 之后插入新项
    auto* con_user_wid = new ContactUserItem();
    con_user_wid->SetInfo(auth_info);
    auto* item = new QListWidgetItem();
    item->setSizeHint(con_user_wid->sizeHint());
    int index = this->row(_groupitem);  // 获取 groupitem 的索引
    this->insertItem(index+1, item);
    this->setItemWidget(item, con_user_wid);
}

/**
 * @brief ContactUserList::slot_auth_rsp
 */
void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "slot auth rsp called";

    // 在 groupitem 之后插入新项
    auto* con_user_wid = new ContactUserItem();
    con_user_wid->SetInfo(auth_rsp);
    auto* item = new QListWidgetItem();
    item->setSizeHint(con_user_wid->sizeHint());
    int index = this->row(_groupitem);  // 获取 groupitem 的索引
    this->insertItem(index+1, item);
    this->setItemWidget(item, con_user_wid);
}
