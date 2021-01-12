#include "createrectdialog.h"
#include "ui_createrectdialog.h"

CreateRectDialog::CreateRectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateRectDialog),
    _accepted(false)
{
    ui->setupUi(this);
}

CreateRectDialog::~CreateRectDialog()
{
    delete ui;
}
