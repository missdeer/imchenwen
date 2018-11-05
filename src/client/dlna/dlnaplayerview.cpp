#include "dlnaplayerview.h"
#include "ui_dlnaplayerview.h"
#include "skin.h"
#include "config.h"
#include "util.h"
#include "browser.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QResizeEvent>
#include <QTimer>

DLNAPlayerView::DLNAPlayerView(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::DLNAPlayerView)
{
    // init ui
    ui->setupUi(this);
    ui->pauseButton->hide();
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton};
    for (int i = 0; i < 3; i++)
    {
        buttons[i]->setIconSize(QSize(16, 16));
        buttons[i]->setFixedSize(QSize(32, 32));
    }
    QPushButton *buttons2[] = {ui->playlistButton, ui->searchButton, ui->volumeButton, ui->settingsButton};
    for (int i = 0; i < 5; i++)
    {
        buttons2[i]->setIconSize(QSize(16, 16));
        buttons2[i]->setFixedSize(QSize(24, 20));
    }
    ui->controllerWidget->setFixedSize(QSize(450, 70));
    ui->closeButton->setFixedSize(QSize(16, 16));
    ui->minButton->setFixedSize(QSize(16, 16));
    ui->maxButton->setFixedSize(QSize(16, 16));
    ui->titleBar->setFixedHeight(24);
    ui->titlebarLayout->setContentsMargins(QMargins(8, 4, 8, 4));
    ui->titlebarLayout->setSpacing(4);
    m_leftBorder = new Border(this, Border::LEFT);
    m_rightBorder = new Border(this, Border::RIGHT);
    m_bottomBorder = new Border(this, Border::BOTTOM);
    m_bottomLeftBorder = new Border(this, Border::BOTTOMLEFT);
    m_bottomRightBorder = new Border(this, Border::BOTTOMRIGHT);
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(ui->titleBar, 0, 0, 1, 3);
    gridLayout->addWidget(m_leftBorder, 1, 0, 1, 1);
    gridLayout->addWidget(m_rightBorder, 1, 2, 1, 1);
    gridLayout->addWidget(m_bottomBorder, 2, 1, 1, 1);
    gridLayout->addWidget(m_bottomLeftBorder, 2, 0, 1, 1);
    gridLayout->addWidget(m_bottomRightBorder, 2, 2, 1, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    setAcceptDrops(true);
    setMinimumSize(QSize(640, 360));

    m_quitRequested = false;
    m_noPlayNext = false;
    m_ctrlPressed = false;
    m_paused = false;

    // create volume slider
    QWidget *volumePopup = new QWidget(this, Qt::Popup);
    volumePopup->resize(QSize(24, 80));
    m_volumeSlider = new QSlider(Qt::Vertical, volumePopup);
    m_volumeSlider->setRange(0, 10);
    m_volumeSlider->setValue(10);
    m_volumeSlider->resize(QSize(20, 70));
    m_volumeSlider->move(2, 5);

    connect(m_volumeSlider, &QSlider::valueChanged, this, &DLNAPlayerView::saveVolume);
    connect(ui->stopButton, &QPushButton::clicked, this, &DLNAPlayerView::onStopButton);
    connect(ui->maxButton, &QPushButton::clicked, this, &DLNAPlayerView::onMaxButton);
    connect(ui->playButton, &QPushButton::clicked, this, &DLNAPlayerView::onPlay);
    connect(ui->pauseButton, &QPushButton::clicked, this, &DLNAPlayerView::onPause);
    connect(ui->volumeButton, &QPushButton::clicked, this, &DLNAPlayerView::showVolumeSlider);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &DLNAPlayerView::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &DLNAPlayerView::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &DLNAPlayerView::onTimeSliderReleased);

    Config cfg;
    m_volumeSlider->setValue(cfg.read<int>("volumn", 50));
}

DLNAPlayerView::~DLNAPlayerView()
{
    Browser::instance().kast().stop(m_renderer);
    delete ui;
}

void DLNAPlayerView::playMedias(const QStringList &medias)
{
    Browser::instance().kast().stop(m_renderer);

    QUrl u(medias[0]);
    Browser::instance().kast().setPlaybackUrl(m_renderer, u, QFileInfo(u.path()));
    for (int i = 1; i < medias.length(); i++)
    {
        QUrl u(medias[i]);
        Browser::instance().kast().setNextPlaybackUrl(m_renderer, u);
    }
}

void DLNAPlayerView::title(const QString &title)
{
    if (!title.isEmpty())
    {
        setWindowTitle(title + " - " + tr("imchenwen DMC"));
    }
}

void DLNAPlayerView::referrer(const QString &referrer)
{
}

void DLNAPlayerView::userAgent(const QString &userAgent)
{
}

void DLNAPlayerView::onStopButton()
{
    m_noPlayNext = true;
}

void DLNAPlayerView::onStopped()
{
    if (m_quitRequested)
    {
        m_quitRequested = false;
        close();
    }
    else if (m_noPlayNext)
    {
        m_noPlayNext = false;
        ui->playButton->show();
        ui->pauseButton->hide();
    }
}

void DLNAPlayerView::closeEvent(QCloseEvent *e)
{
    m_noPlayNext = true;

    //TODO: stop playback

    e->accept();
    Q_EMIT finished(0, QProcess::NormalExit);
}

// resize ui
void DLNAPlayerView::resizeEvent(QResizeEvent *e)
{
    // move and resize controller
    int c_x = (e->size().width() - ui->controllerWidget->width()) / 2;
    int c_y = e->size().height() - 130;
    ui->controllerWidget->move(c_x, c_y);
    ui->controllerWidget->raise();

    // raise borders and titlebar
    m_leftBorder->raise();
    m_rightBorder->raise();
    m_bottomBorder->raise();
    m_bottomLeftBorder->raise();
    m_bottomRightBorder->raise();
    ui->titleBar->raise();

    e->accept();
}

// Keyboard shortcuts
void DLNAPlayerView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Control:
        m_ctrlPressed = true;
        break;
    case Qt::Key_O:
        if (m_ctrlPressed)
        {
            // key-release event may not be received after dialog is shown
            m_ctrlPressed = false;
        }
        break;
    case Qt::Key_U:
        if (m_ctrlPressed)
        {
            // key-release event may not be received after dialog is shown
            m_ctrlPressed = false;
        }
        break;
    case Qt::Key_Space:
        if (m_paused)
            onResume();
        else
            onPause();
        break;
    case Qt::Key_Comma:
        if (m_ctrlPressed)
        {
            // key-release event may not be received after dialog is shown
            m_ctrlPressed = false;
        }
        break;
    case Qt::Key_Left:
        ui->timeSlider->setValue(ui->timeSlider->value() - 5);
        break;
    case Qt::Key_Right:
        ui->timeSlider->setValue(ui->timeSlider->value() + 5);
        break;
    case Qt::Key_Up:
        m_volumeSlider->setValue(m_volumeSlider->value() + 1);
        break;
    case Qt::Key_Down:
        m_volumeSlider->setValue(m_volumeSlider->value() - 1);
        break;
    default: break;
    }
    e->accept();
}

void DLNAPlayerView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        m_ctrlPressed = false;
    e->accept();
}

void DLNAPlayerView::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();
}


void DLNAPlayerView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_dPos = e->pos();
    e->accept();
}

void DLNAPlayerView::mouseMoveEvent(QMouseEvent *e)
{
    // move window
    if (!m_dPos.isNull())
        move(e->globalPos() - m_dPos);

    // show controller, titlebar and cursor
    ui->controllerWidget->show();
    ui->titleBar->show();
    setCursor(QCursor(Qt::ArrowCursor));
    e->accept();
}

void DLNAPlayerView::mouseReleaseEvent(QMouseEvent *e)
{
    m_dPos = QPoint();
    e->accept();
}

void DLNAPlayerView::onLengthChanged(int len)
{
    if (len == 0) //playing TV
        ui->timeSlider->setEnabled(false);
    else //playing video
    {
        ui->timeSlider->setEnabled(true);
        ui->timeSlider->setMaximum(len);
        ui->durationLabel->setText(secToTime(len));
    }
    activateWindow();
    raise();
}

void DLNAPlayerView::onTimeChanged(int time)
{
    ui->timeLabel->setText(secToTime(time));
    if (!ui->timeSlider->isSliderDown() && time % 4 == 0) // Make slider easier to drag
        ui->timeSlider->setValue(time);
}

void DLNAPlayerView::onTimeSliderPressed()
{
    if (m_paused)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void DLNAPlayerView::onTimeSliderValueChanged(int time)
{
    if (m_paused)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        Browser::instance().kast().seekPlayback(m_renderer, QTime::fromString(secToTime(time), "h:m:s"));
}

void DLNAPlayerView::onTimeSliderReleased()
{
    if (m_paused)
        return;
    Browser::instance().kast().seekPlayback(m_renderer, QTime::fromString(secToTime(ui->timeSlider->value()), "h:m:s"));
}

void DLNAPlayerView::onSizeChanged(const QSize &sz)
{
    if (isFullScreen())
        return;
    QRect available = QApplication::desktop()->availableGeometry(this);
    if (sz.width() / devicePixelRatioF() > available.width() || sz.height()/ devicePixelRatioF() > available.height())
        setGeometry(available);
    else
    {
        resize(sz / devicePixelRatioF());
        move((available.width() - sz.width()/devicePixelRatioF())/2, (available.height() - sz.height()/devicePixelRatioF())/2);
    }
}

// show volume slider
void DLNAPlayerView::showVolumeSlider()
{
    QWidget *volumePopup = m_volumeSlider->window();
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->volumeButton->pos());
    volumePopup->move(vbPos.x(), vbPos.y() - volumePopup->height());
    volumePopup->show();
}

void DLNAPlayerView::onPlay()
{
    Browser::instance().kast().play(m_renderer);
}

void DLNAPlayerView::onPause()
{
    Browser::instance().kast().pause(m_renderer);
}

void DLNAPlayerView::onResume()
{
    Browser::instance().kast().resume(m_renderer);
}

void DLNAPlayerView::setRenderer(const QString &renderer)
{
    m_renderer = renderer;
}

void DLNAPlayerView::saveVolume(int vol)
{
    Config cfg;
    cfg.write("volume", vol);
}


#ifdef Q_OS_MAC
void DLNAPlayerView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *ce = static_cast<QWindowStateChangeEvent*>(e);
        if ((ce->oldState() & Qt::WindowFullScreen) && !isFullScreen())
            setWindowFlag(Qt::FramelessWindowHint, true);
        ce->accept();
        return;
    }
    QWidget::changeEvent(e);
}
#endif


// maximize or normalize window
void DLNAPlayerView::onMaxButton()
{
    if (isFullScreen())
        return;
    else if (isMaximized())
        showNormal();
    else
        showMaximized();
}
