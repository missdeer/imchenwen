#ifndef DLNAPLAYERVIEW_H
#define DLNAPLAYERVIEW_H

#include <QPoint>
#include <QWidget>
#include <QProcess>
#include "Kast.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QSlider;
class QTimer;
QT_END_NAMESPACE

namespace Ui {
class DLNAPlayerView;
}
class Border;
class DLNARenderer;

class DLNAPlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit DLNAPlayerView(QWidget *parent = nullptr);
    ~DLNAPlayerView();

    void playMedias(const QStringList& medias);
    void title(const QString& title);
    void referrer(const QString& referrer);
    void userAgent(const QString& userAgent);
    void setRenderer(const QString &renderer);

protected:
#ifdef Q_OS_MAC
    void changeEvent(QEvent *e);
#endif
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

signals:
    void finished(int , QProcess::ExitStatus );
private slots:
    void onLengthChanged(int len);
    void onTimeChanged(int time);
    void onTimeSliderPressed(void);
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased(void);
    void onMaxButton(void);
    void onStopButton(void);
    void onStopped(void);
    void saveVolume(int vol);
    void showVolumeSlider(void);

    void onPlay();
    void onPause();
    void onResume();
    void onReceivePlaybackInfo(DLNAPlaybackInfo* info);
private:
    Ui::DLNAPlayerView *ui;
    Border *m_leftBorder;
    Border *m_rightBorder;
    Border *m_bottomBorder;
    Border *m_bottomLeftBorder;
    Border *m_bottomRightBorder;
    QSlider *m_volumeSlider;
    QTimer *m_getPositionInfoTimer;
    QPoint m_dPos;
    bool m_quitRequested;
    bool m_noPlayNext;
    bool m_ctrlPressed;
    bool m_paused;
    DLNARenderer* m_renderer;
    QString m_referrer;
    QString m_userAgent;
};

#endif // PLAYERVIEW_H
