#include "mpvopenglwidget.h"
#include <stdexcept>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MPVOpenGLWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

MPVOpenGLWidget::MPVOpenGLWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "warn");
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpv::qt::set_option_variant(mpv, "vo", "opengl-cb");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv::qt::set_option_variant(mpv, "hwdec-codecs", "all");

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(mpv_gl, MPVOpenGLWidget::on_update, (void *)this);
    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv::qt::set_option_variant(mpv, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv::qt::set_option_variant(mpv, "input-vo-keyboard", "yes");

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, wakeup, this);
}

MPVOpenGLWidget::~MPVOpenGLWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, nullptr, nullptr);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(mpv_gl);
}

void MPVOpenGLWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

void MPVOpenGLWidget::setOption(const QString &name, const QVariant &value)
{
    mpv::qt::set_option_variant(mpv, name, value);
}

void MPVOpenGLWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MPVOpenGLWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

QSize MPVOpenGLWidget::sizeHint() const
{
    return QSize(480, 270);
}

void MPVOpenGLWidget::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(mpv_gl, nullptr, get_proc_address, nullptr);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void MPVOpenGLWidget::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void MPVOpenGLWidget::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void MPVOpenGLWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MPVOpenGLWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        }
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MPVOpenGLWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    } else {
        update();
    }
}

void MPVOpenGLWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MPVOpenGLWidget*)ctx, "maybeUpdate");
}
