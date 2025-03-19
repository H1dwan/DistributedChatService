#include "friendinfoitem.h"
#include "ui_friendinfoitem.h"

FriendInfoItem::FriendInfoItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendInfoItem)
{
    ui->setupUi(this);
}

FriendInfoItem::~FriendInfoItem()
{
    delete ui;
}

void FriendInfoItem::SetBackName(QString back_name)
{
    ui->info_key->setText("备注");
    ui->info_value->setText(back_name);
}
