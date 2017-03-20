#include "playurldialog.h"
#include "ui_playurldialog.h"

PlayUrlDialog::PlayUrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayUrlDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());
}

PlayUrlDialog::~PlayUrlDialog()
{
    delete ui;
}

void PlayUrlDialog::on_btnPlay_clicked()
{
    m_url = ui->lineEdit->text();
    accept();
}

void PlayUrlDialog::on_btnCancel_clicked()
{
    reject();
}
