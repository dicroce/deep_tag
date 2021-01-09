#include "createrectdialog.h"
#include "ui_createrectdialog.h"

CreateRectDialog::CreateRectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateRectDialog)
{
    ui->setupUi(this);
}

CreateRectDialog::~CreateRectDialog()
{
    delete ui;
}
