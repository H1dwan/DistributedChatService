#include "applyfriend.h"
#include "ui_applyfriend.h"
#include <QScrollBar>
#include <QEvent>
#include <QJsonDocument>
#include "usermgr.h"
#include "tcpmgr.h"
#include "global.h"


ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriend)
{
    ui->setupUi(this);

    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);

    ui->name_edit->setPlaceholderText("CHAEWON");
    ui->lb_edit->setPlaceholderText("搜索、添加标签");
    ui->back_edit->setPlaceholderText("LE SSERAFIM");

    // 设置 lb_edit 属性
    ui->lb_edit->SetMaxLength(21);
    ui->lb_edit->move(2, 2);
    ui->lb_edit->setFixedHeight(20);
    ui->lb_edit->SetMaxLength(10);
    ui->input_tip_wid->hide();  // 输入提示框

    _tip_cur_point = QPoint(5, 5);

    // TODO: 测试用的标签
    _tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                 "父与子学Python","nodejs开发指南","go 语言开发指南",
                 "游戏伙伴","金融投资","微信读书","拼多多拼友" };
    InitTipLbs();

    // 连接展示更多标签事件
    connect(ui->more_lb, &ClickedOneceLabel::clicked, this, &ApplyFriend::ShowMoreLabel);

    // 连接输入标签回车事件
    connect(ui->lb_edit, &CustomizedEdit::returnPressed, this, &ApplyFriend::SlotLabelEnter);
    connect(ui->lb_edit, &CustomizedEdit::textChanged, this, &ApplyFriend::SlotLabelTextChange);
    connect(ui->lb_edit, &CustomizedEdit::editingFinished, this, &ApplyFriend::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickedOneceLabel::clicked, this, &ApplyFriend::SlotAddFirendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);

    ui->sure_btn->SetState("normal", "hover", "press");
    ui->cancel_btn->SetState("normal", "hover", "press");

    // 连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplySure);
}

ApplyFriend::~ApplyFriend()
{
    qDebug()<< "ApplyFriend destruct";
    delete ui;
}

void ApplyFriend::InitTipLbs()
{
    int lines = 1;
    for (size_t i = 0; i < _tip_data.size(); ++i)
    {
        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed",
                     "selected_normal", "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        //
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取 QLabel 控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度
        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width())
        {
            lines++;
            if (lines > 2)
            {
                delete lb;
                return;
            }
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        auto next_point = _tip_cur_point;
        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }
}

/**
 * @brief ApplyFriend::AddTipLbs
 * @param lb
 * @param cur_pt
 * @param next_pt
 * @param text_width
 * @param text_height
 */
void ApplyFriend::AddTipLbs(ClickedLabel* lb, QPoint cur_pt, QPoint &next_pt, int text_width, int text_height)
{
    lb->move(cur_pt);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_pt.setX(lb->pos().x() + text_width + 15);
    next_pt.setY(lb->pos().y());
}

void ApplyFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto apply_name = UserMgr::GetInstance()->GetName();
    auto back_name = si->name;
    ui->name_edit->setText(apply_name);
    ui->back_edit->setText(back_name);
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea)
    {
        if (event->type() == QEvent::Enter)
        {
            ui->scrollArea->verticalScrollBar()->setHidden(false);
        }
        else if (event->type() == QEvent::Leave)
        {
            ui->scrollArea->verticalScrollBar()->setHidden(true);
        }
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriend::AddLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end())
    {
        ui->lb_edit->clear();
        return;
    }

    auto tmp_label = new FriendLabel(ui->gridWidget);
    tmp_label->SetText(name);
    tmp_label->setObjectName("FriendLabel");

    auto max_width = ui->gridWidget->width();
    // TODO: ... 添加宽度统计
    if (_label_point.x() + tmp_label->width() > max_width)
    {
        _label_point.setY(_label_point.y() + tmp_label->height() + 6);
        _label_point.setX(2);
    }
    else
    {

    }

    tmp_label->move(_label_point);
    tmp_label->show();
    _friend_labels[tmp_label->Text()] = tmp_label;
    _friend_label_keys.push_back(tmp_label->Text());

    connect(tmp_label, &FriendLabel::sig_close, this, &ApplyFriend::SlotRemoveFriendLabel);

    _label_point.setX(_label_point.x() + tmp_label->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width())
    {
        ui->lb_edit->move(2, _label_point.y() + tmp_label->height() + 2);
    }
    else
    {
        ui->lb_edit->move(_label_point);
    }

    ui->lb_edit->clear();

    if (ui->gridWidget->height() < _label_point.y() + tmp_label->height() + 2)
    {
        ui->gridWidget->setFixedHeight(_label_point.y() + tmp_label->height() * 2 + 2);
    }
}

void ApplyFriend::ResetLabels()
{
    auto max_width = ui->gridWidget->width();
    auto label_height = 0;
    for(auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++)
    {
        // TODO: ... 添加宽度统计
        if( _label_point.x() + iter.value()->width() > max_width)
        {
            _label_point.setY(_label_point.y()+iter.value()->height()+6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x()+iter.value()->width()+2);
        _label_point.setY(_label_point.y());
        label_height = iter.value()->height();
    }

    if(_friend_labels.isEmpty())
    {
        ui->lb_edit->move(_label_point);
        return;
    }

    if(_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->gridWidget->width())
    {
        ui->lb_edit->move(2,_label_point.y()+label_height+6);
    }
    else
    {
        ui->lb_edit->move(_label_point);
    }
}

void ApplyFriend::ShowMoreLabel()
{
    qDebug()<< "receive more label clicked";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth;
    int textHeight;
    //重拍现有的label
    for(auto & added_key : _add_label_keys){
        auto added_lb = _add_labels[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth = fontMetrics.horizontalAdvance(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height(); // 获取文本的高度

        if(_tip_cur_point.x() +textWidth + tip_offset > ui->lb_list->width()){
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y()+textHeight+15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());

        _tip_cur_point = next_point;

    }

    //添加未添加的
    for(size_t i = 0; i < _tip_data.size(); i++){
        auto iter = _add_labels.find(_tip_data[i]);
        if(iter != _add_labels.end()){
            continue;
        }

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->SetState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height(); // 获取文本的高度

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

        }

        next_point = _tip_cur_point;

        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);

        _tip_cur_point = next_point;

    }

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    //qDebug()<<"after resize ui->lb_list size is " <<  ui->lb_list->size();
    ui->scrollAreaWidgetContents->setFixedHeight(ui->scrollAreaWidgetContents->height()+diff_height);
}

void ApplyFriend::SlotLabelEnter()
{
    if (ui->lb_edit->text().isEmpty())
    {
        return;
    }

    auto text = ui->lb_edit->text();
    AddLabel(text);
    ui->input_tip_wid->hide();

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    // 找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    // 判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollAreaWidgetContents->setFixedHeight(ui->scrollAreaWidgetContents->height() + diff_height);
}

void ApplyFriend::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    _label_point.setX(2);
    _label_point.setY(6);

    auto find_iter = _friend_labels.find(name);

    if(find_iter == _friend_labels.end()){
        return;
    }

    auto find_key = _friend_label_keys.end();
    for(auto iter = _friend_label_keys.begin(); iter != _friend_label_keys.end();
         iter++){
        if(*iter == name){
            find_key = iter;
            break;
        }
    }

    if(find_key != _friend_label_keys.end()){
        _friend_label_keys.erase(find_key);
    }


    delete find_iter.value();

    _friend_labels.erase(find_iter);

    ResetLabels();

    auto find_add = _add_labels.find(name);
    if(find_add == _add_labels.end()){
        return;
    }

    find_add.value()->ResetNormalState();
}

void ApplyFriend::SlotChangeFriendLabelByTip(QString lbtext, ClickLbState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if(find_iter == _add_labels.end()){
        return;
    }

    if(state == ClickLbState::Selected){
        //编写添加逻辑
        qDebug() << "111111";
        AddLabel(lbtext);
        return;
    }

    if(state == ClickLbState::Normal){
        //编写删除逻辑
        qDebug() << "22222";
        SlotRemoveFriendLabel(lbtext);
        return;
    }
}

void ApplyFriend::SlotLabelTextChange(const QString &text)
{
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_tip_wid->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        auto new_text = add_prefix + text;
        ui->tip_lb->setText(new_text);
        ui->input_tip_wid->show();
        return;
    }
    ui->tip_lb->setText(text);
    ui->input_tip_wid->show();
}

void ApplyFriend::SlotLabelEditFinished()
{
    ui->input_tip_wid->hide();
}

void ApplyFriend::SlotAddFirendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    AddLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    //找到了就只需设置状态为选中即可
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    //判断标签展示栏是否有该标签
    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    //标签展示栏也增加一个标签, 并设置绿色选中
    auto* lb = new ClickedLabel(ui->lb_list);
    lb->SetState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &ApplyFriend::SlotChangeFriendLabelByTip);
    qDebug() << "ui->lb_list->width() is " << ui->lb_list->width();
    qDebug() << "_tip_cur_point.x() is " << _tip_cur_point.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height(); // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (_tip_cur_point.x() + textWidth+ tip_offset+3 > ui->lb_list->width()) {

        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);

    }

    auto next_point = _tip_cur_point;

    AddTipLbs(lb, _tip_cur_point, next_point, textWidth,textHeight);
    _tip_cur_point = next_point;

    int diff_height = next_point.y() + textHeight + tip_offset - ui->lb_list->height();
    ui->lb_list->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollAreaWidgetContents->setFixedHeight(ui->scrollAreaWidgetContents->height()+ diff_height );
}

/**
 * @brief ApplyFriend::SlotApplySure    申请添加好友，(添加好友到通讯录确认按钮)
 */
void ApplyFriend::SlotApplySure()
{
    qDebug()<<"Slot Apply Sure called" ;
    // 发送添加好友请求的逻辑
    QJsonObject json_obj;
    json_obj["uid"] = UserMgr::GetInstance()->GetUID();
    json_obj["applyname"] = ui->name_edit->text().isEmpty() ?
                                ui->name_edit->placeholderText() : ui->name_edit->text();
    json_obj["backname"] = ui->back_edit->text().isEmpty() ?
                              ui->back_edit->placeholderText() : ui->back_edit->text();
    json_obj["touid"] = _si->uid;
    QJsonDocument doc(json_obj);
    auto json_data = doc.toJson(QJsonDocument::Compact);
    // 发送 tcp 请求给 ChatServer
    emit TcpMgr::GetInstance()->sig_send_data(ReqID::ID_ADD_FRIEND_REQ, json_data);
    // 关闭添加好友界面
    this->hide();
    this->deleteLater();
}

void ApplyFriend::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

