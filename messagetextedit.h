#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QTextEdit>
#include "global.h"

class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget* parent = nullptr);

    QVector<MsgInfo> GetMsgList();
    void InsertFileFromUrl(const QStringList& urls);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool IsImage(const QString& url) const; // 判断文件是否为图片
    bool CanInsertFromMimeData(const QMimeData* source) const;

    void InsertImages(const QString& url);
    void InsertTextFile(const QString& url);
    void InsertFromMimeData(const QMimeData* source);
    void InsertMsgList(QVector<MsgInfo>& list, QString flag, QString text, QPixmap pix);

    QStringList GetUrl(QString text);
    QString GetFileSize(qint64 size);   // 获取文件大小
    QPixmap GetFileIconPixmap(const QString& url);  // 获取文件图标及大小信息，并转化成图片

private:
    QVector<MsgInfo> m_MsgList;     // 存储原始的消息列表（可能包含文本和嵌入的对象）
    QVector<MsgInfo> m_GetMsgList;  // 存储提取后的消息列表

signals:
    void sig_send();

private slots:
    void SlotTextEditChanged();
};

#endif // MESSAGETEXTEDIT_H
