#ifndef MainWindow_H
#define MainWindow_H

#include <QtWidgets/QWidget>

class MPVOpenGLWidget;
class QSlider;
class QPushButton;
class MPVWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MPVWindow(QWidget *parent = nullptr);
    void playMedias(const QStringList& medias);
    void closeEvent(QCloseEvent *event) override;
public Q_SLOTS:
    void seek(int pos);
private Q_SLOTS:
    void setSliderRange(int duration);
private:
    MPVOpenGLWidget *m_mpv;
    QSlider *m_slider;
};

#endif // MainWindow_H
