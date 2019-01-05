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
            if (!m_demuxed)
            {
                if (stream->quality.contains("audio only")
                        || stream->quality.contains("audio/webm")
                        || stream->quality.contains("audio/mp4"))
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
    ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_complexUrlResources = true;
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

bool PlayDialog::uploadToStorageService()
{
    return ui->btnStorage->isChecked();
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
    doOk();
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
            ui->cbPlayers->addItem(QIcon(":/player/external.png"), p->name() + " " + p->arguments());
            break;
        }
    }
    int index = cfg.read<int>("selectedPlayerIndex", 0);
    if (index >= ui->cbPlayers->count())
        index = 0;
    ui->cbPlayers->setCurrentIndex(index);
}

void PlayDialog::doOk()
{
    int currentIndex = ui->cbPlayers->currentIndex();
    m_selectedPlayer = m_players.at(currentIndex);

    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select a media item in list to be played."), QMessageBox::Ok);
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    if (m_complexUrlResources)
    {
        m_selectedVideo = m_resultStreams[currentRow];
    }
    else
    {
        m_selectedUrl = m_resultUrls[currentRow];
    }
    if ((m_selectedPlayer->type() == Player::PT_EXTERNAL && QFile::exists(m_selectedPlayer->name())) || m_selectedPlayer->type() != Player::PT_EXTERNAL)
    {
        if (m_demuxed && !m_selectedAudio)
        {
            if (QMessageBox::question(this,
                                            tr("Confirm"),
                                            tr("No stream is marked as audio track, continue anyway?"),
                                            QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No)
                return;
        }
        Config cfg;
        cfg.write("selectedPlayerIndex", currentIndex);
        accept();
    }
    else
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Cannot find player at '%1', please reconfiguration it.").arg(m_selectedPlayer->name()),
                             QMessageBox::Ok);
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

void PlayDialog::on_listMedia_itemActivated(QListWidgetItem *)
{
    doOk();
}

void PlayDialog::onListMediaContextmenu(const QPoint &pos)
{
    QPoint globalPos = ui->listMedia->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction(tr("Mark as Audio Track"), this, SLOT(onMarkAsAudioTrack()));
    myMenu.addAction(tr("Unmark as Audio Track"),  this, SLOT(onUnmarkAsAudioTrack()));
    myMenu.exec(globalPos);
}

void PlayDialog::onMarkAsAudioTrack()
{
    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select a media item in list to be marked as audio track."), QMessageBox::Ok);
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    if (m_complexUrlResources)
    {
        m_selectedAudio = m_resultStreams[currentRow];
    }
}

void PlayDialog::onUnmarkAsAudioTrack()
{
    m_selectedAudio.reset();
}
