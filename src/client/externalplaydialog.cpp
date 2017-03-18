#include "externalplaydialog.h"
#include "ui_externalplaydialog.h"
#include "optiondialog.h"
#include <QMessageBox>

ExternalPlayDialog::ExternalPlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExternalPlayDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());

    createExternalPlayerList();
}

ExternalPlayDialog::~ExternalPlayDialog()
{
    delete ui;
}

void ExternalPlayDialog::setStreams(const Streams& streams)
{
    for (auto stream : streams)
    {
        ui->listMedia->addItem(stream->container + "\n" + stream->quality /*+ "\n" + stream->urls.join("\n")*/);
    }
    ui->listMedia->setCurrentRow(0);
    m_streams = streams;
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
    QListWidgetItem *currentItem = ui->listMedia->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select a media item in list to be played."), QMessageBox::Ok);
        return;
    }
    int currentRow = ui->listMedia->currentRow();
    m_selectedMedia = m_streams[currentRow];

    int currentIndex = ui->cbPlayers->currentIndex();
    m_selectedPlayer = m_players.at(currentIndex);
    accept();
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
