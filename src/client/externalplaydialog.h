#ifndef EXTERNALPLAYDIALOG_H
#define EXTERNALPLAYDIALOG_H

#include <QDialog>
#include "config.h"
#include "linkresolver.h"

QT_BEGIN_NAMESPACE
class QListWidgetItem;
QT_END_NAMESPACE

namespace Ui {
class ExternalPlayDialog;
}

class ExternalPlayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExternalPlayDialog(QWidget *parent = 0);
    ~ExternalPlayDialog();
    void setStreams(const Streams& streams);
    Tuple2 player() { return m_selectedPlayer; }
    StreamInfoPtr media() { return m_selectedMedia; }
private slots:
    void on_btnExternalPlayerConfiguration_clicked();

    void on_btnPlay_clicked();

    void on_btnCancel_clicked();

    void on_listMedia_itemActivated(QListWidgetItem *);

private:
    Ui::ExternalPlayDialog *ui;
    Tuple2List m_players;
    Tuple2 m_selectedPlayer;
    StreamInfoPtr m_selectedMedia;
    Streams m_streams;
    void createExternalPlayerList();
    void doOk();
};

#endif // EXTERNALPLAYDIALOG_H
