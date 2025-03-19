#include "applyfriendpage.h"
#include "ui_applyfriendpage.h"
#include <QRandomGenerator>
#include <QPainter>
#include "applyfriendlist.h"
#include "authenfrienddialog.h"
#include "tcpmgr.h"
#include "usermgr.h"

ApplyFriendPage::ApplyFriendPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyFriendPage)
{
    ui->setupUi(this);

    LoadApplyList();
    // 连接隐藏搜索界面的信号
    connect(ui->apply_friend_list, &ApplyFriendList::sig_show_searchpage, this, &ApplyFriendPage::sig_show_searchpage);
    // 连接 tcp 传递的 authrsp 信号处理
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ApplyFriendPage::slot_auth_rsp);

}

ApplyFriendPage::~ApplyFriendPage()
{
    delete ui;
}

/**
 * @brief ApplyFriendPage::AddNewApply  在 receiver 新的朋友界面添加一条未处理的好友申请通知
 * @param apply 好友的申请数据，sender的数据
 */
void ApplyFriendPage::AddNewApply(std::shared_ptr<AddFriendApply> apply)
{
    auto apply_info = std::make_shared<ApplyInfo>(apply);
    // 创建一个好友申请条目
    auto* apply_item = new ApplyFriendItem();
    apply_item->SetInfo(apply_info);
    // 将条目插入到申请列表中
    auto* item = new QListWidgetItem();
    item->setSizeHint(apply_item->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    ui->apply_friend_list->insertItem(0, item);
    ui->apply_friend_list->setItemWidget(item, apply_item);

    // 将该 item 加入到未处理的好友申请列表中
    auto uid = apply_item->GetUid();
    _unauth_items[uid] = apply_item;
    apply_item->ShowAddBtn(true);

    // 收到审核好友添加的信号
    connect(apply_item, &ApplyFriendItem::sig_auth_friend, this, [this](std::shared_ptr<ApplyInfo> apply_info){
        auto* authfriend_dlg = new AuthenFriendDialog(this);
        authfriend_dlg->SetApplyInfo(apply_info);
        authfriend_dlg->setModal(true);
        authfriend_dlg->show();
    });
}

void ApplyFriendPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

/**
 * @brief ApplyFriendPage::LoadApplyList    加载好友申请列表
 */
void ApplyFriendPage::LoadApplyList()
{
    // 添加好友申请
    auto apply_list = UserMgr::GetInstance()->GetApplyList();
    for (auto& apply : apply_list)
    {
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int head_i = randomValue % heads.size();
        apply->SetIcon(heads[head_i]);
        auto* apply_item = new ApplyFriendItem();
        apply_item->SetInfo(apply);
        auto* item = new QListWidgetItem();
        item->setSizeHint(apply_item->sizeHint());
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        // 头插法
        ui->apply_friend_list->insertItem(0, item);
        ui->apply_friend_list->setItemWidget(item, apply_item);

        if (apply->status)
        {
            apply_item->ShowAddBtn(false);
        }
        else
        {
            apply_item->ShowAddBtn(true);
            auto uid = apply_item->GetUid();
            _unauth_items[uid] = apply_item;
        }

        // 收到审核好友信号
        connect(apply_item, &ApplyFriendItem::sig_auth_friend, this, [this](std::shared_ptr<ApplyInfo> apply_info){
            auto* authfriend_dlg = new AuthenFriendDialog(this);
            authfriend_dlg->SetApplyInfo(apply_info);   // sender的信息
            authfriend_dlg->setModal(true);
            authfriend_dlg->show();
        });
    }
}

/**
 * @brief ApplyFriendPage::slot_auth_rsp
 * @param auth_rsp
 */
void ApplyFriendPage::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    auto uid = auth_rsp->uid;
    auto find_iter = _unauth_items.find(uid);
    if (find_iter == _unauth_items.end())
    {
        return;
    }

    find_iter->second->ShowAddBtn(false);
}
