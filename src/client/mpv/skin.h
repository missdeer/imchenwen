#ifndef SKIN_H
#define SKIN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QString;
class QPushButton;
class QPixmap;
QT_END_NAMESPACE


class Border : public QWidget
{
public:
    enum BorderType{LEFT, RIGHT, BOTTOM, BOTTOMLEFT, BOTTOMRIGHT};
    Border(QWidget* topwin, BorderType type);
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
private:
    BorderType type;
    QWidget* topwindow;
    QPoint oldPos;
};

#endif // SKIN_H
