#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QPoint>
#include <QWidget>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QMenu;
class QSlider;
class QTimer;
QT_END_NAMESPACE

namespace Ui {
class PlayerView;
}
class Border;
class PlayerCore;

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = nullptr);
    ~PlayerView() override;
    PlayerView& operator=(const PlayerView&) = delete;
    PlayerView(const PlayerView&) = delete;
    PlayerView& operator=( PlayerView&&) = delete;
    PlayerView( PlayerView&&) = delete;

    void playMedia(const QString &video, const QString &audio, const QString &subtitle);
    void title(const QString& title);
    void referrer(const QString& referrer);
    void userAgent(const QString& userAgent);
protected:
#ifdef Q_OS_MAC
    void changeEvent(QEvent *e) override;
#endif
    void contextMenuEvent(QContextMenuEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

signals:
    void finished(int , QProcess::ExitStatus );
private slots:
    void onLengthChanged(int len);
    void onTimeChanged(int time);
    void onTimeSliderPressed();
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased();
    void onSizeChanged(const QSize &sz);
    void onMaxButton();
    void onStopButton();
    void onStopped();
    void addAudioTrack();
    void selectAudioTrack();
    void setAudioDelay();
    void saveVolume(int vol);
    void setFullScreen();
    void showVolumeSlider();
    void hideElements();

private:
    Ui::PlayerView *ui;
    Border *m_leftBorder;
    Border *m_rightBorder;
    Border *m_bottomBorder;
    Border *m_bottomLeftBorder;
    Border *m_bottomRightBorder;
    PlayerCore *m_playerCore;
    QMenu *m_menu;
    QSlider *m_volumeSlider;
    QTimer *m_hideTimer;
    QPoint m_dPos;
    bool m_quitRequested;
    bool m_noPlayNext;
    bool m_ctrlPressed;
};

#endif // PLAYERVIEW_H
