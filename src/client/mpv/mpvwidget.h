#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QWidget>
#include <mpv/client.h>
#include <mpv/qthelper.hpp>

class MPVWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MPVWidget(QWidget *parent = nullptr);
    ~MPVWidget();
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QSize sizeHint() const override;
signals:
    void durationChanged(int value);
    void positionChanged(int value);
    void mpv_events();
public slots:
    void onMpvEvents();
private:
    void handleMpvEvent(mpv_event *event);

    mpv::qt::Handle mpv;
};

#endif // MPVWIDGET_H
