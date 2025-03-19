#ifndef APPLYFRIENDITEM_H
#define APPLYFRIENDITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ApplyFriendItem;
}

class ApplyFriendItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ApplyFriendItem(QWidget *parent = nullptr);
    ~ApplyFriendItem();

    void SetInfo(std::shared_ptr<ApplyInfo> apply_info);
    void ShowAddBtn(bool show = false);
    int GetUid();

    QSize sizeHint() const override {
        return QSize(480, 80); // 返回自定义的尺寸
    }

private:
    Ui::ApplyFriendItem *ui;

    bool _added;    // 好友申请是否已添加
    std::shared_ptr<ApplyInfo> _apply_info;

signals:
    // 向服务器发送验证请求的信号
    void sig_auth_friend(std::shared_ptr<ApplyInfo> apply_info);
};

#endif // APPLYFRIENDITEM_H
