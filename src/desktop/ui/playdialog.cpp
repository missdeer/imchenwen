#include <QClipboard>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QMovie>
#include <QtCore>

#include "playdialog.h"

#include "browser.h"
#include "settings.h"
#include "ui_playdialog.h"

PlayDialog::PlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayDialog),
    m_complexUrlResources(false),
    m_demuxed(false)
{
    ui->setupUi(this);

    QMovie *luxMovie = new QMovie(":/loader.gif");
    ui->luxLoader->setMovie(luxMovie);
    luxMovie->start();

    QMovie *ykdlMovie = new QMovie(":/loader.gif");
    ui->ykdlLoader->setMovie(ykdlMovie);
    ykdlMovie->start();

    QMovie *yougetMovie = new QMovie(":/loader.gif");
    ui->yougetLoader->setMovie(yougetMovie);
    yougetMovie->start();

    QMovie *ytdlMovie = new QMovie(":/loader.gif");
    ui->youtubedlLoader->setMovie(ytdlMovie);
    ytdlMovie->start();

    ui->listMedia->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listMedia, &QListWidget::customContextMenuRequested, this, &PlayDialog::onListMediaContextmenu);

    createExternalPlayerList();
    Config cfg;
    ui->cbAutoSelectHighestQualityVideoTrack->setChecked(
        cfg.read<bool>("autoSelectHighestQualityVideoTrack", true));

    ui->btnDownload->setEnabled(cfg.read<bool>(QLatin1String("enableStorageService")));
    ui->btnPlayDownload->setEnabled(cfg.read<bool>(QLatin1String("enableStorageService")));
}

PlayDialog::~PlayDialog()
{
    delete ui;
}

void PlayDialog::setMediaInfo(const QString &originalUrl, MediaInfoPtr mi)
{
    if (originalUrl != m_originalUrl)
    {
        m_resultStreams.clear();
        ui->listMedia->clear();
        m_originalUrl = originalUrl;
    }

    if (!ui->cbSubtitles->count())
    {
        bool selectedSubtitle = ui->cbSubtitleEnabled->isChecked();
        for (auto subtitle : mi->subtitles)
        {
            ui->cbSubtitles->addItem(subtitle->manual ? QIcon(":/manualgenerated.png") : QIcon(":/autogenerated.png"), subtitle->language);
            selectedSubtitle |= subtitle->manual;
            m_subtitles.append(subtitle);
        }
        if (selectedSubtitle)
        {
            int index = ui->cbSubtitles->findText("zh-CN");
            if (index >= 0)
                ui->cbSubtitles->setCurrentIndex(index);
        }
        if (!selectedSubtitle)
        {
            int index = ui->cbSubtitles->findText("en");
            if (index >= 0)
            {
                ui->cbSubtitles->setCurrentIndex(index);
                selectedSubtitle = true;
            }
        }
    }

    ui->labelSubtitle->setVisible(!m_subtitles.isEmpty());
    ui->cbSubtitles->setVisible(!m_subtitles.isEmpty());
    ui->cbSubtitleEnabled->setVisible(!m_subtitles.isEmpty());

    ui->yougetLoader->setVisible(!mi->you_getDone);
    ui->youtubedlLoader->setVisible(!mi->youtube_dlDone);
    ui->ykdlLoader->setVisible(!mi->ykdlDone);
    ui->luxLoader->setVisible(!mi->luxDone);

    struct
    {
        Streams& streams;
        QString tag;
        QIcon icon;
    } ss[] = {
        {mi->youtube_dl, tr(" - by youtube_dl"), QIcon(":/youtube-dl.png")},
        {mi->ykdl, tr(" - by ykdl"), QIcon(":/ykdl.png")},
        {mi->you_get, tr(" - by you-get"), QIcon(":/you-get.png")},
        {mi->lux, tr(" - by lux"), QIcon(":/lux.png")},
    };
    for (auto & s : ss)
    {
        for (auto stream : qAsConst(s.streams))
        {
            if (stream->urls.isEmpty())
                continue;

            QString itemText = mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality + s.tag;
            auto items = ui->listMedia->findItems(itemText, Qt::MatchExactly);
            if (!items.isEmpty())
                break;
            if (!m_demuxed && stream->maybeAudio())
            {
                m_demuxed = true;
            }

            auto item = addItem(s.icon,
                                itemText,
                                ui->listMedia->count() % 2 ? Qt::white : QColor(0xf0f0f0));
            m_resultStreams.append(stream);
            if (stream->urls.length() > 1)
                item->setToolTip(QString("%1 x %2").arg(QUrl(stream->urls[0]).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery)).arg(stream->urls.length()));
            else
                item->setToolTip(QUrl(stream->urls[0]).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery));
        }
    }
    //ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_complexUrlResources = true;
    ui->cbAutoSelectAudioTrack->setEnabled(m_demuxed);
    ui->cbAutoSelectAudioTrack->setChecked(m_demuxed);
}

void PlayDialog::setMediaInfo(const QString &originalUrl, const QString &title, const QStringList &results)
{
    if (originalUrl != m_originalUrl)
    {
        m_resultStreams.clear();
        ui->listMedia->clear();
        m_originalUrl = originalUrl;
    }

    ui->labelSubtitle->setVisible(false);
    ui->cbSubtitles->setVisible(false);
    ui->cbSubtitleEnabled->setVisible(false);

    for( const auto& url : results)
    {
        if (m_resultUrls.contains(url))
            continue;

        auto item = addItem(QIcon(":/video.png"),
                            title + "\n" + url,
                            ui->listMedia->count() % 2 ? Qt::white : QColor(0xf0f0f0));
        item->setToolTip(QUrl(url).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery));
        m_resultUrls.append(url);
    }
    //ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_complexUrlResources = false;
    ui->cbAutoSelectAudioTrack->setEnabled(false);
    ui->cbAutoSelectAudioTrack->setChecked(true);
    ui->cbAutoSelectHighestQualityVideoTrack->setEnabled(false);
    ui->cbAutoSelectHighestQualityVideoTrack->setChecked(true);

    ui->yougetLoader->setVisible(false);
    ui->youtubedlLoader->setVisible(false);
    ui->ykdlLoader->setVisible(false);
    ui->luxLoader->setVisible(false);
}

QString PlayDialog::audioUrl()
{
    if (m_selectedAudio && !m_selectedAudio->urls.isEmpty())
        return m_selectedAudio->urls[0];
    return {};
}

void PlayDialog::on_btnExternalPlayerConfiguration_clicked()
{
    SettingsDialog dlg(this);
    dlg.setCurrentPage(6);
    if (dlg.exec())
    {
        ui->cbPlayers->clear();
        createExternalPlayerList();
    }
}

void PlayDialog::on_btnPlay_clicked()
{
    m_action = PlayDialog::PLAY;
    if (doOk())
        accept();
}

void PlayDialog::on_btnCancel_clicked()
{
    m_action = PlayDialog::CANCEL;
    reject();
}

void PlayDialog::createExternalPlayerList()
{
    m_players.clear();
    PlayerPtr builtinPlayer(new Player(Player::PT_BUILTIN, tr("Built-in player"), tr("")));
    m_players.push_back(builtinPlayer);
    Config cfg;
    cfg.read("externalPlayers", m_players);
    auto dlnaRenderers = Browser::instance().kast().getRenderers();
    for (auto& r : dlnaRenderers)
    {
        PlayerPtr p(new Player(Player::PT_DLNA, r, tr("")));
        m_players.push_back(p);
    }
    for (auto& p : m_players)
    {
        switch (p->type())
        {
        case Player::PT_BUILTIN:
            ui->cbPlayers->addItem(QIcon(":/player/builtin.png"), p->title());
            break;
        case Player::PT_DLNA:
            ui->cbPlayers->addItem(QIcon(":/player/dlna.png"), tr("DLNA: ") + p->title());
            break;
        case Player::PT_EXTERNAL:
            ui->cbPlayers->addItem(QIcon(":/player/external.png"), p->title());
            break;
        }
    }
    int index = cfg.read<int>("selectedPlayerIndex", 0);
    if (index >= ui->cbPlayers->count())
        index = 0;
    ui->cbPlayers->setCurrentIndex(index);
}

bool PlayDialog::doOk()
{
    int currentIndex = ui->cbPlayers->currentIndex();
    m_selectedPlayer = m_players.at(currentIndex);

    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        if (m_complexUrlResources && ui->cbAutoSelectHighestQualityVideoTrack->isChecked())
        {
            auto it = std::max_element(m_resultStreams.begin(), m_resultStreams.end(),
                                       [](StreamInfoPtr largest, StreamInfoPtr first){return largest < first;});
            if (m_resultStreams.end() != it)
                ui->listMedia->setCurrentRow(static_cast<int>(std::distance(m_resultStreams.begin(), it)));
        }

        if (!ui->listMedia->currentItem())
        {
            QMessageBox::warning(this,
                                 tr("Error"),
                                 tr("Please select a media item in list to be played."),
                                 QMessageBox::Ok);
            return false;
        }
    }
    int currentRow = ui->listMedia->currentRow();
    if (m_complexUrlResources)
    {
        auto video = m_resultStreams[currentRow];
        if (video->maybeAudio() && QMessageBox::warning(this,
                                                           tr("Warning"),
                                                           tr("This media item may be an audio track, continue anyway?"),
                                                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            return false;
        }
        m_selectedVideo = video;
    }
    else
    {
        m_selectedUrl = m_resultUrls[currentRow];
    }

    if (ui->cbSubtitleEnabled->isChecked() && !m_subtitles.isEmpty())
    {
        currentRow = ui->cbSubtitles->currentIndex();
        m_subtitleUrl = m_subtitles[currentRow]->url;
    }
    if ((m_selectedPlayer->type() == Player::PT_EXTERNAL && QFile::exists(m_selectedPlayer->path())) ||
        m_selectedPlayer->type() != Player::PT_EXTERNAL)
    {
        if (m_demuxed && !m_selectedAudio)
        {
            if (QMessageBox::question(this,
                                      tr("Confirm"),
                                      tr("No stream is marked as audio track, continue anyway?"),
                                      QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No)
                return false;
        }
        Config cfg;
        cfg.write("selectedPlayerIndex", currentIndex);
        return true;
    }

    QMessageBox::warning(
        this, tr("Error"), tr("Cannot find player '%1', please reconfiguration it.").arg(m_selectedPlayer->title()), QMessageBox::Ok);
    return false;
}

QListWidgetItem * PlayDialog::addItem(const QIcon& icon, const QString& text, const QColor &backgroundColor)
{
    QListWidgetItem *item = new QListWidgetItem(icon, text, ui->listMedia);
    QFont font(item->font());
#if defined(Q_OS_WIN)
    font.setFamily("Microsoft YaHei");
#elif defined(Q_OS_MAC)
    font.setFamily("Pingfang SC");
#endif
    font.setPixelSize(14);
    item->setFont(font);
    item->setBackground(QBrush(backgroundColor));
    ui->listMedia->addItem(item);
    return item;
}

void PlayDialog::on_listMedia_itemActivated(QListWidgetItem *)
{
    m_action = PlayDialog::PLAY_AND_DOWNLOAD;
    if (doOk())
        accept();
}

void PlayDialog::onListMediaContextmenu(const QPoint &pos)
{
    QPoint globalPos = ui->listMedia->mapToGlobal(pos);

    QMenu menu;
    menu.addAction(tr("Mark as Audio Track"), this, SLOT(onMarkAsAudioTrack()));
    menu.addAction(tr("Unmark as Audio Track"),  this, SLOT(onUnmarkAsAudioTrack()));
    menu.addSeparator();

    Config cfg;
    if (cfg.read<bool>(QLatin1String("enableStorageService"))
            && QUrl(cfg.read<QString>(QLatin1String("storageServiceAddress"))).isValid())
    {
        menu.addAction(tr("Play & Download"), this, SLOT(onPlayAndDownload()));
        menu.addAction(tr("Download"), this, SLOT(onDownload()));
    }
    menu.addAction(tr("Play"), this, SLOT(onPlay()));
    menu.addAction(tr("Copy Address"), this, SLOT(onCopyAddress()));
    menu.exec(globalPos);
}

void PlayDialog::onMarkAsAudioTrack()
{
    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Please select a media item in list to be marked as audio track."),
                             QMessageBox::Ok);
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    if (m_complexUrlResources)
    {
        auto audio = m_resultStreams[currentRow];
        if (!audio->maybeAudio() && QMessageBox::warning(this,
                                                            tr("Warning"),
                                                            tr("This media item may be not an audio track, continue anyway?"),
                                                            QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No)
        {
            return;
        }
        m_selectedAudio = audio;
    }
}

void PlayDialog::onUnmarkAsAudioTrack()
{
    m_selectedAudio.reset();
}

void PlayDialog::onPlayAndDownload()
{
    m_action = PlayDialog::PLAY_AND_DOWNLOAD;
    if (doOk())
        accept();
}

void PlayDialog::onDownload()
{
    m_action = PlayDialog::DOWNLOAD;
    if (doOk())
        accept();
}

void PlayDialog::onPlay()
{
    m_action = PlayDialog::PLAY;
    if (doOk())
        accept();
}

void PlayDialog::onCopyAddress()
{
    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem) {
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    if (m_complexUrlResources) {
        auto stream = m_resultStreams[currentRow];
        QApplication::clipboard()->setText(stream->urls.join('\n'));
    } else {
        QApplication::clipboard()->setText(m_resultUrls[currentRow]);
    }
}

void PlayDialog::on_cbAutoSelectAudioTrack_stateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        m_selectedAudio.reset();
    }
    else if (state == Qt::Checked)
    {
        auto it = std::find_if(m_resultStreams.rbegin(), m_resultStreams.rend(),
                               [](StreamInfoPtr stream){ return stream->maybeAudio() && stream->container.compare("m4a",  Qt::CaseInsensitive) == 0;}); // m4a is preferred first
        if (m_resultStreams.rend() != it)
        {
            m_selectedAudio = *it;
            return;
        }

        it = std::find_if(m_resultStreams.rbegin(), m_resultStreams.rend(),
                                       [](StreamInfoPtr stream){ return stream->maybeAudio();});
        if (m_resultStreams.rend() != it)
        {
            m_selectedAudio = *it;
            return;
        }
    }
}

void PlayDialog::on_cbSubtitles_currentIndexChanged(int )
{
    ui->cbSubtitleEnabled->setChecked(true);
}

void PlayDialog::on_cbAutoSelectHighestQualityVideoTrack_stateChanged(int state)
{
    Config cfg;
    cfg.write("autoSelectHighestQualityVideoTrack", state == Qt::Checked);
}

PlayDialog::Action PlayDialog::action() const
{
    return m_action;
}

void PlayDialog::on_btnPlayDownload_clicked()
{
    m_action = PlayDialog::PLAY_AND_DOWNLOAD;
    if (doOk())
        accept();
}

void PlayDialog::on_btnDownload_clicked()
{
    m_action = PlayDialog::DOWNLOAD;
    if (doOk())
        accept();
}

void PlayDialog::on_cbLuxResult_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
    }
    else
    {
    }
}

void PlayDialog::on_cbYKDL_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
    }
    else
    {
    }
}

void PlayDialog::on_cbYouGet_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
    }
    else
    {
    }
}

void PlayDialog::on_cbYoutubeDL_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
    }
    else
    {
    }
}
