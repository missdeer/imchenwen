#ifndef MPLAYER_H
#define MPLAYER_H

#include <QOpenGLWidget>
#include <QHash>
#include <mpv/client.h>
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
using mpv_context = mpv_opengl_cb_context;
#else
#include <mpv/render_gl.h>
using mpv_context = mpv_render_context;
#endif

class PlayerCore : public QOpenGLWidget
{
    Q_OBJECT
public:
    enum State {STOPPING, VIDEO_PLAYING, VIDEO_PAUSING, TV_PLAYING} ;
    explicit PlayerCore(QWidget *parent = nullptr);
    ~PlayerCore() override;
    PlayerCore& operator=(const PlayerCore&) = delete;
    PlayerCore(const PlayerCore&) = delete;
    PlayerCore& operator=( PlayerCore&&) = delete;
    PlayerCore( PlayerCore&&) = delete;
    State state;
    QString currentFile() { return m_mediaFile; }
    int64_t getTime() { return m_time; }
    int64_t getLength() { return m_length; }
    double getAudioDelay() { return m_audioDelay; }
    const QStringList &getAudioTracksList() { return m_audioTracksList; }

    void command(const QVariant& params);
    void setOption(const QString& name, const QVariant& value);
    void setProperty(const QString& name, const QVariant& value);

    void initialize();

signals:
    void played();
    void paused();
    void stopped();
    void fullScreen();
    void timeChanged(int pos);
    void lengthChanged(int len);
    void sizeChanged(const QSize &size);

public slots:
    void stop();
    void changeState();
    void jumpTo(int pos);
    void setProgress(int pos);
    void setVolume(int volume);
    void openFile(const QString &m_mediaFile, const QString &m_audioTrack = QString());
    void openMedia(const QString &video, const QString &audio, const QString &subtitle);
    void openAudioTrack(const QString &audioFile);
    void screenShot();
    void speedUp();
    void speedDown();
    void speedSetToDefault();
    void showText(const QByteArray &text);
    void pauseRendering();
    void unpauseRendering();
    void setAid(int64_t aid);
    void setSid(int64_t sid);
    void setAudioDelay(double v);
    void setChannel_Left();
    void setChannel_Right();
    void setChannel_Stereo();
    void setChannel_Swap();
    void setRatio_16_9();
    void setRatio_16_10();
    void setRatio_4_3();
    void setRatio_0();
    void setBrightness(int64_t v);
    void setContrast(int64_t v);
    void setSaturation(int64_t v);
    void setGamma(int64_t v);
    void setHue(int64_t v);


private slots:
    void swapped();
    void maybeUpdate();
protected:
    void initializeGL() override;
    void paintGL() override;
    bool event(QEvent *e) override;

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

#endif // MPLAYER_H
