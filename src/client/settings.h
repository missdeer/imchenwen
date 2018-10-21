#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtWidgets/QDialog>
#include "ui_settings.h"
#include "config.h"

class SettingsDialog : public QDialog, public Ui_Settings
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
    void accept();
    void setCurrentPage(int index);
private slots:
    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();

    void setHomeToCurrentPage();
    void showCookies();
    void showExceptions();

    void chooseFont();
    void chooseFixedFont();

    void on_httpUserAgent_editingFinished();
    void on_httpAcceptLanguage_editingFinished();

    void onSelectExternalPlayer();
    void onAddExternalPlayer();
    void onRemoveExternalPlayer();
    void onModifyExternalPlayer();
    void onExternalPlayerListCurrentRowChanged(int row);

    void onBrowseYouGetPath();
    void onBrowseYKDLPath();
    void onBrowseYoutubeDLPath();
    void onBrowseAnniePath();
private:
    QFont standardFont;
    QFont fixedFont;
    Tuple2List m_players;
};

#endif // SETTINGS_H
