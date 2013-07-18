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
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 0);
    ui->splitter->setStretchFactor(2, 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_parseButton_clicked()
{

}

void MainWindow::on_searchButton_clicked()
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
