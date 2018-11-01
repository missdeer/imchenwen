#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QtWidgets/QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

class MPVOpenGLWidget Q_DECL_FINAL: public QOpenGLWidget
{
    Q_OBJECT
public:
    MPVOpenGLWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
    ~MPVOpenGLWidget() override;
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QSize sizeHint() const override;
Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
private Q_SLOTS:
    void swapped();
    void on_mpv_events();
    void maybeUpdate();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
};



#endif // PLAYERWINDOW_H
