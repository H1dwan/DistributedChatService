#include "messagetextedit.h"
#include <QMimeData>
#include <QMimeDatabase>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QPainter>
#include <QFileIconProvider>

MessageTextEdit::MessageTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    this->setMaximumHeight(60);
}


/**
 * @brief MessageTextEdit::GetMsgList   从 MessageTextEdit 的文档中提取文本和嵌入对象，并将其存储到 m_GetMsgList 中
 * @return
 */
QVector<MsgInfo> MessageTextEdit::GetMsgList()
{
    // 初始化
    m_GetMsgList.clear();
    QString doc = this->document()->toPlainText();  // Edit 中的纯文本内容
    QString text="";// 存储文本信息
    int indexUrl = 0;
    int count = m_MsgList.size();

    // 遍历文档的每一个字符
    for(int index=0; index < doc.size(); index++)
    {
        // 处理嵌入的对象（例如图片、表情等）
        if(doc[index] == QChar::ObjectReplacementCharacter)
        {
            // 如果 text 不为空，将当前文本片段添加到 m_GetMsgList 中，并清空 text
            if(!text.isEmpty())
            {
                QPixmap pix;
                InsertMsgList(m_GetMsgList, "text", text, pix);
                text.clear();
            }
            // 遍历 m_MsgList，查找与当前嵌入对象匹配的消息（通过 HTML 内容匹配）
            while(indexUrl < count)
            {
                MsgInfo msg =  m_MsgList[indexUrl];
                if(this->document()->toHtml().contains(msg.content, Qt::CaseSensitive))
                {
                    indexUrl++;
                    m_GetMsgList.append(msg);
                    break;
                }
                indexUrl++;
            }
        }
        else    // 处理普通文本
        {
            text.append(doc[index]);
        }
    }
    // 处理剩余的文本
    if(!text.isEmpty())
    {
        QPixmap pix;
        InsertMsgList(m_GetMsgList, "text", text, pix);
        text.clear();
    }
    m_MsgList.clear();
    this->clear();
    return m_GetMsgList;
}


/**
 * @brief MessageTextEdit::InsertFileFromUrl    根据 URL 列表插入文件内容到 QTextEdit 中
 * @param urls
 */
void MessageTextEdit::InsertFileFromUrl(const QStringList &urls)
{
    if (urls.isEmpty())
    {
        return;
    }

    foreach (QString url, urls) {
        if (IsImage(url))
        {
            InsertImages(url);
        }
        else
        {
            InsertTextFile(url);
        }
    }
}

/**
 * @brief MessageTextEdit::dragEnterEvent   处理拖动事件的进入，当用户尝试将某个对象拖入到 MessageTextEdit 控件中时，该函数会被调用
 * @param event
 */
void MessageTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查拖动的源是否是当前的 QTextEdit 控件
    if (event->source() == this)
    {
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

/**
 * @brief MessageTextEdit::dropEvent    用于处理拖拽操作释放时的事件，使得MessageTextEdit能够支持拖拽文件或图片的功能
 * @param event
 */
void MessageTextEdit::dropEvent(QDropEvent *event)
{

    InsertFromMimeData(event->mimeData());
    // 表示接受并处理了拖拽释放事件，防止事件传递到父类
    event->accept();
}

/**
 * @brief MessageTextEdit::keyPressEvent    该函数用于处理 QTextEdit 的按键事件，支持按下 Enter 键触发 sig_send 信号。
 * @param event
 */
void MessageTextEdit::keyPressEvent(QKeyEvent *event)
{
    // 检查是否按下了 Enter 或 Return 键，且没有按下 Shift 键
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) &&
        !(event->modifiers() & Qt::ShiftModifier)) {
        qDebug() << "Enter key pressed without Shift";
        emit sig_send();
        return;
    }

    // 默认按键事件处理
    QTextEdit::keyPressEvent(event);
}

bool MessageTextEdit::IsImage(const QString &url) const
{
    QString imageFormat = "bmp,jpg,png,tif,gif,pcx,tga,exif,fpx,svg,psd,cdr,pcd,dxf,ufo,eps,ai,raw,wmf,webp";
    QStringList imageFormatList = imageFormat.split(",");
    QFileInfo fileInfo(url);
    QString suffix = fileInfo.suffix();
    if(imageFormatList.contains(suffix,Qt::CaseInsensitive)){
        return true;
    }
    return false;
}

bool MessageTextEdit::CanInsertFromMimeData(const QMimeData *source) const
{
    return QTextEdit::canInsertFromMimeData(source);
}

/**
 * @brief MessageTextEdit::InsertImages 将图片插入到 MessageTextEdit 中，并将图片信息添加到消息列表（m_MsgList）中
 * @param url
 */
void MessageTextEdit::InsertImages(const QString &url)
{
    // 加载并缩放图片
    QImage image(url);
    if (image.width() > 120 || image.height() > 80)
    {
        if(image.width() > image.height())
        {
            image =  image.scaledToWidth(120, Qt::SmoothTransformation);
        }
        else
            image = image.scaledToHeight(80, Qt::SmoothTransformation);
    }
    // 将图片插入到光标位置
    QTextCursor cursor = this->textCursor();
    cursor.insertImage(image, url);
    // 将图片信息添加到消息列表
    InsertMsgList(m_MsgList, "image", url, QPixmap::fromImage(image));
}

/**
 * @brief MessageTextEdit::InsertTextFile   用于将文本文件的信息插入到 MessageTextEdit 中，并将文件信息添加到消息列表（m_MsgList）中
 * @param url
 */
void MessageTextEdit::InsertTextFile(const QString &url)
{
    QFileInfo fileInfo(url);
    // 检查文件是否为目录
    if(fileInfo.isDir())
    {
        QMessageBox::information(this, "提示", "只允许拖拽单个文件!");
        return;
    }
    // 检查文件大小
    if(fileInfo.size() > 100*1024*1024)
    {
        QMessageBox::information(this, "提示", "发送的文件大小不能大于100M");
        return;
    }
    // 获取文件图标并插入到光标位置
    QPixmap pix = GetFileIconPixmap(url);
    QTextCursor cursor = this->textCursor();
    cursor.insertImage(pix.toImage(), url);
    // 将文件信息添加到消息列表
    InsertMsgList(m_MsgList, "file", url, pix);
}

/**
 * @brief MessageTextEdit::InsertFromMimeData
 *        从 QMimeData 中提取 URL 列表，并根据文件类型（图片或文本文件）插入到 MessageTextEdit 中
 * @param source
 */
void MessageTextEdit::InsertFromMimeData(const QMimeData *source)
{
    // 从 QMimeData 的文本内容中提取 URL 列表
    // qDebug() << source->text();
    QStringList urls = GetUrl(source->text());

    if(urls.isEmpty())
    {
        return;
    }

    foreach (QString url, urls)
    {
        qDebug() << url;
        if(IsImage(url))
            InsertImages(url);
        else
            InsertTextFile(url);
    }
}

/**
 * @brief MessageTextEdit::InsertMsgList    将消息信息（MsgInfo）添加到消息列表（QVector<MsgInfo>）中
 * @param list  传出参数
 * @param flag  消息类型（例如 "text", "image", "file"）
 * @param text  消息内容（例如文本、文件路径、图片 URL）
 * @param pix   消息图标（例如文件图标、图片缩略图）
 */
void MessageTextEdit::InsertMsgList(QVector<MsgInfo> &list, QString flag, QString text, QPixmap pix)
{
    // 参数校验
    if (flag.isEmpty() || text.isEmpty()) {
        qWarning() << "Invalid message flag or text";
        return;
    }
    // 创建 MsgInfo 对象
    MsgInfo msg {flag, text, pix};
     // 添加到消息列表
    list.append(std::move(msg));
}

/**
 * @brief MessageTextEdit::GetUrl   用于从文本中提取 URL 列表，假设文本中的 URL 以 /// 分隔，提取 /// 后的部分作为 URL
 * @param text
 * @return
 */
QStringList MessageTextEdit::GetUrl(QString text)
{
    QStringList urls;
    if (text.isEmpty()) {
        qWarning() << "Input text is empty";
        return urls;
    }

    QStringList list = text.split("\n");
    foreach (QString url, list) {
        if(!url.isEmpty()){
            QStringList str = url.split("///");
            if(str.size()>=2)
                urls.append(str.at(1));
        }
    }

    return urls;
}

/**
 * @brief MessageTextEdit::GetFileSize  用于将文件大小（以字节为单位）转换为更易读的格式（例如 B, KB, MB, GB）
 * @param size
 * @return
 */
QString MessageTextEdit::GetFileSize(qint64 size)
{
    QString Unit;
    double num;
    if(size < 1024){
        num = size;
        Unit = "B";
    }
    else if(size < 1024 * 1224){
        num = size / 1024.0;
        Unit = "KB";
    }
    else if(size <  1024 * 1024 * 1024){
        num = size / 1024.0 / 1024.0;
        Unit = "MB";
    }
    else{
        num = size / 1024.0 / 1024.0/ 1024.0;
        Unit = "GB";
    }
    return QString::number(num, 'f', 2) + " " + Unit;
}

/**
 * @brief MessageTextEdit::GetFileIconPixmap 生成一个包含文件图标、文件名和文件大小的 QPixmap
 * @param url
 * @return
 */
QPixmap MessageTextEdit::GetFileIconPixmap(const QString &url)
{
    // 获取文件的图标
    QFileIconProvider provder;
    QFileInfo fileinfo(url);
    QIcon icon = provder.icon(fileinfo);

    QString strFileSize = GetFileSize(fileinfo.size());
    //qDebug() << "FileSize=" << fileinfo.size();

    // 设置字体
    QFont font(QString("宋体"), 10, QFont::Normal,false);
    QFontMetrics fontMetrics(font);
    QSize textSize = fontMetrics.size(Qt::TextSingleLine, fileinfo.fileName());
    // 计算文本尺寸
    QSize FileSize = fontMetrics.size(Qt::TextSingleLine, strFileSize);
    int maxWidth = textSize.width() > FileSize.width() ? textSize.width() :FileSize.width();
    // 创建 QPixmap
    QPixmap pix(50 + maxWidth + 10, 50);
    pix.fill();
    // 绘制内容
    QPainter painter;
    painter.begin(&pix);
    // 文件图标
    QRect rect(0, 0, 50, 50);
    painter.drawPixmap(rect, icon.pixmap(40,40));
    painter.setPen(Qt::black);
    // 在图标右侧绘制文件名
    QRect rectText(50+10, 3, textSize.width(), textSize.height());
    painter.drawText(rectText, fileinfo.fileName());
    // 在文件名下方绘制文件大小
    QRect rectFile(50+10, textSize.height()+5, FileSize.width(), FileSize.height());
    painter.drawText(rectFile, strFileSize);
    painter.end();
    return pix;
}

void MessageTextEdit::SlotTextEditChanged()
{
    qDebug() << "text changed!";
}
