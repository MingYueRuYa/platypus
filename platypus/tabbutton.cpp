#include "tabbutton.h"
#include "systemscale.h"

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtGui/QPainter>

#include <Windows.h>
#include <algorithm>

const QString k1KRESOURCE = ":/1kplatypus/image/default_100_precent/%1";
const QString k2KRESOURCE = ":/2kplatypus/image/default_200_precent/%1";

int TabButton::sMAX_HIEGHT   = 42;
int TabButton::sMAX_WIDTH    = 200;
int TabButton::sMIN_WIDTH    = 60;

std::atomic_int TabButton::sID;
const int kBTN_CLOSE_WIDTH  = 16;
const int kLINE_HEIGHT      = 22;
const int kBTN_CLOSE_DISTANCE       = 8;
const int kWEBSITE_ICON_DISTANCE    = 10;

TabButton::TabButton(const QString &url, int width, int height, QWidget *parent)
    : QPushButton(parent)
{
    this->setFixedSize(width, height);
    this->setMouseTracking(true);

    mID                 = ++sID;
    mShowIndex          = 0;
    mPushStatus         = PushStatus::Normal;
    mHighTextColor      = QColor("#FFFFFF");
    mNormalTextColor    = QColor("#AAAAAA");

    double dpi_scale    = SystemScaleInstance.GetScalePrecision();
    const int kbtn_close_with	= kBTN_CLOSE_WIDTH*dpi_scale;
    mBtnCloseRect				= QRect(width-kbtn_close_with-kBTN_CLOSE_DISTANCE, (height-kbtn_close_with)/2, kbtn_close_with, kbtn_close_with);

    double width_distance   = 32*dpi_scale;
    mTextRectF              = QRectF(width_distance, 0, width-width_distance-9-kbtn_close_with, height);
    mWebsiteRect            = QRect(kWEBSITE_ICON_DISTANCE/200.0*width, (height-kbtn_close_with)/2, kbtn_close_with, kbtn_close_with);
 
    mFont               = QFont("Microsoft YaHei", 9);
    mCloseBtnIsHover    = false;
    mDelete             = false;

    bool is_scale = SystemScaleInstance.IsScale();

    mNormalBK   = QPixmap(is_scale ? k2KRESOURCE.arg("tab.png") : k1KRESOURCE.arg("tab.png"));
    mHoverBK    = QPixmap(is_scale ? k2KRESOURCE.arg("tab_hover.png") : k1KRESOURCE.arg("tab_hover.png"));
    mPushBK     = QPixmap(is_scale ? k2KRESOURCE.arg("tab_check.png") : k1KRESOURCE.arg("tab_check.png"));
    mNormalBtnClose = QPixmap(is_scale ? k2KRESOURCE.arg("tab_close.png") : k1KRESOURCE.arg("tab_close.png"));
    mHoverBtnClose  = QPixmap(is_scale ? k2KRESOURCE.arg("tab_close_hover.png") : k1KRESOURCE.arg("tab_close_hover.png"));
    mPushBtnClose   = QPixmap(is_scale ? k2KRESOURCE.arg("tab_close_pressed.png") : k1KRESOURCE.arg("tab_close_pressed.png"));

    initStatsMap();

    mIconUrl = QUrl(url);
}

TabButton::~TabButton()
{
}

void TabButton::SetPushStats(const PushStatus pushStatus)
{
    mPushStatus = pushStatus;

    update();
}

void TabButton::SetText(const QString &title)
{
    if (title == mTitle) { return; }

    mTitle = title;
    update(); 
}

void TabButton::Resize(const QSize &size)
{
    int adapt_width = std::max<int>(std::min<int>(
        sMAX_WIDTH*SystemScaleInstance.GetScalePrecision(), size.width()),
        sMIN_WIDTH*SystemScaleInstance.GetScalePrecision());

    this->setFixedSize(QSize(adapt_width, size.height()));

    const int kbtn_close_with  = kBTN_CLOSE_WIDTH*SystemScaleInstance.GetScalePrecision();

    mBtnCloseRect.setLeft(adapt_width-kbtn_close_with-kBTN_CLOSE_DISTANCE);
    mBtnCloseRect.setWidth(kbtn_close_with);

    mWebsiteRect.setLeft(10/200.0*adapt_width);
    mWebsiteRect.setWidth(kbtn_close_with);

    double width_distance   = 32*SystemScaleInstance.GetScalePrecision();
    mTextRectF              = QRectF(width_distance, 0, adapt_width-width_distance-9-kbtn_close_with, size.height());

    update();
}

void TabButton::SetDelete(bool isDelete)
{
    mDelete = isDelete;
}

bool TabButton::GetDelete() const
{
    return mDelete;
}

void TabButton::SetShowIndex(int index)
{
    mShowIndex = index;
}

int TabButton::GetShowIndex() const
{
    return mShowIndex;
}

int TabButton::GetID() const
{
    return mID;
}

bool TabButton::operator==(const TabButton &tabButton)
{
    return this->mID == tabButton.mID;
}

void TabButton::UpdateWebSiteIcon(const QString &url)
{
}

void TabButton::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QRect rect = this->rect();
    rect.setHeight(this->size().height());
    QPixmap *bk_pixmap = mStatusMap[mPushStatus];
    // 0.画出icon
    mWebsiteIcon.paint(&painter, mWebsiteRect);
    
    // 1.画出背景
    painter.drawPixmap(rect, *bk_pixmap);
    // 2.画出文字
    if (! mTitle.isEmpty()) {
        QString draw_text = getElidedText(mFont, mTitle, mTextRectF.width());
        painter.setFont(mFont);
        painter.setPen(mPushStatus == PushStatus::Press ? mHighTextColor : mNormalTextColor);
        painter.drawText(mTextRectF, draw_text, QTextOption(Qt::AlignVCenter | Qt::AlignLeft));
    }
    // 3.画出关闭按钮
    {
        painter.drawPixmap(mBtnCloseRect, mCloseBtnIsHover ? mHoverBtnClose : mNormalBtnClose);
    }

    // 4.是否需要绘制分割线
    if (mPushStatus == PushStatus::Normal || mPushStatus == PushStatus::Leave) {
        painter.setPen(QColor(0, 0, 0, 30));
        int start_height = (rect.height()-kLINE_HEIGHT*SystemScaleInstance.GetScalePrecision())/2;
        painter.drawLine(rect.width()-1, start_height, rect.width()-1, start_height+kLINE_HEIGHT*SystemScaleInstance.GetScalePrecision());
    }
}

void TabButton::mousePressEvent(QMouseEvent *mouseEvent)
{
    mPushStatus = PushStatus::Press;
}

void TabButton::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (mBtnCloseRect.contains(mouseEvent->pos())) {
        emit OnClose(this);
    } else {
        emit OnClick(this);
    }

    QPushButton::mouseReleaseEvent(mouseEvent);
}

void TabButton::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (mBtnCloseRect.contains(mouseEvent->pos())) {
        if (! mCloseBtnIsHover) {
            mCloseBtnIsHover = true;
            update();
        }
    } else {
        if (mCloseBtnIsHover) {
            mCloseBtnIsHover = false;
            update();
        }
    }

    QPushButton::mouseMoveEvent(mouseEvent);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (PushStatus::Press == mPushStatus) { return; }

    mPushStatus         = PushStatus::Normal;
    mCloseBtnIsHover    = false;
    update();
}

void TabButton::enterEvent(QEvent *event)
{
    if (PushStatus::Press == mPushStatus || PushStatus::Enter == mPushStatus) 
    { return; }

    mPushStatus = PushStatus::Enter;
    update();
}

void TabButton::initIcon()
{
}

void TabButton::initStatsMap()
{
    /*
    if (mNormalBK.isNull() || mHoverBK.isNull() || mPushBK.isNull()) {
        throw std::logic_error("you must init QPixmap.");
    } 
    */

    mStatusMap[PushStatus::Normal] = &mNormalBK;
    mStatusMap[PushStatus::Enter] = &mHoverBK;
    mStatusMap[PushStatus::Press] = &mPushBK;
    mStatusMap[PushStatus::Leave] = &mNormalBK;
}

QString TabButton::getElidedText(const QFont &font, const QString &text, int maxWidth)
{
    QString elided_text = text;
    QFontMetrics fontWidth(font);  
    int width = fontWidth.width(text);
    if(width >= maxWidth) {
         elided_text = fontWidth.elidedText(text, Qt::ElideRight, maxWidth);
    }
    return elided_text;
}

void TabButton::OnTitleChange(const shared_ptr<wstring> title)
{
    QString temp_title = QString::fromStdWString(*title);

    if (temp_title == mTitle) { return; }
    mTitle = temp_title;

    update();
}
