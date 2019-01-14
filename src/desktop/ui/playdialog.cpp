#include "playdialog.h"
#include "ui_playdialog.h"
#include "settings.h"
#include "browser.h"
#include <QtCore>
#include <QMessageBox>
#include <QFile>
#include <QMenu>

PlayDialog::PlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayDialog),
    m_complexUrlResources(false),
    m_demuxed(false)
{
    ui->setupUi(this);

    ui->listMedia->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listMedia, &QListWidget::customContextMenuRequested, this, &PlayDialog::onListMediaContextmenu);

    createExternalPlayerList();
    Config cfg;
    ui->cbAutoSelectHighestQualityVideoTrack->setChecked(cfg.read<bool>("autoSelectHighestQualityVideoTrack", true));
    ui->cbUploadToStorageService->setChecked(cfg.read<bool>("uploadToStorageService", false));
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

    ui->labelSubtitle->setVisible(!!ui->cbSubtitles->count());
    ui->cbSubtitles->setVisible(!!ui->cbSubtitles->count());
    ui->cbSubtitleEnabled->setVisible(!!ui->cbSubtitles->count());

    struct {
        Streams& streams;
        QString tag;
        QIcon icon;
    } ss[] = {
        { mi->youtube_dl, tr(" - by youtube_dl"), QIcon(":/youtube-dl.png")},
        { mi->ykdl,       tr(" - by ykdl"),       QIcon(":/ykdl.png")},
        { mi->you_get,    tr(" - by you-get"),    QIcon(":/you-get.png")},
        { mi->annie,      tr(" - by annie"),      QIcon(":/annie.png")},
    };
    for (auto & s : ss)
    {
        for (auto stream : s.streams)
        {
            if (stream->urls.isEmpty())
                continue;

            QString itemText = mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality + s.tag;
            auto items = ui->listMedia->findItems(itemText, Qt::MatchExactly);
            if (!items.isEmpty())
                break;
            if (!m_demuxed && maybeAudioTrack(stream))
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
    ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_complexUrlResources = false;
}

QString PlayDialog::audioUrl()
{
    if (m_selectedAudio && !m_selectedAudio->urls.isEmpty())
        return m_selectedAudio->urls[0];
    return "";
}

bool PlayDialog::uploadToStorageService()
{
    return ui->cbUploadToStorageService->isChecked();
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
    if (doOk())
        accept();
}

void PlayDialog::on_btnCancel_clicked()
{
    reject();
}

void PlayDialog::createExternalPlayerList()
{
    m_players.clear();
    PlayerPtr builtinPlayer(new Player(Player::PT_BUILTIN, tr("Built-in player")));
    m_players.push_back(builtinPlayer);
    Config cfg;
    cfg.read("externalPlayers", m_players);
    auto dlnaRenderers = Browser::instance().kast().getRenderers();
    for (auto& r : dlnaRenderers)
    {
        PlayerPtr p(new Player(Player::PT_DLNA, r));
        m_players.push_back(p);
    }
    for (auto& p : m_players)
    {
        switch (p->type())
        {
        case Player::PT_BUILTIN:
            ui->cbPlayers->addItem(QIcon(":/player/builtin.png"), p->name());
            break;
        case Player::PT_DLNA:
            ui->cbPlayers->addItem(QIcon(":/player/dlna.png"), tr("DLNA: ") + p->name());
            break;
        case Player::PT_EXTERNAL:
            ui->cbPlayers->addItem(QIcon(":/player/external.png"), p->name()/* + " " + p->arguments()*/);
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
            auto it = std::max_element(m_resultStreams.begin(), m_resultStreams.end());
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
        if (maybeAudioTrack(video) && QMessageBox::warning(this,
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
    if ((m_selectedPlayer->type() == Player::PT_EXTERNAL
         && QFile::exists(m_selectedPlayer->name()))
            || m_selectedPlayer->type() != Player::PT_EXTERNAL)
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

    QMessageBox::warning(this,
                         tr("Error"),
                         tr("Cannot find player at '%1', please reconfiguration it.").arg(m_selectedPlayer->name()),
                         QMessageBox::Ok);
    return false;
}

QListWidgetItem * PlayDialog::addItem(const QIcon& icon, const QString& text, const QColor &backgroundColor)
{
    QListWidgetItem *item = new QListWidgetItem(icon, text, ui->listMedia);
    QFont font(item->font());
#if defined(Q_OS_WIN)
    font.setFamily("Microsoft YaHei");
#elif defined(Q_OS_MAC)
    font.setFamily("Pingfang CS");
#endif
    font.setPixelSize(14);
    item->setFont(font);
    item->setBackgroundColor(backgroundColor);
    ui->listMedia->addItem(item);
    return item;
}

bool PlayDialog::maybeAudioTrack(StreamInfoPtr media)
{
    return (media->quality.contains("audio only")
            || media->quality.contains("audio/"));
}

void PlayDialog::on_listMedia_itemActivated(QListWidgetItem *)
{
    if (doOk())
        accept();
}

void PlayDialog::onListMediaContextmenu(const QPoint &pos)
{
    QPoint globalPos = ui->listMedia->mapToGlobal(pos);

    QMenu menu;
    menu.addAction(tr("Mark as Audio Track"), this, SLOT(onMarkAsAudioTrack()));
    menu.addAction(tr("Unmark as Audio Track"),  this, SLOT(onUnmarkAsAudioTrack()));
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
        if (!maybeAudioTrack(audio) && QMessageBox::warning(this,
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

void PlayDialog::on_cbAutoSelectAudioTrack_stateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        m_selectedAudio.reset();
    }
    else if (state == Qt::Checked)
    {
        auto it = std::find_if(m_resultStreams.rbegin(), m_resultStreams.rend(),
                               [this](StreamInfoPtr stream){ return maybeAudioTrack(stream);});
        if (m_resultStreams.rend() != it)
            m_selectedAudio = *it;
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

void PlayDialog::on_cbUploadToStorageService_stateChanged(int state)
{
    Config cfg;
    cfg.write("uploadToStorageService", state == Qt::Checked);
}
