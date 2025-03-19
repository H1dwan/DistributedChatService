#include "findfaileddialog.h"
#include "ui_findfaileddialog.h"

FindFailedDialog::FindFailedDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindFailedDialog)
{
    ui->setupUi(this);

    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 设置属性
    this->setObjectName("FindFailDlg");
    ui->fail_sure_btn->SetState("normal", "hover", "press");
    this->setModal(true);
}

FindFailedDialog::~FindFailedDialog()
{
    qDebug()<< "Find FailDlg destruct";
    delete ui;
}

void FindFailedDialog::on_fail_sure_btn_clicked()
{
    this->hide();
    // this->deleteLater();
}

