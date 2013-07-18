#include "mainwindow.h"
#include <QUrl>
#include <QDesktopServices>
#include <QClipboard>
#include <QRegExp>
#include "optiondialog.h"
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

    createParseMenu();
    connect( QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(on_clipboard_dataChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createParseMenu()
{
    QMenu* menuParse = new QMenu(this);
    menuParse->addAction(ui->actionParseURL);
    menuParse->addAction(ui->actionParsePlaylist);
    ui->parseButton->setMenu(menuParse);
}

void MainWindow::on_clipboard_dataChanged()
{
    QString text =  QApplication::clipboard()->text();
    // regex match
    QRegExp regex("^http:\\/\\/v\\.qq\\.com.*");
    if (regex.exactMatch(text))
    {
        ui->urlEdit->setText(text);
    }
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionOption_triggered()
{
    OptionDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_actionContent_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.dfordsoft.com/imchenwen/features.htm"));
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

void MainWindow::on_actionParseURL_triggered()
{

}

void MainWindow::on_actionParsePlaylist_triggered()
{

}

void MainWindow::on_parseButton_clicked()
{

}

void MainWindow::on_searchButton_clicked()
{

}

