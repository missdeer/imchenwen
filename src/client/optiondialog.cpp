#include <QFileDialog>
#include "config.h"
#include "optiondialog.h"
#include "ui_optiondialog.h"

OptionDialog::OptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());

    Config cfg;
    ui->playerPathEdit->setText(cfg.read<QString>("playerPath"));
    ui->playerArgumentsEdit->setText(cfg.read<QString>("playerArguments"));
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::on_buttonBox_accepted()
{
    Config cfg;
    cfg.write("playerPath", ui->playerPathEdit->text());
    cfg.write("playerArguments", ui->playerArgumentsEdit->text());
}

void OptionDialog::on_browseButton_clicked()
{
    QString mediaPlayerPath = QFileDialog::getOpenFileName(this,
                                 tr("select media player executable"));
    if (!mediaPlayerPath.isEmpty())
    {
        ui->playerPathEdit->setText(mediaPlayerPath);
    }
}
