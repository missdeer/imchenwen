#ifndef EXTERNALPLAYDIALOG_H
#define EXTERNALPLAYDIALOG_H

#include <QDialog>
#include "config.h"
#include "linkresolver.h"

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
    explicit PlayDialog(QWidget *parent = 0);
    ~PlayDialog();
    void setMediaInfo(MediaInfoPtr mi);
    Tuple2 player() { return m_selectedPlayer; }
    StreamInfoPtr media() { return m_selectedMedia; }
private slots:
    void on_btnExternalPlayerConfiguration_clicked();

    void on_btnPlay_clicked();

    void on_btnCancel_clicked();

    void on_listMedia_itemActivated(QListWidgetItem *);

private:
    Ui::PlayDialog *ui;
    Tuple2List m_players;
    Tuple2 m_selectedPlayer;
    StreamInfoPtr m_selectedMedia;
    MediaInfoPtr m_mediaInfo;
    void createExternalPlayerList();
    void doOk();
    void addItem(const QString& text, const QColor& backgroundColor);
};

#endif // EXTERNALPLAYDIALOG_H
