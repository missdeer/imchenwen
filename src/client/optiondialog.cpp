#include "optiondialog.h"
#include "ui_optiondialog.h"
#include <QFileDialog>
#include <QMessageBox>

OptionDialog::OptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());

    Config cfg;
    cfg.read("externalPlayers", m_players);
    for (auto p : m_players)
    {
        ui->playerList->addItem(std::get<0>(p) + "\n" + std::get<1>(p));
    }
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::on_browseButton_clicked()
{
    QString mediaPlayerPath = QFileDialog::getOpenFileName(this,
                                 tr("select media player executable"));
    if (!mediaPlayerPath.isEmpty())
    {
        ui->playerPathEdit->setText(mediaPlayerPath);
        ui->playerArgumentsEdit->setText("");
    }
}

void OptionDialog::on_btnClose_clicked()
{
    accept();
}

void OptionDialog::on_btnAddPlayer_clicked()
{
    if (ui->playerPathEdit->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input player path."), QMessageBox::Ok);
        return;
    }
    auto it = std::find_if(m_players.begin(), m_players.end(),
                           [this](const Tuple2& t) { return std::get<0>(t) == ui->playerPathEdit->text() && std::get<1>(t) == ui->playerArgumentsEdit->text();});
    if (m_players.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This configuration item exists already."), QMessageBox::Ok);
        return;
    }
    m_players.push_back(std::make_tuple(ui->playerPathEdit->text(), ui->playerArgumentsEdit->text()));
    Config cfg;
    cfg.write("externalPlayers", m_players);
    ui->playerList->addItem(ui->playerPathEdit->text() + "\n" + ui->playerArgumentsEdit->text());
    ui->playerPathEdit->setText("");
    ui->playerArgumentsEdit->setText("");
}

void OptionDialog::on_btnRemovePlayer_clicked()
{
    QListWidgetItem *currentItem = ui->playerList->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select an item in list to be removed."), QMessageBox::Ok);
        return;
    }
    int currentRow = ui->playerList->currentRow();
    m_players.removeAt(currentRow);
    Config cfg;
    cfg.write("externalPlayers", m_players);
    currentItem = ui->playerList->takeItem(currentRow);
    delete currentItem;
}

void OptionDialog::on_btnModifyPlayer_clicked()
{
    QListWidgetItem *currentItem = ui->playerList->currentItem();
    if (!currentItem)
    {
        return;
    }
    int currentRow = ui->playerList->currentRow();
    m_players[currentRow] = std::make_tuple(ui->playerPathEdit->text(), ui->playerArgumentsEdit->text());
    currentItem->setText(ui->playerPathEdit->text() + "\n" + ui->playerArgumentsEdit->text());
    Config cfg;
    cfg.write("externalPlayers", m_players);
}

void OptionDialog::on_playerList_currentRowChanged(int currentRow)
{
    if (currentRow < 0 && currentRow >= m_players.size())
        return;

    const Tuple2& p = m_players.at(currentRow);
    ui->playerPathEdit->setText(std::get<0>(p));
    ui->playerArgumentsEdit->setText(std::get<1>(p));
}
