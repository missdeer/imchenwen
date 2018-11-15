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
    void setMediaInfo(const QString& title, const QString& url);
    PlayerPtr player() { return m_selectedPlayer; }
    StreamInfoPtr media() { return m_selectedMedia; }
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
    bool m_multiMediaResources;
    void createExternalPlayerList();
    void doOk();
    QListWidgetItem *addItem(const QIcon &icon, const QString& text, const QColor& backgroundColor);
};

#endif // EXTERNALPLAYDIALOG_H
