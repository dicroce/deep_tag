#ifndef CREATERECTDIALOG_H
#define CREATERECTDIALOG_H

#include <QDialog>

namespace Ui {
class CreateRectDialog;
}

class CreateRectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateRectDialog(QWidget *parent = nullptr);
    ~CreateRectDialog();

    bool accepted(){return _accepted;}

private slots:
    void accept(){_accepted = true; close(); };

private:
    Ui::CreateRectDialog *ui;
    bool _accepted;
};

#endif // CREATERECTDIALOG_H
