#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include <QDir>
#include "applyfriend.h"

FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindSuccessDlg)
    , _parent(parent)
{
    ui->setupUi(this);

    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 设置按钮状态
    ui->add_friend_btn->SetState("normal", "hover", "press");
    // 阻止用户与其他窗口交互，直到该模态窗口关闭
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg()
{
    delete ui;
}

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    // 加载头像
    QPixmap head_pix(_si->icon);
    head_pix = head_pix.scaled(ui->head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    // 加载用户名
    ui->name_lb->setText(_si->name);
}


void FindSuccessDlg::on_add_friend_btn_clicked()
{
    this->hide();
    auto applyFriendPage = new ApplyFriend(_parent);
    applyFriendPage->SetSearchInfo(_si);
    applyFriendPage->setModal(true);
    applyFriendPage->show();
}

