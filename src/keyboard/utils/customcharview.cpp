#include "customcharview.h"
#include <QEvent>
#include <QMouseEvent>

CustomCharView::CustomCharView(QWidget *parent) : QWidget(parent)
{
    unitMinHeight = -1;
    unitMinWidth = -1;
}
int CustomCharView::getUnitMinWidth() const
{
    return unitMinWidth;
}

void CustomCharView::setUnitMinWidth(int value)
{
    unitMinWidth = value;
}
int CustomCharView::getUnitMinHeight() const
{
    return unitMinHeight;
}

void CustomCharView::setUnitMinHeight(int value)
{
    unitMinHeight = value;
}

bool CustomCharView::event(QEvent *e)
{
    static bool pressed = false;
    static QPoint pressedPoint;
    if(e->type() == QEvent::MouseButtonPress)
    {
        pressed = true;
        pressRect = QRect();
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(e);
        pressedPoint = mouse_event->globalPos();
        int index = findcontansMouseRect(mouse_event->pos(), pressRect);
        if(index != -1)
        {
            emit stringPressed(dataStrings.at(index),
                               mapToGlobal(QPoint(pressRect.x(), pressRect.y())));
            update(pressRect);
        }
        return true;
    }
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        if(pressed)
        {
            if(!pressRect.isEmpty())
            {
                int index = dataRects.indexOf(pressRect);
                emit clicked(dataStrings.at(index), index);
            }
            update(pressRect);
            emit stringPressed("", QPoint());
        }
        pressed = false;
        pressRect = QRect();
        return true;
    }
    //鼠标左键按下的同时移动，如果移动到了另外的符号框内，就将对应的符号进行同样的操作
    else if(pressed && e->type() == QEvent::MouseMove)
    {
        if(pressed)
        {
            QRect cur_rect;
            QMouseEvent *mouse_event = static_cast<QMouseEvent *>(e);
            int index = findcontansMouseRect(mouse_event->pos(), cur_rect);
            if(index == -1)
            {
                emit(stringPressed("", QPoint()));
                pressed = false;
                update(pressRect);
                pressRect = QRect();

            }
        }
        return true;
    }
    return QWidget::event(e);
}

int CustomCharView::findcontansMouseRect(const QPoint &mousePos, QRect &rect)
{
    for (int i = 0; i < dataRects.size(); i++)
    {
        if (dataRects.at(i).contains(mousePos))
        {
            rect = dataRects.at(i);
            return i;
        }
    }

    return -1;
}
