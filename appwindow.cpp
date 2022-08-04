#include "appwindow.h"
#include "ui_appwindow.h"
#include <QtGlobal>
#include<QDebug>
#include <QFileDialog>
#include <QClipboard>
#include <QTimer>
#include <QMessageBox>
#include <QThreadPool>


AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AppWindow)
{
    ui->setupUi(this);
    this->initializeApp();
    this->isLoaded = false;

    this->ui->btClear->setDisabled(true);
}

AppWindow::~AppWindow()
{
    delete ui;
}

void AppWindow::initializeApp()
{
    QString style = this->loadStyle(":/dark/window.qss");
    this->setStyleSheet(style);
    this->initComponents();





    //hashList.push_back(RunCommand("certUtil", QStringList() << "-hashfile " ));

}

QString AppWindow::loadStyle(const QString &file)
{
    QFile in(file);


    if(!in.open(QFile::OpenModeFlag::ReadOnly)){
        return "";
    }

    QByteArray data = in.readAll();
    QString str(data);
    return str;
}

void AppWindow::initComponents()
{
    //ui->acClipboard_MD5->connect(this,"click",)
    //this->connect(this->ui->acGenerateHash, SIGNAL(triggered()), this, SLOT(actionGenerateHashes()));
    //connect(this->ui->acGenerateHash, &QAction::triggered, this, &AppWindow::actionGenerateHashes);
    QObject::connect(this->ui->acGenerateHash, &QAction::triggered, this, &AppWindow::actionGenerateHashes);
    this->ui->btGenerate->setDefaultAction(this->ui->acGenerateHash);

    QObject::connect(this->ui->acClear, &QAction::triggered, this, &AppWindow::actionClearAll);
    this->ui->btClear->setDefaultAction(this->ui->acClear);


    QObject::connect(this->ui->acClipboard_MD5,&QAction::triggered, this, &AppWindow::actionCopyMD5);
    QObject::connect(this->ui->acClipboard_SHA256,&QAction::triggered, this, &AppWindow::actionCopySHA256);
    QObject::connect(this->ui->acClipboard_SHA512,&QAction::triggered, this, &AppWindow::actionCopySHA512);

    this->ui->btClipboardMD5->setDefaultAction(this->ui->acClipboard_MD5);
    this->ui->btClipboardSHA256->setDefaultAction(this->ui->acClipboard_SHA256);
    this->ui->btClipboardSHA512->setDefaultAction(this->ui->acClipboard_SHA512);
}

bool AppWindow::fileNotLoaded()
{
   if(!this->isLoaded){
       QMessageBox::critical(this, "Erro", "Nenhum Arquivo foi carregado, por favor carregue um arquivo antes de usar esta opção!", QMessageBox::StandardButton::Ok);
       return false;
   }

   return true;
}

void AppWindow::actionGenerateHashes()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::ViewMode::List);

    QStringList path;

    if(!dialog.exec()){
        return;
    }

    path = dialog.selectedFiles();
    this->filepath = path[0];
    this->ui->fieldFile->setPlainText(path.back());
    this->isLoaded = true;
    this->ui->btClear->setDisabled(false);




    hashList.push_back(new RunCommand("certUtil",
                                  QStringList() << "-hashfile" << this->filepath << "MD5",
                                  HashType::MD5));


    hashList.push_back(new RunCommand("certUtil",
                                  QStringList() << "-hashfile" << this->filepath << "SHA256",
                                  HashType::SHA256));

    hashList.push_back(new RunCommand("certUtil",
                                  QStringList() << "-hashfile" << this->filepath << "SHA512",
                                  HashType::SHA512));




  for(auto& cmd : hashList){
   cmd->setAutoDelete(true);
   QThreadPool::globalInstance()->start(cmd);
  }

}

void AppWindow::actionClearAll()
{
    this->fileNotLoaded();

    this->isLoaded = false;
    this->ui->fieldMd5->clear();
    this->ui->fieldSHA256->clear();
    this->ui->fieldSHA512->clear();
    this->ui->fieldFile->clear();
    this->ui->btClear->setDisabled(true);
}

void AppWindow::actionCopyMD5()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    c->setText(this->ui->fieldMd5->toPlainText());
    this->ui->labelStatus->setText("Hash MD5 Copiado para o clipboard, Use CTRL+V para Colar!");
    QTimer::singleShot(5000, this, &AppWindow::clearStatusText);
}

void AppWindow::actionCopySHA256()
{
    if(!this->fileNotLoaded()) return;
    QClipboard *c = QApplication::clipboard();
    c->setText(this->ui->fieldSHA256->toPlainText());
}

void AppWindow::actionCopySHA512()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    c->setText(this->ui->fieldSHA512->toPlainText());
}

void AppWindow::clearStatusText()
{
    this->ui->labelStatus->clear();
}

