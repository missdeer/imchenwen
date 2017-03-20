#include "externalplaydialog.h"
#include "ui_externalplaydialog.h"
#include "optiondialog.h"
#include <QMessageBox>

ExternalPlayDialog::ExternalPlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExternalPlayDialog)
{
    ui->setupUi(this);

    createExternalPlayerList();
}

ExternalPlayDialog::~ExternalPlayDialog()
{
    delete ui;
}

void ExternalPlayDialog::setMediaInfo(MediaInfoPtr mi)
{
    for (auto stream : mi->ykdl)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/);
    }
    for (auto stream : mi->you_get)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/);
    }
    for (auto stream : mi->youtube_dl)
    {
        addItem(mi->title + "\n" + mi->site + " - " + stream->container + " - " + stream->quality/*  + "\n"+ stream->urls.join("\n")*/);
    }
    ui->listMedia->setCurrentRow(0);
    m_mediaInfo = mi;
}

void ExternalPlayDialog::on_btnExternalPlayerConfiguration_clicked()
{
    OptionDialog dlg(this);
    if (dlg.exec())
    {
        ui->cbPlayers->clear();
        createExternalPlayerList();
    }
}

void ExternalPlayDialog::on_btnPlay_clicked()
{
    doOk();
}

void ExternalPlayDialog::on_btnCancel_clicked()
{
    close();
}

void ExternalPlayDialog::createExternalPlayerList()
{
    m_players.clear();
    Config cfg;
    cfg.read("externalPlayers", m_players);
    for (auto p : m_players)
    {
        ui->cbPlayers->addItem(std::get<0>(p) + " " + std::get<1>(p));
    }
    if (!m_players.isEmpty())
    {
        ui->cbPlayers->setCurrentIndex(0);
    }
}

void ExternalPlayDialog::doOk()
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
    else
        m_selectedMedia = m_mediaInfo->youtube_dl[currentRow - m_mediaInfo->ykdl.length() - m_mediaInfo->you_get.length()];

    int currentIndex = ui->cbPlayers->currentIndex();
    m_selectedPlayer = m_players.at(currentIndex);
    accept();
}

void ExternalPlayDialog::addItem(const QString& text)
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
    ui->listMedia->addItem(item);
}

void ExternalPlayDialog::on_listMedia_itemActivated(QListWidgetItem *)
{
    doOk();
}
