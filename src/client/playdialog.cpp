#include "playdialog.h"
#include "ui_playdialog.h"
#include "settings.h"
#include <QMessageBox>

PlayDialog::PlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayDialog)
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
    for (auto stream : mi->ykdl)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/, Qt::white);
    }
    for (auto stream : mi->you_get)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/, QColor(0xf0f0f0));
    }
    for (auto stream : mi->youtube_dl)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/, Qt::white);
    }
    for (auto stream : mi->vip)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/, Qt::white);
    }
    ui->listMedia->setCurrentRow(0);
    m_mediaInfo = mi;
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
    Config cfg;
    cfg.read("externalPlayers", m_players);
    m_players.push_back(std::make_tuple(tr("Built-in player"), ""));
    for (auto p : m_players)
    {
        ui->cbPlayers->addItem(std::get<0>(p) + " " + std::get<1>(p));
    }
    if (!m_players.isEmpty())
    {
        ui->cbPlayers->setCurrentIndex(0);
    }
}

void PlayDialog::doOk()
{
    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select a media item in list to be played."), QMessageBox::Ok);
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    if (currentRow < m_mediaInfo->ykdl.length())
        m_selectedMedia = m_mediaInfo->ykdl[currentRow];
    else if (currentRow < m_mediaInfo->ykdl.length() + m_mediaInfo->you_get.length())
        m_selectedMedia = m_mediaInfo->you_get[currentRow - m_mediaInfo->ykdl.length()];
    else if (currentRow < m_mediaInfo->ykdl.length() + m_mediaInfo->you_get.length() + m_mediaInfo->youtube_dl.length())
        m_selectedMedia = m_mediaInfo->youtube_dl[currentRow - (m_mediaInfo->ykdl.length() + m_mediaInfo->you_get.length())];
    else
        m_selectedMedia = m_mediaInfo->vip[currentRow - (m_mediaInfo->ykdl.length() + m_mediaInfo->you_get.length() + m_mediaInfo->youtube_dl.length())];

    int currentIndex = ui->cbPlayers->currentIndex();
    if (currentIndex == m_players.length() - 1)
    {
#if defined(Q_OS_WIN)
        m_selectedPlayer = std::make_tuple(QApplication::applicationDirPath() + "/mpv.exe", "--vo=direct3d --hwdec=dxva2 --ytdl=no");
#elif defined(Q_OS_MAC)
        m_selectedPlayer = std::make_tuple("/usr/local/bin/mpv", "--vo=opengl --hwdec=videotoolbox --ytdl=no");
#endif
    }
    else
        m_selectedPlayer = m_players.at(currentIndex);
    if (QFile::exists(std::get<0>(m_selectedPlayer)))
        accept();
    else
        QMessageBox::warning(this, tr("Error"), tr("Cannot find builtin player, please select an external player."), QMessageBox::Ok);
}

void PlayDialog::addItem(const QString& text, const QColor &backgroundColor)
{
    QListWidgetItem* item = new QListWidgetItem(text, ui->listMedia);
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
