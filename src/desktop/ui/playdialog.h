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
    enum Action {
        PLAY_AND_DOWNLOAD,
        PLAY,
        DOWNLOAD,
        CANCEL,
    };
    explicit PlayDialog(QWidget *parent = nullptr);
    ~PlayDialog();
    void setMediaInfo(const QString &originalUrl, MediaInfoPtr mi);
    void setMediaInfo(const QString &originalUrl, const QString &title, const QStringList &urls);
    PlayerPtr player() { return m_selectedPlayer; }
    StreamInfoPtr video() { return m_selectedVideo; }
    StreamInfoPtr audio() { return m_selectedAudio; }
    QString audioUrl();
    QString subtitleUrl() { return m_subtitleUrl; }
    QString videoUrl() { return m_selectedUrl; }
    Action action() const;

private slots:
    void onListMediaContextmenu(const QPoint&);
    void onMarkAsAudioTrack();
    void onUnmarkAsAudioTrack();

    void on_btnExternalPlayerConfiguration_clicked();
    void on_btnPlay_clicked();
    void on_btnCancel_clicked();
    void on_btnPlayDownload_clicked();
    void on_btnDownload_clicked();
    void on_listMedia_itemActivated(QListWidgetItem *);
    void on_cbAutoSelectAudioTrack_stateChanged(int state);
    void on_cbSubtitles_currentIndexChanged(int);
    void on_cbAutoSelectHighestQualityVideoTrack_stateChanged(int state);

private:
    Ui::PlayDialog *ui;
    PlayerList m_players;
    PlayerPtr m_selectedPlayer;
    StreamInfoPtr m_selectedVideo;
    StreamInfoPtr m_selectedAudio;
    Streams m_resultStreams;
    Subtitles m_subtitles;
    QStringList m_resultUrls;
    QString m_selectedUrl;
    QString m_originalUrl;
    QString m_subtitleUrl;
    bool m_complexUrlResources;
    bool m_demuxed;
    Action m_action;
    void createExternalPlayerList();
    bool doOk();
    QListWidgetItem *addItem(const QIcon &icon, const QString& text, const QColor& backgroundColor);
    bool maybeAudioTrack(StreamInfoPtr media);
};

#endif // EXTERNALPLAYDIALOG_H
