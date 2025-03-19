#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"

ApplyFriendItem::ApplyFriendItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ApplyFriendItem)
    , _added(false)
{
    ui->setupUi(this);

    SetItemType(ListItemType::APPLY_FRIEND_ITEM);

    ui->addBtn->SetState("normal", "hover",  "press");
    ui->addBtn->hide();

    // 连接同意添加好友的信号和槽
    connect(ui->addBtn, &ClickedBtn::clicked, [this]() {
        emit sig_auth_friend(_apply_info);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;

    QPixmap pixmap(_apply_info->icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(_apply_info->name);
    ui->user_chat_lb->setText(_apply_info->description);
}

void ApplyFriendItem::ShowAddBtn(bool show)
{
    if (show)
    {
        ui->addBtn->show();
        ui->already_add_lb->hide();
        _added = false;
    }
    else
    {
        ui->addBtn->hide();
        ui->already_add_lb->show();
        _added = true;
    }
}

int ApplyFriendItem::GetUid()
{
    return _apply_info->uid;
}
