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
    explicit PlayerView(const QString& hwdec, QWidget *parent = nullptr);
    ~PlayerView();

    void playMedia(const QString &media);
    void title(const QString& title);
    void referrer(const QString& referrer);
    void userAgent(const QString& userAgent);
protected:
#ifdef Q_OS_MAC
    void changeEvent(QEvent *e);
#endif
    void contextMenuEvent(QContextMenuEvent *e);
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
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
    void onSizeChanged(const QSize &sz);
    void onMaxButton(void);
    void onStopButton(void);
    void onStopped(void);
    void addAudioTrack(void);
    void selectAudioTrack(void);
    void setAudioDelay(void);
    void saveVolume(int vol);
    void setFullScreen(void);
    void showVolumeSlider(void);
    void hideElements(void);

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
