#include "mpvwidget.h"

#include <sstream>
#include <stdexcept>
#include <QJsonDocument>
#include <QtCore>

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    MPVWidget *mainwindow = (MPVWidget *)ctx;
    emit mainwindow->mpv_events();
}


MPVWidget::MPVWidget(QWidget *parent)
    : QWidget(parent)
{
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);

    int64_t wid = winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    //mpv::qt::set_option_variant(mpv, "vo", "direct3d");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv::qt::set_option_variant(mpv, "hwdec-codecs", "all");

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpv, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpv, "input-vo-keyboard", "yes");

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    mpv_request_log_messages(mpv, "info");

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &MPVWidget::mpv_events, this, &MPVWidget::onMpvEvents,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("mpv failed to initialize");
}

MPVWidget::~MPVWidget()
{
}

void MPVWidget::command(const QVariant &params)
{
    mpv::qt::command_variant(mpv, params);
}

void MPVWidget::setProperty(const QString &name, const QVariant &value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MPVWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

QSize MPVWidget::sizeHint() const
{
    return QSize(480, 270);
}

void MPVWidget::onMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleMpvEvent(event);
    }
}

void MPVWidget::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE: {
            mpv_event_property *prop = (mpv_event_property *)event->data;
            if (strcmp(prop->name, "time-pos") == 0) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    double time = *(double *)prop->data;
                } else if (prop->format == MPV_FORMAT_NONE) {
                    // The property is unavailable, which probably means playback was stopped.
                    qDebug() << "mpv is quiting?";
                }
            } else if (strcmp(prop->name, "chapter-list") == 0 ||
                       strcmp(prop->name, "track-list") == 0)
            {
                // Dump the properties as JSON for demo purposes.
                if (prop->format == MPV_FORMAT_NODE) {
#if !defined (QT_NO_DEBUG)
                    QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
                    // Abuse JSON support for easily printing the mpv_node contents.
                    QJsonDocument d = QJsonDocument::fromVariant(v);
                    qDebug() <<"Change property " + QString(prop->name) + ":\n" <<d.toJson().data();
#endif
                }
            } else if (strcmp(prop->name, "duration") == 0) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    double time = *(double *)prop->data;
                    Q_EMIT durationChanged(time);
                }
            }
            break;
        }
        case MPV_EVENT_VIDEO_RECONFIG: {
            // Retrieve the new video size.
            int64_t w, h;
            if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
                mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
                w > 0 && h > 0)
            {
                // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
                // imply a resize, and you should check yourself if the video
                // dimensions really changed.
                // mpv itself will scale/letter box the video to the container size
                // if the video doesn't fit.
                qDebug() << "Reconfig: " << w << " " << h;
            }
            break;
        }
        case MPV_EVENT_LOG_MESSAGE: {
            struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
            qDebug() << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
            break;
        }
        case MPV_EVENT_SHUTDOWN: {
            //mpv_terminate_destroy(mpv);
            break;
        }
        default: ;
            // Ignore uninteresting or unknown events.
    }
}
