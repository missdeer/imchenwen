#include "inputurldialog.h"
#include "ui_inputurldialog.h"

InputUrlDialog::InputUrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputUrlDialog)
{
    ui->setupUi(this);
}

InputUrlDialog::~InputUrlDialog()
{
    delete ui;
}

QString InputUrlDialog::url()
{
    return ui->edtURL->text();
}

bool InputUrlDialog::playDirectly()
{
    return ui->btnPlay->isChecked();
}

bool InputUrlDialog::resolveThenPlay()
{
    return ui->btnResolve->isChecked();
}

bool InputUrlDialog::sniffThenPlay()
{
    return ui->btnSniff->isChecked();
}

bool InputUrlDialog::resolveAsVIPThenPlay()
{
    return ui->btnResolveAsVIP->isChecked();
}
