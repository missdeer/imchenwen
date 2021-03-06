#include "playercore.h"
#include "config.h"
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QHash>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QSysInfo>
#include <QtCore>

namespace {
// wayland fix
#ifdef Q_OS_LINUX
#include <QGuiApplication>
#include <QX11Info>
#include <qpa/qplatformnativeinterface.h>
static void *GLAPIENTRY glMPGetNativeDisplay(const char *name)
{
    if (strcmp(name, "wl") == 0 && !QX11Info::isPlatformX11())
        return QGuiApplication::platformNativeInterface()->nativeResourceForWindow("display", NULL);
    else if (strcmp(name, "x11") == 0 && QX11Info::isPlatformX11())
        return QX11Info::display();
    return NULL;
}
#endif

void wakeup(void *ptr)
{
    auto *core = static_cast<PlayerCore*>(ptr);
    QCoreApplication::postEvent(core, new QEvent(QEvent::User));
}

void *get_proc_address(void *, const char *name)
{
    // hardware acceleration fix
#ifdef Q_OS_LINUX
    if(strcmp(name, "glMPGetNativeDisplay") == 0)
        return (void*) glMPGetNativeDisplay;
#endif

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;

    void *res = (void *)glctx->getProcAddress(QByteArray(name));
#ifdef Q_OS_WIN32
    // wglGetProcAddress(), which is used by Qt, does not always resolve all
    // builtin functions with all drivers (only extensions). Qt compensates this
    // for a degree, but does this only for functions Qt happens to need. So
    // we need our own falback as well.
    if (!res)
    {
        HMODULE handle = (HMODULE)QOpenGLContext::openGLModuleHandle();
        if (handle)
            res = (void *)GetProcAddress(handle, name);
    }
#endif
    return res;
}
}

void PlayerCore::initialize()
{
    m_mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!m_mpv)
    {
        qDebug("Cannot create mpv instance.");
    }
    // initialize mpv
    if (mpv_initialize(m_mpv) < 0)
    {
        qDebug("Cannot initialize mpv.");
    }

    // set mpv options
    mpv::qt::set_option_variant(m_mpv, "softvol", "yes");         // mpv handles the volume
    mpv::qt::set_option_variant(m_mpv, "ytdl", "no");             // We handle video url parsing
    mpv::qt::set_option_variant(m_mpv, "osc", "no");
    mpv::qt::set_option_variant(m_mpv, "avsync", 0);
    mpv::qt::set_option_variant(m_mpv, "cache-secs", 60);
    mpv::qt::set_option_variant(m_mpv, "cache-default", 8196);
    mpv::qt::set_option_variant(m_mpv, "prefetch-playlist", "yes");
    mpv::qt::set_option_variant(m_mpv, "merge-files", "yes");
    mpv::qt::set_option_variant(m_mpv, "screenshot-directory", QDir::toNativeSeparators( QDir::homePath()));
    mpv::qt::set_option_variant(m_mpv, "reset-on-next-file", "speed,video-aspect,af,sub-delay,sub-visibility,audio-delay");
    mpv_request_log_messages(m_mpv, "info");

    // set hardware decoding
#if defined(Q_OS_LINUX)
    Config cfg;
    if (cfg.read<QString>("hwdec", "auto") == "auto")
    {
        mpv::qt::set_option_variant(m_mpv, "hwdec-preload", "auto");
    }
    else if (cfg.read<QString>("hwdec", "auto") == "vaapi")
    {
        mpv::qt::set_option_variant(m_mpv, "hwdec-preload", "vaapi-egl");
    }
    else
    {
        mpv::qt::set_option_variant(m_mpv, "hwdec-preload", "vdpau-glx");
    }
#endif
    Config cfg;
    QString hwdec = cfg.read<QString>(QLatin1String("builtinPlayerHWAccel"));
    if (!hwdec.isEmpty())
        mpv::qt::set_option_variant(m_mpv, "hwdec", hwdec);

#ifdef USE_OPENGL_CB
    mpv::qt::set_option_variant(m_mpv, "vo", "opengl-cb");
#else
    mpv::qt::set_option_variant(m_mpv, "vo", "libmpv");
#endif
    mpv::qt::set_option_variant(m_mpv, "hwdec-codecs", "all");

    // listen mpv event
    mpv_observe_property(m_mpv, 0, "duration",         MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "width",            MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "height",           MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "playback-time",    MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "core-idle",        MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "track-list",       MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "sid",              MPV_FORMAT_INT64);
    mpv_set_wakeup_callback(m_mpv, wakeup, this);

#ifdef USE_OPENGL_CB
    // initialize opengl
    m_mpvGL = (mpv_opengl_cb_context*) mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
    if (!m_mpvGL)
    {
        qDebug("OpenGL not compiled in");
    }
    mpv_opengl_cb_set_update_callback(m_mpvGL, PlayerCore::on_update, (void*) this);
    connect(this, &PlayerCore::frameSwapped, this, &PlayerCore::swapped);
#else
    QWidget *nativeParent = nativeParentWidget();
    if (nativeParent == nullptr) {
        qFatal("glwidget, no native parent handle");
    }
    mpv_opengl_init_params gl_init_params{ get_proc_address, this, nullptr };
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if (mpv_render_context_create(&m_mpvGL, m_mpv, params) < 0)
        qDebug("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(m_mpvGL, PlayerCore::on_update, static_cast<void *>(this));
#endif

    // set state
    state = STOPPING;
    m_noEmitStopped = false;
    m_reloadWhenIdle = false;
    m_emitStoppedWhenIdle = false;
    m_unseekableForced = false;
    m_renderingPaused = false;
}

PlayerCore::PlayerCore(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_mpvGL(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
}

// opengl
void PlayerCore::initializeGL()
{
    qDebug() << "OpenGL Version: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    initialize();
#ifdef USE_OPENGL_CB
#ifdef Q_OS_LINUX
    int r = mpv_opengl_cb_init_gl(m_mpvGL, "GL_MP_MPGetNativeDisplay", get_proc_address, nullptr);
#else
    int r = mpv_opengl_cb_init_gl(m_mpvGL, nullptr, get_proc_address, nullptr);
#endif
    if (r < 0)
    {
        qDebug("Cannot initialize OpenGL.");
    }
#endif
}

void PlayerCore::paintGL()
{
#ifdef USE_OPENGL_CB
    mpv_opengl_cb_draw(m_mpvGL, defaultFramebufferObject(), width() * devicePixelRatioF(), -height() * devicePixelRatioF());
#else
    mpv_opengl_fbo mpfbo {
        static_cast<int>(defaultFramebufferObject()),
                width() * devicePixelRatio(),
                height() * devicePixelRatio(),
                0
    };
    int flip_y = 1;

    mpv_render_param params[] = {
        // Specify the default framebuffer (0) as target. This will
        // render onto the entire screen. If you want to show the video
        // in a smaller rectangle or apply fancy transformations, you'll
        // need to render into a separate FBO and draw it manually.
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        // Flip rendering (needed due to flipped GL coordinate system).
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(m_mpvGL, params);
#endif
}

void PlayerCore::swapped()
{
#ifdef USE_OPENGL_CB
    mpv_opengl_cb_report_flip(m_mpvGL, 0);
#endif
}

void PlayerCore::maybeUpdate()
{
    if (window()->isMinimized() || m_renderingPaused)
    {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    }
    else
        update();
}

void PlayerCore::on_update(void *ctx)
{
    QMetaObject::invokeMethod(static_cast<PlayerCore*>(ctx), "maybeUpdate", Qt::QueuedConnection);
}

void PlayerCore::pauseRendering()
{
    m_renderingPaused = true;
}

void PlayerCore::unpauseRendering()
{
    m_renderingPaused = false;
}

PlayerCore::~PlayerCore()
{
    makeCurrent();

#ifdef USE_OPENGL_CB
    mpv_opengl_cb_set_update_callback(m_mpvGL, nullptr, nullptr);
    mpv_opengl_cb_uninit_gl(m_mpvGL);
#else
    mpv_render_context_set_update_callback(m_mpvGL, nullptr, nullptr);
    mpv_render_context_free(m_mpvGL);
#endif
    m_mpvGL = nullptr;
}

void PlayerCore::command(const QVariant &params)
{
    mpv::qt::command_variant(m_mpv, params);
}

void PlayerCore::setOption(const QString &name, const QVariant &value)
{
    mpv::qt::set_option_variant(m_mpv, name, value);
}

void PlayerCore::setProperty(const QString &name, const QVariant &value)
{
    mpv::qt::set_property_variant(m_mpv, name, value);
}

// handle event
bool PlayerCore::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return QWidget::event(e);

    while (m_mpv)
    {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event == nullptr)
            break;
        if (event->event_id == MPV_EVENT_NONE)
            break;

        handleMpvError(event->error);

        switch (event->event_id)
        {
        case MPV_EVENT_START_FILE:
            m_videoWidth = m_videoHeight = 0;
            m_time = 0;
            emit timeChanged(m_time);
            break;

        case MPV_EVENT_FILE_LOADED:
        {
            int f = 0;
            handleMpvError(mpv_set_property_async(m_mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
            state = VIDEO_PLAYING;
            emit played();
        }
            break;
        case MPV_EVENT_UNPAUSE:
            state = VIDEO_PLAYING;
            emit played();
            break;

        case MPV_EVENT_PAUSE:
            state = VIDEO_PAUSING;
            emit paused();
            break;

        case MPV_EVENT_END_FILE:
        {
            mpv_event_end_file *ef = static_cast<mpv_event_end_file*>(event->data);
            if (ef->error == MPV_ERROR_LOADING_FAILED)
            {
                m_reloadWhenIdle =  (QMessageBox::question(this,
                                                           tr("MPV Error"),
                                                           tr("Fails to load: ") + m_mediaFile,
                                                           tr("Skip"),
                                                           tr("Try again")) == 1);
                if (m_reloadWhenIdle)
                {
                    state = STOPPING;
                    break;
                }
            }
            else
                handleMpvError(ef->error);
            if (m_noEmitStopped)  // switch to new file when playing
                m_noEmitStopped = false;
            else
            {
                state = STOPPING;
                m_emitStoppedWhenIdle = true;
            }
            break;
        }
        case MPV_EVENT_IDLE:
            if (m_reloadWhenIdle)
            {
                m_reloadWhenIdle = false;
                openFile(m_mediaFile);
            }
            else if (m_emitStoppedWhenIdle)
            {
                m_emitStoppedWhenIdle = false;
                emit stopped();
            }
            break;

        case MPV_EVENT_LOG_MESSAGE:
        {
            mpv_event_log_message *msg = static_cast<mpv_event_log_message*>(event->data);
            fprintf(stderr, "[%s] %s", msg->prefix, msg->text);
            break;
        }
        case MPV_EVENT_PROPERTY_CHANGE:
        {
            mpv_event_property *prop = static_cast<mpv_event_property*>(event->data);
            if (prop->data == nullptr)
                break;
            QByteArray propName = prop->name;
            if (propName == "playback-time")
            {
                int newTime = *(double*) prop->data;
                if (newTime != m_time)
                {
                    m_time = newTime;
                    emit timeChanged(m_time);
                }
            }
            else if (propName == "duration")
            {
                m_length = *(double*) prop->data;
                emit lengthChanged(m_length);
            }
            else if (propName == "width")
            {
                m_videoWidth = *(int64_t*) prop->data;
                if (m_videoWidth && m_videoHeight)
                {
                    emit sizeChanged(QSize(m_videoWidth, m_videoHeight));
                    if (!m_audioTrack.isEmpty())
                        openAudioTrack(m_audioTrack);
                }
            }
            else if (propName == "height")
            {
                m_videoHeight = *(int64_t*) prop->data;
                if (m_videoWidth && m_videoHeight)
                {
                    emit sizeChanged(QSize(m_videoWidth, m_videoHeight));
                    if (!m_audioTrack.isEmpty())
                        openAudioTrack(m_audioTrack);
                }
            }
            else if (propName == "paused-for-cache")
            {
                if (prop->format == MPV_FORMAT_FLAG)
                {
                    if ((bool)*(unsigned*)prop->data && state != STOPPING)
                        showText("Network is slow...");
                    else
                        showText("");
                }
            }
            else if (propName == "core-idle")
            {
                if(prop->format == MPV_FORMAT_FLAG)
                {
                    if( *(unsigned*)prop->data && state == VIDEO_PLAYING)
                        showText("Buffering...");
                    else
                        showText("");
                }
            }
            else if (propName == "sid") // set danmaku's delay
            {
                int sid = *(int64_t *) prop->data;
            }
            else if (propName == "track-list") // read tracks info
            {
                m_audioTracksList.clear();
                m_subtitleList.clear();
                mpv_node *node = (mpv_node *) prop->data;
                for (int i = 0; i < node->u.list->num; i++)
                {
                    mpv_node_list *item = node->u.list->values[i].u.list;
                    QByteArray type;
                    int id = 0;
                    QString title;
                    for (int n = 0; n < item->num; n++)
                    {
                        if (!strcmp(item->keys[n], "type"))
                            type = item->values[n].u.string;
                        else if (!strcmp(item->keys[n], "id"))
                            id = item->values[n].u.int64;
                        else if (!strcmp(item->keys[n], "title"))
                            title = QString::fromUtf8(item->values[n].u.string);
                    }
                    // subtitles
                    if (type == "sub")
                    {
                        if (m_subtitleList.size() <= id)
                        {
                            for (int j = m_subtitleList.size(); j < id; j++)
                                m_subtitleList.append('#' + QString::number(j));
                            m_subtitleList.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                            m_subtitleList[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                    }
                    // audio tracks
                    if (type == "audio")
                    {
                        if (m_audioTracksList.size() <= id)
                        {
                            for (int j = m_audioTracksList.size(); j < id; j++)
                                m_audioTracksList.append('#' + QString::number(j));
                            m_audioTracksList.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                            m_audioTracksList[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                    }
                }
            }
            break;
        }
        default: break;
        }
    }
    return true;
}

// open file
void PlayerCore::openFile(const QString &file, const QString &audioTrack)
{    
    if (state != STOPPING)
    {
        m_noEmitStopped = true;
    }
    m_mediaFile = file;
    m_audioTrack = audioTrack;

    m_playSpeed = 1.0;

    QByteArray tmp = file.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), nullptr};
    handleMpvError(mpv_command_async(m_mpv, 2, args));
}

void PlayerCore::openMedia(const QString &video, const QString &audio, const QString &subtitle)
{
    if (!QUrl(video).isValid())
        return;
    if (state != STOPPING)
    {
        m_noEmitStopped = true;
    }

    m_playSpeed = 1.0;
    m_mediaFile = video;
    mpv::qt::command_variant(m_mpv, QStringList() << "loadfile" << video);
    if (QUrl(subtitle).isValid())
    {
        // workaround force to show subtitle, add a delay
        QTimer::singleShot(200, [subtitle, this](){
            QByteArray tmp = subtitle.toUtf8();
            const char *args[] = {"sub-add", tmp.constData(),"select", nullptr};
            handleMpvError(mpv_command_async(m_mpv, 2, args));
        });

#if defined(Q_OS_WIN)
        mpv::qt::set_property_variant(m_mpv, "sub-font", "Microsoft YaHei");
#elif defined(Q_OS_MAC)
        mpv::qt::set_property_variant(m_mpv, "sub-font", "Pingfang CS");
#else
#endif
    }

    if (QUrl(audio).isValid())
    {
        m_audioTrack = audio;
        mpv::qt::set_property_variant(m_mpv, "audio-file", audio);
    }
}

// switch between play and pause
void PlayerCore::changeState()
{
    int f;
    switch (state)
    {
    case VIDEO_PAUSING:
        f = 0;
        handleMpvError(mpv_set_property_async(m_mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
        break;
    case VIDEO_PLAYING:
        f = 1;
        handleMpvError(mpv_set_property_async(m_mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
        break;
    default: break;
    }
}

void PlayerCore::stop()
{
    if (state == STOPPING)
        return;
    const char *args[] = {"stop", nullptr};
    handleMpvError(mpv_command_async(m_mpv, 0, args));
}

void PlayerCore::setVolume(int volume)
{
    double vol = volume * 10.0;
    if (state == STOPPING)
        mpv_set_option(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    else
    {
        mpv_set_property_async(m_mpv, 2, "volume", MPV_FORMAT_DOUBLE, &vol);
        showText("Volume: " + QByteArray::number(vol));
    }
}

// set sid and subtitle delay
void PlayerCore::setSid(int64_t sid)
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_async(m_mpv, 0, "sid", MPV_FORMAT_INT64, &sid));
}

// set brightness, constrast, saturation, gamma and hue
void PlayerCore::setBrightness(int64_t v)
{
    v *= 10;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "brightness", MPV_FORMAT_INT64, &v));
}

void PlayerCore::setContrast(int64_t v)
{
    v *= 10;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "contrast", MPV_FORMAT_INT64, &v));
}

void PlayerCore::setSaturation(int64_t v)
{
    v *= 10;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "saturation", MPV_FORMAT_INT64, &v));
}

void PlayerCore::setGamma(int64_t v)
{
    v *= 10;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "gamma", MPV_FORMAT_INT64, &v));
}

void PlayerCore::setHue(int64_t v)
{
    v *= 10;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "hue", MPV_FORMAT_INT64, &v));
}

// set progress
void PlayerCore::setProgress(int pos)
{
    if (state == STOPPING)
        return;
    if (pos != m_time)
    {
        QByteArray tmp = QByteArray::number(pos);
        const char *args[] = {"seek", tmp.constData(), "absolute", nullptr};
        mpv_command_async(m_mpv, 2, args);
    }
}

void PlayerCore::jumpTo(int pos)
{
    if (state == STOPPING)
        return;
    if (state == VIDEO_PLAYING)
        changeState();
    setProgress(pos);
}

void PlayerCore::screenShot()
{
    if (state == STOPPING)
        return;
    const char *args[] = {"osd-msg" ,"screenshot", nullptr};
    mpv_command_async(m_mpv, 2, args);
}

// set playback speed
void PlayerCore::speedDown()
{
    if (m_playSpeed > 0.5 && state != STOPPING)
    {
        m_playSpeed -= 0.1;
        mpv_set_property_async(m_mpv, 2, "speed", MPV_FORMAT_DOUBLE, &m_playSpeed);
        showText("Speed: " + QByteArray::number(m_playSpeed));
    }
}

void PlayerCore::speedUp()
{
    if (m_playSpeed < 2.0 && state != STOPPING)
    {
        m_playSpeed += 0.1;
        mpv_set_property_async(m_mpv, 2, "speed", MPV_FORMAT_DOUBLE, &m_playSpeed);
        showText("Speed: " + QByteArray::number(m_playSpeed));
    }
}

void PlayerCore::speedSetToDefault()
{
    if (state != STOPPING)
    {
        m_playSpeed = 1;
        mpv_set_property_async(m_mpv, 2, "speed", MPV_FORMAT_DOUBLE, &m_playSpeed);
        showText("Speed: 1");
    }
}

// set aid and audio delay
void PlayerCore::setAid(int64_t sid)
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_async(m_mpv, 0, "aid", MPV_FORMAT_INT64, &sid));
}

void PlayerCore::setAudioDelay(double v)
{
    if (state == STOPPING)
        return;
    m_audioDelay = v;
    handleMpvError(mpv_set_property_async(m_mpv, 2, "audio-delay", MPV_FORMAT_DOUBLE, &v));
    showText("Audio delay: " + QByteArray::number(v));
}

void PlayerCore::openAudioTrack(const QString &audioFile)
{
    if (state == STOPPING)
        return;
    QByteArray tmp = audioFile.toUtf8();
    const char *args[] = {"audio-add", tmp.constData(), "select", nullptr};
    handleMpvError(mpv_command_async(m_mpv, 2, args));
}

// set audio channel
void PlayerCore::setChannel_Left()
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_string(m_mpv, "af", "channels=2:[0-0,0-1]"));
    showText("Left channel");
}

void PlayerCore::setChannel_Right()
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_string(m_mpv, "af", "channels=2:[1-0,1-1]"));
    showText("Right channel");
}

void PlayerCore::setChannel_Stereo()
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_string(m_mpv, "af", ""));
    showText("Stereo");
}

void PlayerCore::setChannel_Swap()
{
    if (state == STOPPING)
        return;
    handleMpvError(mpv_set_property_string(m_mpv, "af", "channels=2:[0-1,1-0]"));
    showText("Swap channel");
}

// set video aspect
void PlayerCore::setRatio_0()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(m_mpv, "video-aspect", "0");
}

void PlayerCore::setRatio_4_3()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(m_mpv, "video-aspect", "4:3");
}

void PlayerCore::setRatio_16_9()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(m_mpv, "video-aspect", "16:9");
}

void PlayerCore::setRatio_16_10()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(m_mpv, "video-aspect", "16:10");
}

// handle error
void PlayerCore::handleMpvError(int code)
{
    if(code < 0)
    {
        qDebug() << tr("Error while playing file:\n")
                 << m_mediaFile
                 << m_audioTrack
                 << mpv_error_string(code);
    }
}

// show text
void PlayerCore::showText(const QByteArray &text)
{
    if (state == STOPPING)
        return;
    const char *args[] = {"show-text", text.constData(), nullptr};
    mpv_command_async(m_mpv, 2, args);
}
