#ifndef PLAYURLDIALOG_H
#define PLAYURLDIALOG_H

#include <QDialog>

namespace Ui {
class PlayUrlDialog;
}

class PlayUrlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayUrlDialog(QWidget *parent = 0);
    ~PlayUrlDialog();

    const QString& getUrl() const { return m_url; }
private slots:
    void on_btnPlay_clicked();

    void on_btnCancel_clicked();

private:
    Ui::PlayUrlDialog *ui;
    QString m_url;
};

#endif // PLAYURLDIALOG_H
