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

class PlayerCore : public QOpenGLWidget
{
    Q_OBJECT

signals:
    void cutVideo(void);
    void played(void);
    void paused(void);
    void stopped(void);
    void fullScreen(void);
    void timeChanged(int pos);
    void lengthChanged(int len);
    void sizeChanged(const QSize &size);

public:
    typedef enum {STOPPING, VIDEO_PLAYING, VIDEO_PAUSING, TV_PLAYING} State;
    explicit PlayerCore(QWidget *parent = nullptr);
    virtual ~PlayerCore();
    State state;
    inline QString currentFile() { return file; }
    inline int getTime() { return time; }
    inline int getLength() { return length; }
    inline double getAudioDelay() { return audioDelay; }
    inline const QStringList &getAudioTracksList() { return audioTracksList; }

    void command(const QVariant& params);
    void setOption(const QString& name, const QVariant& value);
    void setProperty(const QString& name, const QVariant& value);
public slots:
    void stop(void);
    void changeState(void);
    void jumpTo(int pos);
    void setProgress(int pos);
    void setVolume(int volume);
    void openFile(const QString &file, const QString &audioTrack = QString());
    void openMedias(const QStringList& files);
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

protected:
    void initializeGL();
    void paintGL();
    bool event(QEvent *e);

private:
    mpv::qt::Handle m_mpv;
    mpv_opengl_cb_context *mpv_gl;
    QString file;
    QString audioTrack;
    QStringList audioTracksList;
    QStringList subtitleList;
    int64_t length;
    int64_t time;
    int64_t videoWidth;
    int64_t videoHeight;
    double speed;
    double audioDelay;
    bool no_emit_stopped;
    bool reload_when_idle;
    bool emit_stopped_when_idle;
    bool unseekable_forced;
    bool rendering_paused;

    void handleMpvError(int code);
    static void on_update(void *ctx);

private slots:
    void swapped(void);
    void maybeUpdate();
};

extern PlayerCore *player_core;

#endif // MPLAYER_H
