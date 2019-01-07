#ifndef EXTERNALPLAYDIALOG_H
#define EXTERNALPLAYDIALOG_H

#include <QDialog>
#include "config.h"
#include "linkresolver.h"
#include "player.h"

QT_BEGIN_NAMESPACE
class QListWidgetItem;
QT_END_NAMESPACE

namespace Ui {
class PlayDialog;
}

class PlayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayDialog(QWidget *parent = nullptr);
    ~PlayDialog();
    void setMediaInfo(const QString &originalUrl, MediaInfoPtr mi);
    void setMediaInfo(const QString &originalUrl, const QString &title, const QStringList &urls);
    PlayerPtr player() { return m_selectedPlayer; }
    StreamInfoPtr video() { return m_selectedVideo; }
    QString audioUrl();
    QString subtitleUrl() { return m_subtitleUrl; }
    QString videoUrl() { return m_selectedUrl; }
    bool uploadToStorageService();
private slots:
    void on_btnExternalPlayerConfiguration_clicked();

    void on_btnPlay_clicked();

    void on_btnCancel_clicked();

    void on_listMedia_itemActivated(QListWidgetItem *);

    void onListMediaContextmenu(const QPoint&);
    void onMarkAsAudioTrack();
    void onUnmarkAsAudioTrack();
private:
    Ui::PlayDialog *ui;
    PlayerList m_players;
    PlayerPtr m_selectedPlayer;
    StreamInfoPtr m_selectedVideo;
    StreamInfoPtr m_selectedAudio;
    Streams m_resultStreams;
    QStringList m_resultUrls;
    QString m_selectedUrl;
    QString m_originalUrl;
    QString m_subtitleUrl;
    bool m_complexUrlResources;
    bool m_demuxed;
    void createExternalPlayerList();
    void doOk();
    QListWidgetItem *addItem(const QIcon &icon, const QString& text, const QColor& backgroundColor);
    bool maybeAudioTrack(StreamInfoPtr media);
};

#endif // EXTERNALPLAYDIALOG_H
