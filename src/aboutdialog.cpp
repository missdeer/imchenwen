#include "aboutdialog.h"
#include <QLayout>
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setFixedSize( width(), height());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
