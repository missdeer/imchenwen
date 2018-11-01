#include "mpvwindow.h"
#include "mpvwidget.h"
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
    m_mpv->command(QStringList() << "loadfile" << medias);
}

void MPVWindow::closeEvent(QCloseEvent *event)
{
    m_mpv->command(QStringList() << "stop");
    event->accept();
}

void MPVWindow::seek(int pos)
{
    m_mpv->command(QVariantList() << "seek" << pos << "absolute");
}

void MPVWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
    setWindowState(Qt::WindowFullScreen);
}
