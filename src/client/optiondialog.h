#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include "config.h"

namespace Ui {
class OptionDialog;
}

class OptionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OptionDialog(QWidget *parent = 0);
    ~OptionDialog();
    
private slots:
    void on_browseButton_clicked();

    void on_btnClose_clicked();

    void on_btnAddPlayer_clicked();

    void on_btnRemovePlayer_clicked();

    void on_btnModifyPlayer_clicked();

    void on_playerList_currentRowChanged(int currentRow);

private:
    Ui::OptionDialog *ui;
    Tuple2List m_players;
};

#endif // OPTIONDIALOG_H
