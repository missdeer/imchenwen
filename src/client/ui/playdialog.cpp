#include "playdialog.h"
#include "ui_playdialog.h"
#include "settings.h"
#include "browser.h"
#include <QtCore>
#include <QMessageBox>
#include <QFile>

PlayDialog::PlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayDialog),
    m_multiMediaResources(false)
{
    ui->setupUi(this);

    createExternalPlayerList();
}

PlayDialog::~PlayDialog()
{
    delete ui;
}

void PlayDialog::setMediaInfo(MediaInfoPtr mi)
{
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
        for (auto& stream : s.streams)
        {
            auto item = addItem(s.icon,
                                mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality + s.tag,
                                ui->listMedia->count() % 2 ? Qt::white : QColor(0xf0f0f0));
            if (stream->urls.length() > 1)
                item->setToolTip(QString("%1 x %2").arg(QUrl(stream->urls[0]).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery)).arg(stream->urls.length()));
            else
                item->setToolTip(QUrl(stream->urls[0]).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery));
        }
        m_streams.append(s.streams);
    }
    ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_mediaInfo = mi;
    m_multiMediaResources = true;
}

void PlayDialog::setMediaInfo(const QString &title, const QString &url)
{
    ui->listMedia->clear();
    auto item = addItem(QIcon(":/video.png"), title + "\n" + url, Qt::white);
    ui->listMedia->setCurrentRow(0);
    ui->listMedia->setIconSize(QSize(40, 40));
    m_multiMediaResources = false;
    item->setToolTip(QUrl(url).toString(QUrl::RemoveAuthority | QUrl::RemoveQuery));
}

bool PlayDialog::uploadToStorageService()
{
    return ui->btnStorage->isChecked();
}

void PlayDialog::on_btnExternalPlayerConfiguration_clicked()
{
    SettingsDialog dlg(this);
    dlg.setCurrentPage(5);
    if (dlg.exec())
    {
        ui->cbPlayers->clear();
        createExternalPlayerList();
    }
}

void PlayDialog::on_btnPlay_clicked()
{
    doOk();
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
            ui->cbPlayers->addItem(p->name());
            break;
        case Player::PT_DLNA:
            ui->cbPlayers->addItem(tr("DLNA: ") + p->name());
            break;
        case Player::PT_EXTERNAL:
            ui->cbPlayers->addItem(p->name() + " " + p->arguments());
            break;
        }
    }
    ui->cbPlayers->setCurrentIndex(0);
}

void PlayDialog::doOk()
{
    int currentIndex = ui->cbPlayers->currentIndex();
    m_selectedPlayer = m_players.at(currentIndex);

    if (m_multiMediaResources)
    {
        QListWidgetItem *currentItem = ui->listMedia->currentItem();
        if (!currentItem)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please select a media item in list to be played."), QMessageBox::Ok);
            return;
        }
        int currentRow = ui->listMedia->currentRow();
        m_selectedMedia = m_streams[currentRow];
    }
    if ((m_selectedPlayer->type() == Player::PT_EXTERNAL && QFile::exists(m_selectedPlayer->name())) || m_selectedPlayer->type() != Player::PT_EXTERNAL)
        accept();
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
