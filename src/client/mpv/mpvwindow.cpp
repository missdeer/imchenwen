#include "mpvwindow.h"
#include "mpvwidget.h"
#include "mpvopenglwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QCloseEvent>

MPVWindow::MPVWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("imchenwen builtin media player"));
    m_mpv = new MPVOpenGLWidget(this);
    //m_mpv = new MPVWidget(this);
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(m_mpv);
    vl->addWidget(m_slider);
    setLayout(vl);
    vl->setMargin(0);
    connect(m_slider, SIGNAL(sliderMoved(int)), SLOT(seek(int)));
    connect(m_mpv, SIGNAL(positionChanged(int)), m_slider, SLOT(setValue(int)));
    connect(m_mpv, SIGNAL(durationChanged(int)), this, SLOT(setSliderRange(int)));
}

void MPVWindow::playMedias(const QStringList &medias)
{
    if (medias.isEmpty())
        return;
    m_mpv->setProperty("cache-secs", 600);
    m_mpv->command(QStringList() << "loadfile" << medias[0] << "replace");
    for (int i = 1; i < medias.length(); ++i) {
        m_mpv->command(QStringList() << "loadfile" << medias[i] << "append");
    }
}

void MPVWindow::closeEvent(QCloseEvent *event)
{
    m_mpv->command(QStringList() << "stop");
    delete m_mpv;
    event->accept();
    Q_EMIT finished(0, QProcess::NormalExit);
}

void MPVWindow::title(const QString &title)
{
    m_mpv->setOption("title", title);
    m_mpv->setOption("media-title", title);
}

void MPVWindow::referrer(const QString &referrer)
{
    m_mpv->setOption("referrer", referrer);
}

void MPVWindow::userAgent(const QString &userAgent)
{
    m_mpv->setOption("user-agent", userAgent);
}

void MPVWindow::seek(int pos)
{
    m_mpv->command(QVariantList() << "seek" << pos << "absolute");
}

void MPVWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
    //setWindowState(Qt::WindowFullScreen);
}
