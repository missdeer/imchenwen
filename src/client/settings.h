#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtWidgets/QDialog>
#include "ui_settings.h"
#include "config.h"

class SettingsDialog : public QDialog, public Ui_Settings
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr);
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

    void onAddLiveTVItem();
    void onRemoveLiveTVItem();
    void onModifyLiveTVItem();
    void onImportLiveTVItems();
    void onExportLiveTVItems();
    void onCheckLiveTVItems();
    void onLiveTVTableItemSelectionChanged();

    void onAddVIPVideo();
    void onRemoveVIPVideo();
    void onModifyVIPVideo();
    void onImportVIPVideo();
    void onExportVIPVideo();
    void onVIPVideoTableItemSelectionChanged();
private:
    QFont standardFont;
    QFont fixedFont;
    Tuple2List m_players;
    Tuple3List m_liveTV;
    Tuple2List m_vipVideo;
    bool importLiveTVAsJSON(const QString& path);
    bool importLiveTVAsPlainText(const QString& path);
    void exportLiveTVAsJSON(const QString& path);
    void exportLiveTVAsPlainText(const QString& path);
    bool importVIPVideoAsJSON(const QString& path);
    bool importVIPVideoAsPlainText(const QString& path);
    void exportVIPVideoAsJSON(const QString& path);
    void exportVIPVideoAsPlainText(const QString& path);
    void fillLiveTVTable();
    void fillVIPVideoTable();
    void fillExternalPlayerTable();
};

#endif // SETTINGS_H
