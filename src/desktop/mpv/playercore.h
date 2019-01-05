#ifndef MPLAYER_H
#define MPLAYER_H

#include <QOpenGLWidget>
#include <QHash>
#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

QT_BEGIN_NAMESPACE
class QTimer;
class QProcess;
class QResizeEvent;
class QLabel;
QT_END_NAMESPACE

#if MPV_CLIENT_API_VERSION < MPV_MAKE_VERSION(1, 28)
#define USE_OPENGL_CB
#endif

#ifdef USE_OPENGL_CB
#include <mpv/opengl_cb.h>
typedef mpv_opengl_cb_context mpv_context;
#else
#include <mpv/render_gl.h>
typedef mpv_render_context mpv_context;
#endif

class PlayerCore : public QOpenGLWidget
{
    Q_OBJECT
public:
    typedef enum {STOPPING, VIDEO_PLAYING, VIDEO_PAUSING, TV_PLAYING} State;
    explicit PlayerCore(QWidget *parent = nullptr);
    virtual ~PlayerCore();
    State state;
    QString currentFile() { return m_mediaFile; }
    int64_t getTime() { return m_time; }
    int64_t getLength() { return m_length; }
    double getAudioDelay() { return m_audioDelay; }
    const QStringList &getAudioTracksList() { return m_audioTracksList; }

    void command(const QVariant& params);
    void setOption(const QString& name, const QVariant& value);
    void setProperty(const QString& name, const QVariant& value);

signals:
    void played(void);
    void paused(void);
    void stopped(void);
    void fullScreen(void);
    void timeChanged(int pos);
    void lengthChanged(int len);
    void sizeChanged(const QSize &size);

public slots:
    void stop(void);
    void changeState(void);
    void jumpTo(int pos);
    void setProgress(int pos);
    void setVolume(int volume);
    void openFile(const QString &m_mediaFile, const QString &m_audioTrack = QString());
    void openMedia(const QString &video, const QString &audio);
    void openAudioTrack(const QString &audioFile);
    void screenShot(void);
    void speedUp(void);
    void speedDown(void);
    void speedSetToDefault(void);
    void showText(const QByteArray &text);
    void pauseRendering(void);
    void unpauseRendering(void);
    void setAid(int64_t aid);
    void setSid(int64_t sid);
    void setAudioDelay(double v);
    void setChannel_Left(void);
    void setChannel_Right(void);
    void setChannel_Stereo(void);
    void setChannel_Swap(void);
    void setRatio_16_9(void);
    void setRatio_16_10(void);
    void setRatio_4_3(void);
    void setRatio_0(void);
    void setBrightness(int64_t v);
    void setContrast(int64_t v);
    void setSaturation(int64_t v);
    void setGamma(int64_t v);
    void setHue(int64_t v);


private slots:
    void swapped(void);
    void maybeUpdate();
protected:
    void initializeGL();
    void paintGL();
    bool event(QEvent *e);

private:
    mpv::qt::Handle m_mpv;
    mpv_context  *m_mpvGL;
    QString m_mediaFile;
    QString m_audioTrack;
    QStringList m_audioTracksList;
    QStringList m_subtitleList;
    int64_t m_length;
    int64_t m_time;
    int64_t m_videoWidth;
    int64_t m_videoHeight;
    double m_playSpeed;
    double m_audioDelay;
    bool m_noEmitStopped;
    bool m_reloadWhenIdle;
    bool m_emitStoppedWhenIdle;
    bool m_unseekableForced;
    bool m_renderingPaused;

    void handleMpvError(int code);
    static void on_update(void *ctx);
};

extern PlayerCore *g_playerCore;

#endif // MPLAYER_H
