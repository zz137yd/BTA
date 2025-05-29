#ifndef UTILS_H
#define UTILS_H

// 1.Only double-clicking the left button will work
#include <QTreeView>
#include <QMouseEvent>

// 2.Center, left and right alignment
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

// 3.Style of copy-path button
#include <QPushButton>

// 4.Favorites drop-down box style
#include <QComboBox>

// 5.Image preview adaptation
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

// 6.Mouse partial event interception
#include <QEvent>

// 7.Display content line spacing


// 1.Only double-clicking the left button will work
class MyLeftView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyLeftView(QWidget* parent = nullptr) : QTreeView(parent) {}

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) QTreeView::mouseDoubleClickEvent(event);
        //else event->ignore();
    }
};

// 2. Center alignment
class CenterAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignCenter;
    }
};

// 2. Left alignment
class LeftAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignLeft;
    }
};

// 2. Right alignment
class RightAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignRight;
    }
};

// 3.Style of copy-path button
inline void buttonQss(QPushButton* button)
{
    button->setStyleSheet(
        // Default
        "QPushButton {"
        "   background-color: #ffffff;"         // Transparent background
        "   color: #424242;"                    // Dark gray text
        "   border: 1px solid rgba(0,0,0,0.1);" // Very thin light gray border
        "   border-radius: 6px;"                // Round Corners
        "   padding: 8px 12px;"                 // Padding
        "}"

        // Mouse hover state (slightly lit)
        "QPushButton:hover {"
        "   background-color: rgba(0,0,0,0.03);" // Very light grey translucent
        "   border-color: rgba(0,0,0,0.15);"     // Slightly darken the border
        "}"

        // Mouse down state (minimal click effect)
        "QPushButton:pressed {"
        "   background-color: rgba(0,0,0,0.05);" // Slightly darker grey than hover
        "   border: 1px solid transparent;"      // Remove border
        "   padding-top: 9px;"                   // Slight sinking effect
        "   padding-bottom: 7px;"
        "}"
    );
}

// Dark mode
inline void buttonQssDark(QPushButton* button)
{
    button->setStyleSheet(
        "QPushButton {"
        "   background-color: #3c4b4f;"
        "   color: #d0e0dc;"
        "   border: 1px solid #5a6e6a;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4c5f63;"
        "   border-color: #7a8e8a;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #2e3b3f;"
        "   border: 1px solid transparent;"
        "   padding-top: 9px;"
        "   padding-bottom: 7px;"
        "}"
    );
}

// 4.Favorites drop-down box style
inline void comboboxQss(QComboBox* box)
{
    box->setStyleSheet(R"(
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 11pt;
            color: #424242;
            selection-background-color: #2196F3;
            selection-color: white;
        }
        QComboBox:hover {
            border: 1px solid transparent;
            background-color: rgba(33, 150, 243, 0.1);
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
        }
        QComboBox:focus {
            border-color: #2196F3;
            outline: none;
            box-shadow: 0 0 8px rgba(33, 150, 243, 0.4);
        }
        QComboBox::drop-down {
            background-color: transparent;
            width: 30px;
        }
        QComboBox QAbstractItemView {
            border: none;
            background-color: #ffffff;
            selection-background-color: #bbdefb;
            selection-color: #424242;
            outline: none;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.15);
        }
        QComboBox QAbstractItemView::item {
            padding: 12px 20px;
            color: #424242;
            font-weight: 400;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #f5f5f5;
        }
        QComboBox QAbstractItemView::separator {
            height: 1px;
            background-color: #e0e0e0;
            margin: 8px 0;
        }
        QComboBox::down-arrow {
            image: url(:/up.png);
            width: 17px;
            height: 17px;
            margin-right: 30px;
            margin-top: 2px;
        }
        QComboBox::down-arrow:on {
            image: url(:/down.png);
        }
                       )");
}

// 5.Image preview adaptation
class ScalableImageLabel : public QLabel
{
public:
    explicit ScalableImageLabel(QWidget* parent = nullptr)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);
    }

    // Override the setPixmap method (without writing override)
    void setPixmap(const QPixmap& pix)
    {
        originalPixmap = pix;
        updateScaledPixmap();
    }

protected:
    // Recalculate the scale every time the control changes size
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        updateScaledPixmap();
    }

private:
    QPixmap originalPixmap;

    void updateScaledPixmap()
    {
        if (!originalPixmap.isNull())
            QLabel::setPixmap(originalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
};

// 6.Mouse partial event interception
class NoClickFilter : public QObject
{
    Q_OBJECT
public:
    explicit NoClickFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        // Only intercept mouse related events
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease)
        {
            return true; // Shield
        }
        return QObject::eventFilter(watched, event);
    }
};

class RightClickDoubleClickFilter : public QObject
{
    Q_OBJECT
public:
    explicit RightClickDoubleClickFilter(QObject* parent = nullptr)
      : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* ev) override
    {
        // If it is a double-click event of the mouse, and it is the right button, it will be blocked
        if (ev->type() == QEvent::MouseButtonDblClick)
        {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::RightButton) return true;
        }
        // All other events are allowed
        return QObject::eventFilter(watched, ev);
    }
};

// Display content line spacing
class SpacingDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SpacingDelegate(int extraHeight = 7, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), m_extra(extraHeight) {}

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& idx) const override
    {
        QSize s = QStyledItemDelegate::sizeHint(opt, idx);
        return QSize(s.width(), s.height() + m_extra);
    }

private:
    int m_extra;
};

#endif // UTILS_H
