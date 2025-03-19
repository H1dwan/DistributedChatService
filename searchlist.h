#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QListWidget>
#include "userdata.h"
#include "loadingdialog.h"

class SearchList : public QListWidget
{
    Q_OBJECT
public:
    SearchList(QWidget *parent = nullptr);
    void SetSearchEdit(QWidget* edit);
    void CloseFindDlg();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void _waitPending(bool isPending = true);
    void _AddTipItem();

private:
    bool _send_pending;     // 是否在发送（向服务器请求）
    QWidget* _search_edit;
    LoadingDialog * _loadingDialog;
    std::shared_ptr<QDialog> _find_dlg; // 查找框（基类指针指向派生类，多态）

signals:
    void sig_jump_chat_item(std::shared_ptr<SearchInfo> si);

private slots:
    void slot_item_clicked(QListWidgetItem *item);
    void slot_user_search(std::shared_ptr<SearchInfo> si);
};

#endif // SEARCHLIST_H
