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

private:
    Ui::CreateRectDialog *ui;
};

#endif // CREATERECTDIALOG_H
