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

    void onTabWidgetCurrentChanged(int index);
private:
    QFont standardFont;
    QFont fixedFont;
    PlayerList m_players;
    Tuple2List m_liveTV;
    QStringList m_liveTVSubscription;
    bool importLiveTVAsJSON(const QString& path);
    bool importLiveTVAsPlainText(const QString& path);
    void exportLiveTVAsJSON(const QString& path);
    void exportLiveTVAsPlainText(const QString& path);
    void fillLiveTVTable();
    void fillLiveTVSubscriptionTable();
    void fillExternalPlayerTable();

    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();
    void showCookies();
    void showExceptions();
    void setupLiveTVTable();
};

#endif // SETTINGS_H
