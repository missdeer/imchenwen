#include "donatedialog.h"
#include "ui_donatedialog.h"

DonateDialog::DonateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DonateDialog)
{
    ui->setupUi(this);

    loadImages();
}

DonateDialog::~DonateDialog()
{
    delete ui;
}

void DonateDialog::loadImages()
{
    QPixmap alipay;
    alipay.load(":/qrcode/alipay.jpg");
    ui->alipay->setPixmap(alipay.scaled(340, 340,Qt::KeepAspectRatio));

    QPixmap wepay;
    wepay.load(":/qrcode/wepay.jpg");
    ui->wepay->setPixmap(wepay.scaled(340, 340,Qt::KeepAspectRatio));
}

