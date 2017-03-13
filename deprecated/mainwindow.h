#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

QT_BEGIN_NAMESPACE
class QClipboard;
QT_END_NAMESPACE

class HistoryModel;
class NavigatorModel;
class ResourceModel;
class PlaylistModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_clipboard_dataChanged();

    void on_searchButton_clicked();

    void on_actionExit_triggered();

    void on_actionOption_triggered();

    void on_actionContent_triggered();

    void on_actionHomepage_triggered();

    void on_actionBuy_triggered();

    void on_actionAbout_triggered();

    void on_actionParseURL_triggered();

    void on_actionParsePlaylist_triggered();

private:
    Ui::MainWindow *ui;
    HistoryModel* historyModel_;
    NavigatorModel* navigatorModel_;
    ResourceModel* resourceModel_;
    PlaylistModel* playlistModel_;

    void createParseMenu();
};

#endif // MAINWINDOW_H
