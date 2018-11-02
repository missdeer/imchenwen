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
class CutterBar;
class PlayerCore;

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = nullptr);
    ~PlayerView();

    void playMedias(const QStringList& medias);
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
    void showCutterBar(void);
    void showVolumeSlider(void);
    void hideElements(void);

private:
    Ui::PlayerView *ui;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
    Border *bottomLeftBorder;
    Border *bottomRightBorder;
    CutterBar *cutterBar;
    PlayerCore *core;
    QMenu *menu;
    QSlider *volumeSlider;
    QTimer *hideTimer;
    QPoint dPos;
    bool quit_requested;
    bool no_play_next;
    bool ctrl_pressed;
};

#endif // PLAYERVIEW_H
