#include "mainwindow.h"
#include <QUrl>
#include <QDesktopServices>
#include "aboutdialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_goButton_clicked()
{

}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionOption_triggered()
{

}

void MainWindow::on_actionContent_triggered()
{

}

void MainWindow::on_actionHomepage_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.dfordsoft.com/imchenwen/"));
}

void MainWindow::on_actionBuy_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.dfordsoft.com/imchenwen/buy.htm"));
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}
