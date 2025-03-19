#include "friendinfopage.h"
#include "ui_friendinfopage.h"
#include "friendinfoitem.h"

FriendInfoPage::FriendInfoPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FriendInfoPage), _friend_info(nullptr)
{
    ui->setupUi(this);
    ui->msg_chat->SetState("normal", "hover", "press");
    ui->voice_chat->SetState("normal","hover","press");
    ui->video_chat->SetState("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::SetInfo(std::shared_ptr<FriendInfo> friend_info)
{
    _friend_info = friend_info;
    // 设置头像
    QPixmap pixmap(friend_info->icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    // 设置用户名
    ui->name_lb->setText(friend_info->name);
    // 设置 UID
    ui->uid_lb->setText(QString("UID: %1").arg(friend_info->uid));
    // 清空之前的 info 条目
    ui->friend_info_list->clear();
    // 设置备注名
    auto back_name = friend_info->back.isEmpty() ? QString("点击添加备注") : friend_info->back;
    auto* info_item = new FriendInfoItem();
    info_item->SetBackName("点击添加备注");
    auto* list_item = new QListWidgetItem();
    list_item->setSizeHint(info_item->sizeHint()); // 设置大小
    ui->friend_info_list->addItem(list_item);
    ui->friend_info_list->setItemWidget(list_item, info_item);
    ui->friend_info_list->adjustSize();
}

void FriendInfoPage::on_msg_chat_clicked()
{
    qDebug() << "msg chat label clicked";
    emit sig_jump_chat_page(_friend_info);
}


