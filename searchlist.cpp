#include "searchlist.h"
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QJsonDocument>
#include "tcpmgr.h"
#include "usermgr.h"
#include "adduseritem.h"
#include "findfaileddialog.h"
#include "findsuccessdlg.h"
#include "customizededit.h"


SearchList::SearchList(QWidget *parent)
    :QListWidget(parent), _send_pending(false), _search_edit(nullptr)
{
    // 关闭水平、垂直滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    // 连接搜索条目点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    // 添加条目
    _AddTipItem();
    // 连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}

void SearchList::SetSearchEdit(QWidget *edit)
{
    _search_edit = edit;
}

/**
 * @brief SearchList::CloseFindDlg
 */
void SearchList::CloseFindDlg()
{
    if (_find_dlg)
    {
        qDebug() << "find_dlg use count: " <<_find_dlg.use_count();
        _find_dlg->hide();
        _find_dlg = nullptr;
    }
}

bool SearchList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != this->viewport()) {
        return QListWidget::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::Enter:
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        break;
    case QEvent::Leave:
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        break;
    case QEvent::Wheel: {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int numSteps = wheelEvent->angleDelta().y() / 120; // 直接计算滚动步数
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - numSteps);
        return true; // 停止事件传递
    }
    default:
        break;
    }

    return QListWidget::eventFilter(watched, event);
}

/**
 * @brief SearchList::_waitPending  显示加载动画（在等待服务器回包的过程中）
 * @param isPending
 */
void SearchList::_waitPending(bool isPending)
{
    if (isPending)
    {
        _loadingDialog = new LoadingDialog(this);
        _loadingDialog->setModal(true);
        _loadingDialog->show();
    }
    else
    {
        _loadingDialog->hide();
        _loadingDialog->deleteLater();
    }
    _send_pending = isPending;
}

/**
 * @brief SearchList::_AddTipItem   用于向 QListWidget 中添加两个自定义的列表项
 */
void SearchList::_AddTipItem()
{
    // 添加不可选的提示项
    auto* invalid_item = new QWidget();
    invalid_item->setObjectName("invalid_item");
    auto* item_tmp = new QListWidgetItem();
    item_tmp->setSizeHint(QSize(250, 10));
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);
    this->addItem(item_tmp);
    this->setItemWidget(item_tmp, invalid_item);    // 将 invalid_item 设置为 item_tmp 的自定义控件

    // 添加自定义的用户项
    auto* add_user_item = new AddUserItem();
    auto* item = new QListWidgetItem();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

/**
 * @brief SearchList::slot_item_clicked
 * @param item
 */
void SearchList::slot_item_clicked(QListWidgetItem *item)
{
    auto* widget = this->itemWidget(item);  // 获取与 QListWidgetItem 关联的控件。
    if (!widget)
    {
        qDebug()<< "slot: item clicked widget is nullptr!";
        return;
    }

    auto* customItem = qobject_cast<ListItemBase*>(widget); // 将 widget 转换为 ListItemBase* 类型
    if (!customItem)
    {
        qDebug()<< "slot: item clicked widget is nullptr!";
        return;
    }

    auto itemType = customItem->GetItemType();  // 获取列表项的类型
    if (itemType == ListItemType::INVALID_ITEM)
    {
        qDebug()<< "slot: invalid item clicked!";
        return;
    }
    if (itemType == ListItemType::ADD_USER_TIP_ITEM)    // 创建一个 FindSuccessDlg 对话框并设置搜索信息
    {
        if (_send_pending || !_search_edit)
        {
            return;
        }

        _waitPending(true);
        auto search_edit = dynamic_cast<CustomizedEdit*>(_search_edit);
        auto uid_str = search_edit->text();
        // 发送 tcp 请求给 chat server
        QJsonObject json_obj {{"uid", uid_str}};
        QJsonDocument doc(json_obj);
        QByteArray json_data = doc.toJson(QJsonDocument::Compact);
        emit TcpMgr::GetInstance()->sig_send_data(ReqID::ID_SEARCH_USER_REQ, json_data);
        return;
    }
    // 清除弹出框
    CloseFindDlg();
}

/**
 * @brief SearchList::slot_user_search  处理搜索用户回包的逻辑
 * @param si 服务器返回的搜索信息
 */
void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    _waitPending(false);
    if (si == nullptr)  // 搜索失败，跳转失败界面
    {
        _find_dlg = std::make_shared<FindFailedDialog>(this);
    }
    else    // 搜索成功，根据 UID 分情况处理
    {
        // TODO: 搜索自己，暂且先直接返回
        auto self_uid = UserMgr::GetInstance()->GetUID();
        if (si->uid == self_uid) {
            return;
        }
        // 搜索已存在的好友
        if (UserMgr::GetInstance()->CheckFriendByUID(si->uid))
        {
            emit sig_jump_chat_item(si);
            return;
        }
        // 搜索不存在的好友，则跳转添加好友界面
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
    }
    _find_dlg->show();
}
