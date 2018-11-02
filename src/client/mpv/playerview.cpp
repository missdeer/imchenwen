#include "playerview.h"
#include "ui_playerview.h"
#include "cutterbar.h"
#include "playercore.h"
#include "skin.h"
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

PlayerView::PlayerView(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::PlayerView)
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
    QPushButton *buttons2[] = {ui->playlistButton, ui->searchButton, ui->volumeButton, ui->settingsButton, ui->hideEqualizerButton};
    for (int i = 0; i < 5; i++)
    {
        buttons2[i]->setIconSize(QSize(16, 16));
        buttons2[i]->setFixedSize(QSize(24, 20));
    }
    ui->controllerWidget->setFixedSize(QSize(450, 70));
    ui->equalizerWidget->setFixedSize(QSize(350, 180));
    ui->equalizerWidget->hide();
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

    // create player core
    m_playerCore = new PlayerCore(this);
    m_playerCore->move(0, 0);
    m_playerCore->setAttribute(Qt::WA_TransparentForMouseEvents);

    // create volume slider
    QWidget *volumePopup = new QWidget(this, Qt::Popup);
    volumePopup->resize(QSize(24, 80));
    m_volumeSlider = new QSlider(Qt::Vertical, volumePopup);
    m_volumeSlider->setRange(0, 10);
    m_volumeSlider->setValue(10);
    m_volumeSlider->resize(QSize(20, 70));
    m_volumeSlider->move(2, 5);

    // create menu
    QMenu *video_menu = new QMenu(tr("Video"));
    video_menu->addAction("4:3", m_playerCore, SLOT(setRatio_4_3()));
    video_menu->addAction("16:9", m_playerCore, SLOT(setRatio_16_9()));
    video_menu->addAction("16:10", m_playerCore, SLOT(setRatio_16_10()));
    video_menu->addAction(tr("Default"), m_playerCore, SLOT(setRatio_0()));
    video_menu->addSeparator();
    video_menu->addAction(tr("Equalizer"), ui->equalizerWidget, SLOT(show()));

    QMenu *audio_menu = new QMenu(tr("Audio"));
    audio_menu->addAction(tr("Stereo"), m_playerCore, SLOT(setChannel_Stereo()));
    audio_menu->addAction(tr("Left channel"), m_playerCore, SLOT(setChannel_Left()));
    audio_menu->addAction(tr("Right channel"), m_playerCore, SLOT(setChannel_Right()));
    audio_menu->addAction(tr("Swap channel"), m_playerCore, SLOT(setChannel_Swap()));
    audio_menu->addSeparator();
    audio_menu->addAction(tr("Select track"), this, SLOT(selectAudioTrack()));
    audio_menu->addAction(tr("Load from file"), this, SLOT(addAudioTrack()));
    audio_menu->addAction(tr("Delay"), this, SLOT(setAudioDelay()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up") + "\tCtrl+Right", m_playerCore, SLOT(speedUp()));
    speed_menu->addAction(tr("Speed down") + "\tCtrl+Left", m_playerCore, SLOT(speedDown()));
    speed_menu->addAction(tr("Default") + "\tR", m_playerCore, SLOT(speedSetToDefault()));

    m_menu = new QMenu(this);
    m_menu->addMenu(video_menu);
    m_menu->addMenu(audio_menu);
    m_menu->addMenu(speed_menu);

    m_menu->addSeparator();
    m_menu->addAction(tr("Screenshot") + "\tS", m_playerCore, SLOT(screenShot()));
    m_menu->addAction(tr("Cut video") + "\tC", this, SLOT(showCutterBar()));

    m_menu->addSeparator();

    // create cutterbar
    m_cutterBar = new CutterBar(this);
    m_cutterBar->setWindowFlags(m_cutterBar->windowFlags() | Qt::Popup);

    // create timer
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    setMouseTracking(true);

    connect(m_playerCore, &PlayerCore::lengthChanged, this, &PlayerView::onLengthChanged);
    connect(m_playerCore, &PlayerCore::sizeChanged, this, &PlayerView::onSizeChanged);
    connect(m_playerCore, &PlayerCore::timeChanged, this, &PlayerView::onTimeChanged);
    connect(m_playerCore, &PlayerCore::played, ui->pauseButton, &QPushButton::show);
    connect(m_playerCore, &PlayerCore::played, ui->playButton, &QPushButton::hide);
    connect(m_playerCore, &PlayerCore::paused, ui->playButton, &QPushButton::show);
    connect(m_playerCore, &PlayerCore::paused, ui->pauseButton, &QPushButton::hide);
    connect(m_playerCore, &PlayerCore::stopped, this, &PlayerView::onStopped);
    connect(m_hideTimer, &QTimer::timeout, this, &PlayerView::hideElements);
    connect(m_volumeSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setVolume);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &PlayerView::saveVolume);
    connect(m_cutterBar, &CutterBar::newFrame, m_playerCore, &PlayerCore::jumpTo);
    connect(ui->stopButton, &QPushButton::clicked, this, &PlayerView::onStopButton);
    connect(ui->maxButton, &QPushButton::clicked, this, &PlayerView::onMaxButton);
    connect(ui->playButton, &QPushButton::clicked, m_playerCore, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, m_playerCore, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &PlayerView::showVolumeSlider);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &PlayerView::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &PlayerView::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &PlayerView::onTimeSliderReleased);

    connect(ui->brightnessSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setBrightness);
    connect(ui->contrastSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setContrast);
    connect(ui->saturationSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setSaturation);
    connect(ui->gammaSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setGamma);
    connect(ui->hueSlider, &QSlider::valueChanged, m_playerCore, &PlayerCore::setHue);

    m_volumeSlider->setValue(50);
}

PlayerView::~PlayerView()
{
    delete ui;
}

void PlayerView::playMedias(const QStringList &medias)
{
    m_playerCore->openMedias(medias);
}

void PlayerView::title(const QString &title)
{
    if (!title.isEmpty())
        setWindowTitle(title + " - " + tr("imchenwen builtin media player"));
}

void PlayerView::referrer(const QString &referrer)
{
    if (!referrer.isEmpty())
        m_playerCore->setProperty("referrer", referrer);
}

void PlayerView::userAgent(const QString &userAgent)
{
    if (!userAgent.isEmpty())
        m_playerCore->setOption("user-agent", userAgent);
}

void PlayerView::onStopButton()
{
    m_noPlayNext = true;
    m_playerCore->stop();
}

void PlayerView::onStopped()
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

void PlayerView::closeEvent(QCloseEvent *e)
{
    m_noPlayNext = true;

    // It's not safe to quit until mpv is stopped
    if (m_playerCore->state != PlayerCore::STOPPING)
    {
        m_playerCore->stop();
        m_quitRequested = true;
        e->ignore();
    }
    else
    {
        e->accept();
        Q_EMIT finished(0, QProcess::NormalExit);
    }
}

// Drag & drop files
void PlayerView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void PlayerView::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    bool first = true;
    foreach (QUrl url, urls)
    {
        if (url.isLocalFile())
        {
            // first file
            if (first)
            {
                first = false;
            }
        }
    }
    e->accept();
}

// resize ui
void PlayerView::resizeEvent(QResizeEvent *e)
{
    // resize player core
    m_playerCore->resize(e->size());

    // move and resize controller
    int c_x = (e->size().width() - ui->controllerWidget->width()) / 2;
    int c_y = e->size().height() - 130;
    ui->controllerWidget->move(c_x, c_y);
    ui->controllerWidget->raise();

    // move and resize equalizer
    int e_x = (e->size().width() - ui->equalizerWidget->width()) / 2;
    int e_y = (e->size().height() - ui->equalizerWidget->height()) / 2 - 30;
    ui->equalizerWidget->move(e_x, e_y);
    ui->equalizerWidget->raise();

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
void PlayerView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Control:
        m_ctrlPressed = true;
        break;
    case Qt::Key_S:
        m_playerCore->screenShot();
        break;
    case Qt::Key_C:
        showCutterBar();
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
        m_playerCore->changeState();
        break;
    case Qt::Key_Return:
        setFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen())
            setFullScreen();
        break;
    case Qt::Key_R:
        m_playerCore->speedSetToDefault();
        break;
    case Qt::Key_Comma:
        if (m_ctrlPressed)
        {
            // key-release event may not be received after dialog is shown
            m_ctrlPressed = false;
        }
        break;
    case Qt::Key_Left:
        if (m_ctrlPressed)
            m_playerCore->speedDown();
        else
            ui->timeSlider->setValue(ui->timeSlider->value() - 5);
        break;

    case Qt::Key_Right:
        if (m_ctrlPressed)
            m_playerCore->speedUp();
        else
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

void PlayerView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        m_ctrlPressed = false;
    e->accept();
}

void PlayerView::mouseDoubleClickEvent(QMouseEvent *e)
{
    /* On macOS, this event will be emitted without double-click when mouse
     * is moved to screen edge.
     * Is it a Qt's bug?
     */
    if (e->buttons() == Qt::LeftButton && QRect(0, 0, width(), height()).contains(e->pos(), true))
        setFullScreen();
    e->accept();
}


void PlayerView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_dPos = e->pos();
    e->accept();
}

void PlayerView::mouseMoveEvent(QMouseEvent *e)
{
    // move window
    if (!m_dPos.isNull())
        move(e->globalPos() - m_dPos);

    // show controller, titlebar and cursor
    m_hideTimer->stop();
    ui->controllerWidget->show();
    ui->titleBar->show();
    setCursor(QCursor(Qt::ArrowCursor));
    if (m_playerCore->state == PlayerCore::VIDEO_PLAYING || m_playerCore->state == PlayerCore::TV_PLAYING)
        m_hideTimer->start(2000);
    e->accept();
}

void PlayerView::mouseReleaseEvent(QMouseEvent *e)
{
    m_dPos = QPoint();
    e->accept();
}

void PlayerView::contextMenuEvent(QContextMenuEvent *e)
{
    m_menu->exec(QCursor::pos());
    e->accept();
}

void PlayerView::hideElements()
{
    ui->titleBar->hide();
    if (!ui->controllerWidget->geometry().contains(mapFromGlobal(QCursor::pos())) && !ui->equalizerWidget->isVisible())
    {
        // mouse is not in controller and equalizer is hidden
        ui->controllerWidget->hide();
        setCursor(QCursor(Qt::BlankCursor));
    }
}

void PlayerView::onLengthChanged(int len)
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

void PlayerView::onTimeChanged(int time)
{
    ui->timeLabel->setText(secToTime(time));
    if (!ui->timeSlider->isSliderDown() && time % 4 == 0) // Make slider easier to drag
        ui->timeSlider->setValue(time);
}

void PlayerView::onTimeSliderPressed()
{
    if (m_playerCore->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void PlayerView::onTimeSliderValueChanged(int time)
{
    if (m_playerCore->state == PlayerCore::STOPPING)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        m_playerCore->setProgress(time);
}

void PlayerView::onTimeSliderReleased()
{
    if (m_playerCore->state == PlayerCore::STOPPING)
        return;
    m_playerCore->setProgress(ui->timeSlider->value());
}

void PlayerView::onSizeChanged(const QSize &sz)
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

// show cutterbar
void PlayerView::showCutterBar()
{
    if (m_playerCore->state == PlayerCore::STOPPING || m_playerCore->state == PlayerCore::TV_PLAYING || m_cutterBar->isVisible())
        return;
    QString filename = m_playerCore->currentFile();
    if (!QFile::exists(filename))
    {
        QMessageBox::warning(this, tr("Error"), tr("Only support cutting local videos!"), QMessageBox::Ok);
        return;
    }
    if (m_playerCore->state == PlayerCore::VIDEO_PLAYING) //pause
        m_playerCore->changeState();
    m_cutterBar->init(filename, m_playerCore->getLength(), m_playerCore->getTime());
    m_cutterBar->move(mapToGlobal(QPoint(50, 50)));
    m_cutterBar->show();
}


// show volume slider
void PlayerView::showVolumeSlider()
{
    QWidget *volumePopup = m_volumeSlider->window();
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->volumeButton->pos());
    volumePopup->move(vbPos.x(), vbPos.y() - volumePopup->height());
    volumePopup->show();
}

void PlayerView::saveVolume(int vol)
{
    //
}

// show or exit fullscreen
void PlayerView::setFullScreen()
{
    // avoid freezing
    m_playerCore->pauseRendering();
    QTimer::singleShot(1500, m_playerCore, SLOT(unpauseRendering()));

    if (isFullScreen())
        showNormal();
    else
    {
#ifdef Q_OS_MAC
        setWindowFlag(Qt::FramelessWindowHint, false);
        show();
        QTimer::singleShot(0, this, SLOT(showFullScreen()));
#else
        showFullScreen();
#endif
    }
}

#ifdef Q_OS_MAC
void PlayerView::changeEvent(QEvent *e)
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


// add audio track, select audio track and set audio delay
void PlayerView::addAudioTrack()
{
    QString videoFile = m_playerCore->currentFile();
    QString dir = videoFile.startsWith('/') ? QFileInfo(videoFile).path() : QDir::homePath();
    QString audioFile = QFileDialog::getOpenFileName(this, tr("Open audio track file"), dir);
    if (!audioFile.isEmpty())
        m_playerCore->openAudioTrack(audioFile);
}

void PlayerView::selectAudioTrack()
{
//    int aid = selectionDialog->showDialog_Index(core->getAudioTracksList(), tr("Select audio track:"));
//    if (aid != -1)
//        core->setAid(aid);
}

void PlayerView::setAudioDelay()
{
    bool ok = false;
    double delay = QInputDialog::getDouble(this, "Input", tr("Audio delay (sec):"), m_playerCore->getAudioDelay(), -100, 100, 1, &ok);
    if (ok)
        m_playerCore->setAudioDelay(delay);
}


// maximize or normalize window
void PlayerView::onMaxButton()
{
    if (isFullScreen())
        return;
    else if (isMaximized())
        showNormal();
    else
        showMaximized();
}
