#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtWidgets/QDialog>
#include "ui_settings.h"
#include "config.h"
#include "player.h"

class SettingsDialog : public QDialog, public Ui_Settings
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = nullptr);
    void accept();
    void setCurrentPage(int index);
private slots:
    void onSetHomeToCurrentPage();
    void onChooseFont();
    void onChooseFixedFont();

    void onSelectExternalPlayer();
    void onAddExternalPlayer();
    void onRemoveExternalPlayer();
    void onModifyExternalPlayer();
    void onExternalPlayerListCurrentRowChanged(int row);

    void onBrowseYouGetPath();
    void onBrowseYKDLPath();
    void onBrowseYoutubeDLPath();
    void onBrowseAnniePath();
    void onBrowseFFmpegPath();

    void onAddLiveTVSubscription();
    void onRemoveLiveTVSubscription();
    void onLiveTVSubscriptionTableItemSelectionChanged();

    void onAddLiveTVItem();
    void onRemoveLiveTVItem();
    void onModifyLiveTVItem();
    void onImportLiveTVItems();
    void onExportLiveTVItems();
    void onLiveTVTableItemSelectionChanged();

    void onAddVIPVideoSubscription();
    void onRemoveVIPVideoSubscription();
    void onVIPVideoSubscriptionTableItemSelectionChanged();

    void onAddVIPVideo();
    void onRemoveVIPVideo();
    void onModifyVIPVideo();
    void onImportVIPVideo();
    void onExportVIPVideo();
    void onVIPVideoTableItemSelectionChanged();
private:
    QFont standardFont;
    QFont fixedFont;
    PlayerList m_players;
    Tuple2List m_liveTV;
    QStringList m_liveTVSubscription;
    Tuple2List m_vipVideo;
    QStringList m_vipVideoSubscription;
    bool importLiveTVAsJSON(const QString& path);
    bool importLiveTVAsPlainText(const QString& path);
    void exportLiveTVAsJSON(const QString& path);
    void exportLiveTVAsPlainText(const QString& path);
    bool importVIPVideoAsJSON(const QString& path);
    bool importVIPVideoAsPlainText(const QString& path);
    void exportVIPVideoAsJSON(const QString& path);
    void exportVIPVideoAsPlainText(const QString& path);
    void fillLiveTVTable();
    void fillLiveTVSubscriptionTable();
    void fillVIPVideoTable();
    void fillVIPVideoSubscriptionTable();
    void fillExternalPlayerTable();

    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();
    void showCookies();
    void showExceptions();
    void setupLiveTVTable();
    void setupVIPVideoTable();
};

#endif // SETTINGS_H
