#ifndef APPLYFRIENDPAGE_H
#define APPLYFRIENDPAGE_H

#include <QWidget>
#include "userdata.h"
#include "applyfrienditem.h"

namespace Ui {
class ApplyFriendPage;
}

class ApplyFriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyFriendPage(QWidget *parent = nullptr);
    ~ApplyFriendPage();

    void AddNewApply(std::shared_ptr<AddFriendApply> apply);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void LoadApplyList();

private:
    Ui::ApplyFriendPage *ui;
    std::unordered_map<int, ApplyFriendItem*> _unauth_items;    // 未处理的好友申请

signals:
    void sig_show_searchpage(bool);

private slots:
    void slot_auth_rsp(std::shared_ptr<AuthRsp>);
};

#endif // APPLYFRIENDPAGE_H
