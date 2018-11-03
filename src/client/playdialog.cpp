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
    } ss[] = {
        { mi->ykdl,       tr(" - by ykdl")},
        { mi->you_get,    tr(" - by you-get")},
        { mi->youtube_dl, tr(" - by youtube_dl")},
        { mi->annie,      tr(" - by annie")},
    };
    for (auto & s : ss)
    {
        for (auto& stream : s.streams)
        {
            addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality + s.tag,
                    ui->listMedia->count() % 2 ? Qt::white : QColor(0xf0f0f0));
        }
        m_streams.append(s.streams);
    }
    ui->listMedia->setCurrentRow(0);
    m_mediaInfo = mi;
    m_multiMediaResources = true;
}

void PlayDialog::setMediaInfo(const QString &title, const QString &url)
{
    addItem(title + "\n" + url, Qt::white);
    ui->listMedia->setCurrentRow(0);
    m_multiMediaResources = false;
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
    m_players.push_back(std::make_tuple(tr("Built-in player"), ""));
    Config cfg;
    cfg.read("externalPlayers", m_players);
    auto dlnaRenderers = Browser::instance().kast().getRenderers();
    for (auto& r : dlnaRenderers)
    {
        m_players.push_back(std::make_tuple(tr("DLNA:"), r));
    }
    for (auto& p : m_players)
    {
        ui->cbPlayers->addItem(std::get<0>(p) + " " + std::get<1>(p));
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
    if (QFile::exists(std::get<0>(m_selectedPlayer)) || currentIndex == 0)
        accept();
    else
        QMessageBox::warning(this, tr("Error"), tr("Cannot find player at '%1', please reconfiguration it.").arg(std::get<0>(m_selectedPlayer)), QMessageBox::Ok);
}

void PlayDialog::addItem(const QString& text, const QColor &backgroundColor)
{
    QListWidgetItem *item = new QListWidgetItem(text, ui->listMedia);
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
}

void PlayDialog::on_listMedia_itemActivated(QListWidgetItem *)
{
    doOk();
}
