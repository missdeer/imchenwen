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


static QString secToTime(int second, bool use_format = false)
{
    static QString format = "<span style=\" font-size:14pt; font-weight:600;color:#00ff00;\">%1:%2:%3</span>";
    QString  hour = QString::number(second / 3600);
    QString min = QString::number((second % 3600) / 60);
    QString sec = QString::number(second % 60);
    if (min.length() == 1)
        min.prepend('0');
    if (sec.length() == 1)
        sec.prepend('0');
    if (use_format)
        return format.arg(hour, min, sec);
    else
        return QString("%1:%2:%3").arg(hour, min, sec);
}

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
    leftBorder = new Border(this, Border::LEFT);
    rightBorder = new Border(this, Border::RIGHT);
    bottomBorder = new Border(this, Border::BOTTOM);
    bottomLeftBorder = new Border(this, Border::BOTTOMLEFT);
    bottomRightBorder = new Border(this, Border::BOTTOMRIGHT);
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(ui->titleBar, 0, 0, 1, 3);
    gridLayout->addWidget(leftBorder, 1, 0, 1, 1);
    gridLayout->addWidget(rightBorder, 1, 2, 1, 1);
    gridLayout->addWidget(bottomBorder, 2, 1, 1, 1);
    gridLayout->addWidget(bottomLeftBorder, 2, 0, 1, 1);
    gridLayout->addWidget(bottomRightBorder, 2, 2, 1, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    setAcceptDrops(true);
    setMinimumSize(QSize(640, 360));

    quit_requested = false;
    no_play_next = false;
    ctrl_pressed = false;

    // create player core
    core = new PlayerCore(this);
    core->move(0, 0);
    core->setAttribute(Qt::WA_TransparentForMouseEvents);

    // create volume slider
    QWidget *volumePopup = new QWidget(this, Qt::Popup);
    volumePopup->resize(QSize(24, 80));
    volumeSlider = new QSlider(Qt::Vertical, volumePopup);
    volumeSlider->setRange(0, 10);
    volumeSlider->setValue(10);
    volumeSlider->resize(QSize(20, 70));
    volumeSlider->move(2, 5);

    // create menu
    QMenu *video_menu = new QMenu(tr("Video"));
    video_menu->addAction("4:3", core, SLOT(setRatio_4_3()));
    video_menu->addAction("16:9", core, SLOT(setRatio_16_9()));
    video_menu->addAction("16:10", core, SLOT(setRatio_16_10()));
    video_menu->addAction(tr("Default"), core, SLOT(setRatio_0()));
    video_menu->addSeparator();
    video_menu->addAction(tr("Equalizer"), ui->equalizerWidget, SLOT(show()));

    QMenu *audio_menu = new QMenu(tr("Audio"));
    audio_menu->addAction(tr("Stereo"), core, SLOT(setChannel_Stereo()));
    audio_menu->addAction(tr("Left channel"), core, SLOT(setChannel_Left()));
    audio_menu->addAction(tr("Right channel"), core, SLOT(setChannel_Right()));
    audio_menu->addAction(tr("Swap channel"), core, SLOT(setChannel_Swap()));
    audio_menu->addSeparator();
    audio_menu->addAction(tr("Select track"), this, SLOT(selectAudioTrack()));
    audio_menu->addAction(tr("Load from file"), this, SLOT(addAudioTrack()));
    audio_menu->addAction(tr("Delay"), this, SLOT(setAudioDelay()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up") + "\tCtrl+Right", core, SLOT(speedUp()));
    speed_menu->addAction(tr("Speed down") + "\tCtrl+Left", core, SLOT(speedDown()));
    speed_menu->addAction(tr("Default") + "\tR", core, SLOT(speedSetToDefault()));

    menu = new QMenu(this);
    menu->addMenu(video_menu);
    menu->addMenu(audio_menu);
    menu->addMenu(speed_menu);

    menu->addSeparator();
    menu->addAction(tr("Screenshot") + "\tS", core, SLOT(screenShot()));
    menu->addAction(tr("Cut video") + "\tC", this, SLOT(showCutterBar()));

    menu->addSeparator();

    // create cutterbar
    cutterBar = new CutterBar(this);
    cutterBar->setWindowFlags(cutterBar->windowFlags() | Qt::Popup);

    // create timer
    hideTimer = new QTimer(this);
    hideTimer->setSingleShot(true);
    setMouseTracking(true);

    connect(core, &PlayerCore::lengthChanged, this, &PlayerView::onLengthChanged);
    connect(core, &PlayerCore::sizeChanged, this, &PlayerView::onSizeChanged);
    connect(core, &PlayerCore::timeChanged, this, &PlayerView::onTimeChanged);
    connect(core, &PlayerCore::played, ui->pauseButton, &QPushButton::show);
    connect(core, &PlayerCore::played, ui->playButton, &QPushButton::hide);
    connect(core, &PlayerCore::paused, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::paused, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::stopped, this, &PlayerView::onStopped);
    connect(hideTimer, &QTimer::timeout, this, &PlayerView::hideElements);
    connect(volumeSlider, &QSlider::valueChanged, core, &PlayerCore::setVolume);
    connect(volumeSlider, &QSlider::valueChanged, this, &PlayerView::saveVolume);
    connect(cutterBar, &CutterBar::newFrame, core, &PlayerCore::jumpTo);
    connect(ui->stopButton, &QPushButton::clicked, this, &PlayerView::onStopButton);
    connect(ui->maxButton, &QPushButton::clicked, this, &PlayerView::onMaxButton);
    connect(ui->playButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &PlayerView::showVolumeSlider);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &PlayerView::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &PlayerView::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &PlayerView::onTimeSliderReleased);

    connect(ui->brightnessSlider, &QSlider::valueChanged, core, &PlayerCore::setBrightness);
    connect(ui->contrastSlider, &QSlider::valueChanged, core, &PlayerCore::setContrast);
    connect(ui->saturationSlider, &QSlider::valueChanged, core, &PlayerCore::setSaturation);
    connect(ui->gammaSlider, &QSlider::valueChanged, core, &PlayerCore::setGamma);
    connect(ui->hueSlider, &QSlider::valueChanged, core, &PlayerCore::setHue);

    volumeSlider->setValue(50);
}

PlayerView::~PlayerView()
{
    delete ui;
}

void PlayerView::playMedias(const QStringList &medias)
{
    player_core->openMedias(medias);
}

void PlayerView::title(const QString &title)
{
    if (!title.isEmpty())
        setWindowTitle(title + " - " + tr("imchenwen builtin media player"));
}

void PlayerView::referrer(const QString &referrer)
{
    if (!referrer.isEmpty())
        player_core->setProperty("referrer", referrer);
}

void PlayerView::userAgent(const QString &userAgent)
{
    if (!userAgent.isEmpty())
        player_core->setOption("user-agent", userAgent);
}

void PlayerView::onStopButton()
{
    no_play_next = true;
    core->stop();
}

void PlayerView::onStopped()
{
    if (quit_requested)
    {
        quit_requested = false;
        close();
    }
    else if (no_play_next)
    {
        no_play_next = false;
        ui->playButton->show();
        ui->pauseButton->hide();
    }
}

void PlayerView::closeEvent(QCloseEvent *e)
{
    no_play_next = true;

    // It's not safe to quit until mpv is stopped
    if (core->state != PlayerCore::STOPPING)
    {
        core->stop();
        quit_requested = true;
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
    core->resize(e->size());

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
    leftBorder->raise();
    rightBorder->raise();
    bottomBorder->raise();
    bottomLeftBorder->raise();
    bottomRightBorder->raise();
    ui->titleBar->raise();

    e->accept();
}

// Keyboard shortcuts
void PlayerView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Control:
        ctrl_pressed = true;
        break;
    case Qt::Key_S:
        core->screenShot();
        break;
    case Qt::Key_C:
        showCutterBar();
        break;
    case Qt::Key_O:
        if (ctrl_pressed)
        {
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_U:
        if (ctrl_pressed)
        {
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_Space:
        core->changeState();
        break;
    case Qt::Key_Return:
        setFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen())
            setFullScreen();
        break;
    case Qt::Key_R:
        core->speedSetToDefault();
        break;
    case Qt::Key_Comma:
        if (ctrl_pressed)
        {
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_Left:
        if (ctrl_pressed)
            core->speedDown();
        else
            ui->timeSlider->setValue(ui->timeSlider->value() - 5);
        break;

    case Qt::Key_Right:
        if (ctrl_pressed)
            core->speedUp();
        else
            ui->timeSlider->setValue(ui->timeSlider->value() + 5);
        break;

    case Qt::Key_Up:
        volumeSlider->setValue(volumeSlider->value() + 1);
        break;
    case Qt::Key_Down:
        volumeSlider->setValue(volumeSlider->value() - 1);
        break;
    default: break;
    }
    e->accept();
}

void PlayerView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        ctrl_pressed = false;
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
        dPos = e->pos();
    e->accept();
}

void PlayerView::mouseMoveEvent(QMouseEvent *e)
{
    // move window
    if (!dPos.isNull())
        move(e->globalPos() - dPos);

    // show controller, titlebar and cursor
    hideTimer->stop();
    ui->controllerWidget->show();
    ui->titleBar->show();
    setCursor(QCursor(Qt::ArrowCursor));
    if (core->state == PlayerCore::VIDEO_PLAYING || core->state == PlayerCore::TV_PLAYING)
        hideTimer->start(2000);
    e->accept();
}

void PlayerView::mouseReleaseEvent(QMouseEvent *e)
{
    dPos = QPoint();
    e->accept();
}

void PlayerView::contextMenuEvent(QContextMenuEvent *e)
{
    menu->exec(QCursor::pos());
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
    if (core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void PlayerView::onTimeSliderValueChanged(int time)
{
    if (core->state == PlayerCore::STOPPING)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        core->setProgress(time);
}

void PlayerView::onTimeSliderReleased()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    core->setProgress(ui->timeSlider->value());
}

void PlayerView::onSizeChanged(const QSize &sz)
{
    if (isFullScreen())
        return;
    QRect available = QApplication::desktop()->availableGeometry(this);
    if (sz.width() / devicePixelRatioF() > available.width() || sz.height()/ devicePixelRatioF() > available.height())
        setGeometry(available);
    else
        resize(sz / devicePixelRatioF());
}

// show cutterbar
void PlayerView::showCutterBar()
{
    if (core->state == PlayerCore::STOPPING || core->state == PlayerCore::TV_PLAYING || cutterBar->isVisible())
        return;
    QString filename = core->currentFile();
    if (filename.startsWith("http"))
    {
        QMessageBox::warning(this, tr("Error"), tr("Only support cutting local videos!"), QMessageBox::Ok);
        return;
    }
    if (core->state == PlayerCore::VIDEO_PLAYING) //pause
        core->changeState();
    cutterBar->init(filename, core->getLength(), core->getTime());
    cutterBar->move(mapToGlobal(QPoint(50, 50)));
    cutterBar->show();
}


// show volume slider
void PlayerView::showVolumeSlider()
{
    QWidget *volumePopup = volumeSlider->window();
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
    core->pauseRendering();
    QTimer::singleShot(1500, core, SLOT(unpauseRendering()));

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
    QString videoFile = core->currentFile();
    QString dir = videoFile.startsWith('/') ? QFileInfo(videoFile).path() : QDir::homePath();
    QString audioFile = QFileDialog::getOpenFileName(this, tr("Open audio track file"), dir);
    if (!audioFile.isEmpty())
        core->openAudioTrack(audioFile);
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
    double delay = QInputDialog::getDouble(this, "Input", tr("Audio delay (sec):"), core->getAudioDelay(), -100, 100, 1, &ok);
    if (ok)
        core->setAudioDelay(delay);
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
