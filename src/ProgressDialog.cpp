#include "ProgressDialog.h"
#include <QLabel>

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent)
{
   QHBoxLayout* layout;

   layout = new QHBoxLayout(this);
   progressbar = new QProgressBar(this);
   //this->setHidden(true);
   this->progressbar->setMaximum(100);
   this->progressbar->setMinimum(0);

   QLabel *progressLabel = new QLabel(this);
   progressLabel->setText("Progress:");


   layout->addWidget(progressLabel);
   layout->addWidget(progressbar);

   setWindowTitle(tr("Loading"));

   this->resize(500,50);
   this->setAutoFillBackground(true);

   this->setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);

}

ProgressDialog::~ProgressDialog()
{

}

QProgressBar *ProgressDialog::getProgressbar() const
{
    return progressbar;
}

void ProgressDialog::setProgressbar(QProgressBar *value)
{
    progressbar = value;
}
