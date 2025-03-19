#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>

enum class ListItemType;

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget* parent = nullptr);

    void SetItemType(ListItemType item_type);
    ListItemType GetItemType() const;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:

private:
    ListItemType _item_type;

};

/**
 * @brief The ListItemType enum     自定义的 QListItem 的类型
 */
enum class ListItemType
{
    CHAT_USER_ITEM,         // 聊天用户
    CONTACT_USER_ITEM,      // 联系人用户
    SEARCH_USER_ITEM,       // 搜索到的用户
    ADD_USER_TIP_ITEM,      // 提示添加用户
    GROUP_TIP_ITEM,         // 分组提示条目
    INVALID_ITEM,           // 不可点击条目
    LINE_ITEM,              // 分割线
    APPLY_FRIEND_ITEM,      // 好友申请
};

#endif // LISTITEMBASE_H
