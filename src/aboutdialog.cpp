#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    this->ui->textAbout->setText(
                "Version: 0.3\n"
                "\nby: Archdark"
);

}

AboutDialog::~AboutDialog()
{
    delete ui;
}
