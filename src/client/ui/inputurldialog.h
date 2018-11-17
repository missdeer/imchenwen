#ifndef INPUTURLDIALOG_H
#define INPUTURLDIALOG_H

#include <QDialog>

namespace Ui {
class InputUrlDialog;
}

class InputUrlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputUrlDialog(QWidget *parent = nullptr);
    ~InputUrlDialog();

    QString url();
    bool playDirectly();
    bool resolveThenPlay();
private:
    Ui::InputUrlDialog *ui;
};

#endif // INPUTURLDIALOG_H
