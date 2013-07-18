#include <QFileDialog>
#include "optiondialog.h"
#include "ui_optiondialog.h"

OptionDialog::OptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::on_buttonBox_accepted()
{

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
