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
    void setMediaInfo(MediaInfoPtr mi);
    void setMediaInfo(const QString &title, const QStringList &urls);
    PlayerPtr player() { return m_selectedPlayer; }
    StreamInfoPtr media() { return m_selectedMedia; }
    QString url() { return m_selectedUrl; }
    bool uploadToStorageService();
private slots:
    void on_btnExternalPlayerConfiguration_clicked();

    void on_btnPlay_clicked();

    void on_btnCancel_clicked();

    void on_listMedia_itemActivated(QListWidgetItem *);

private:
    Ui::PlayDialog *ui;
    PlayerList m_players;
    PlayerPtr m_selectedPlayer;
    StreamInfoPtr m_selectedMedia;
    MediaInfoPtr m_mediaInfo;
    Streams m_streams;
    QStringList m_urls;
    QString m_selectedUrl;
    bool m_complexUrlResources;
    void createExternalPlayerList();
    void doOk();
    QListWidgetItem *addItem(const QIcon &icon, const QString& text, const QColor& backgroundColor);
};

#endif // EXTERNALPLAYDIALOG_H
