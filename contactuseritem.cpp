#include "contactuseritem.h"
#include "ui_contactuseritem.h"

ContactUserItem::ContactUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ContactUserItem)
{
    ui->setupUi(this);

    SetItemType(ListItemType::CONTACT_USER_ITEM);

    ui->red_point->raise(); // 提升层级，将 red_point 置于顶层
    ShowRedPoint(false);
}

ContactUserItem::~ContactUserItem()
{
    delete ui;
}

QSize ContactUserItem::sizeHint() const
{
    return QSize(250, 70);  // 返回自定义的尺寸
}

void ContactUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info = std::make_shared<UserInfo>(auth_info);

    // 设置头像
    QPixmap pixmap(_info->icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 设置用户名
    ui->user_name_lb->setText(_info->name);
}

void ContactUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp)
{
    _info = std::make_shared<UserInfo>(auth_rsp);

    // 设置头像
    QPixmap pixmap(_info->icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 设置用户名
    ui->user_name_lb->setText(_info->name);
}

void ContactUserItem::SetInfo(int uid, QString name, QString icon)
{
    _info = std::make_shared<UserInfo>(uid, name, icon);

    // 设置头像
    QPixmap pixmap(_info->icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 设置用户名
    ui->user_name_lb->setText(_info->name);
}

std::shared_ptr<UserInfo> ContactUserItem::GetInfo()
{
    return _info;
}

void ContactUserItem::ShowRedPoint(bool show)
{
    if (show)
    {
        ui->red_point->show();
    }
    else
    {
        ui->red_point->hide();
    }
}
