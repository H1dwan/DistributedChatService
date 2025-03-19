#ifndef CONTACTUSERITEM_H
#define CONTACTUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ContactUserItem;
}

class ContactUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ContactUserItem(QWidget *parent = nullptr);
    ~ContactUserItem();

    QSize sizeHint() const override;

    void SetInfo(std::shared_ptr<AuthInfo>);
    void SetInfo(std::shared_ptr<AuthRsp>);
    void SetInfo(int uid, QString name, QString icon);
    std::shared_ptr<UserInfo> GetInfo();

    void ShowRedPoint(bool show = true);

private:
    Ui::ContactUserItem *ui;
    std::shared_ptr<UserInfo> _info;
};

#endif // CONTACTUSERITEM_H
