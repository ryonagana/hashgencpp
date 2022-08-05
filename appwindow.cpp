#include "appwindow.h"
#include "ui_appwindow.h"
#include <QtGlobal>
#include<QDebug>
#include <QFileDialog>
#include <QClipboard>
#include <QTimer>
#include <QMessageBox>
#include <QThreadPool>
#include <QObject>
#include "aboutdialog.h"

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
    QObject::connect(this->ui->acRegenerate, &QAction::triggered, this, &AppWindow::actionReloadhashes);

    QObject::connect(this->ui->actionAbout, &QAction::triggered, this, &AppWindow::openAboutDialog);

    this->ui->btClipboardMD5->setDefaultAction(this->ui->acClipboard_MD5);
    this->ui->btClipboardSHA256->setDefaultAction(this->ui->acClipboard_SHA256);
    this->ui->btClipboardSHA512->setDefaultAction(this->ui->acClipboard_SHA512);


    this->disableFields(true);
}

bool AppWindow::fileNotLoaded()
{
   if(!this->isLoaded){
       QMessageBox::critical(this, "Error", "Please load a file before copy", QMessageBox::StandardButton::Ok);
       return false;
   }

   return true;
}

void AppWindow::populateWorkers(const QString &filepath)
{
    if(!isLoaded){

        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "MD5",
                                                                           HashType::MD5));

        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA256",
                                                                           HashType::SHA256));


        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA512",
                                                                           HashType::SHA512));
    }
}

void AppWindow::runWorkers()
{
    for(auto& cmd : hashList){

     cmd->setAutoDelete(true);

     QObject::connect(&cmd->getSignals(), &ProcessSignals::progress, this, &AppWindow::isRunnableEnd);
     QObject::connect(&cmd->getSignals(), &ProcessSignals::result, this, &AppWindow::fetchResult);
     QThreadPool::globalInstance()->start(cmd);
    }

}

void AppWindow::actionGenerateHashes()
{
    QFileDialog dialog;
    QFileInfo info;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::ViewMode::List);

    QStringList path;

    if(!dialog.exec()){
        return;
    }

    path = dialog.selectedFiles();
    this->filepath = path[0];

    info.setFile(this->filepath);
#if Q_OS_LINUX
    if(info.isSymLink()){
        QMessageBox::critical(this, "Error", "Symlink is not Allowed!", QMessageBox::StandardButton::Ok);
        this->actionClearAll();
        return;
    }
#endif

    this->ui->fieldFile->setPlainText(info.fileName());

    this->ui->btClear->setDisabled(false);


    this->populateWorkers(this->filepath);

    this->ui->btGenerate->setDefaultAction(this->ui->acRegenerate);
    this->runWorkers();
    this->isLoaded = true;
    this->disableFields(false);

}

void AppWindow::actionClearAll()
{
    if(!this->fileNotLoaded()){
        return;
    }


    this->isLoaded = false;
    this->ui->fieldMd5->clear();
    this->ui->fieldSHA256->clear();
    this->ui->fieldSHA512->clear();
    this->ui->fieldFile->clear();
    this->ui->btClear->setDisabled(true);
    this->ui->btGenerate->setDefaultAction(this->ui->acGenerateHash);

    // all  pointers is automagically deleted after threadpool runs
    // it changes  the pointer ownership to itself  when it  finishes the runnable process
    // frees automatically, no need to worries about memory leak here
    this->hashList.clear();
    this->disableFields(true);
    return;
}

void AppWindow::actionCopyMD5()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    c->setText(this->ui->fieldMd5->toPlainText());
    this->ui->labelStatus->setText("Hash MD5 copied to clipboard (use CTRL + V shortcut tp paste");
    QTimer::singleShot(5000, this, &AppWindow::clearStatusText);
}

void AppWindow::actionCopySHA256()
{
    if(!this->fileNotLoaded()) return;
    QClipboard *c = QApplication::clipboard();
    this->ui->labelStatus->setText("Hash SHA256 copied to clipboard (use CTRL + V shortcut tp paste");
    c->setText(this->ui->fieldSHA256->toPlainText());
}

void AppWindow::actionCopySHA512()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    this->ui->labelStatus->setText("Hash SHA512 copied to clipboard (use CTRL + V shortcut tp paste");
    c->setText(this->ui->fieldSHA512->toPlainText());
}

void AppWindow::clearStatusText()
{
    this->ui->labelStatus->clear();
}

void AppWindow::actionReloadhashes()
{
    if(!this->isLoaded) return;

    this->hashList.clear();
    this->populateWorkers(this->filepath);
    this->runWorkers();

    this->ui->labelStatus->setText("Hashes Regenerated");
    QTimer::singleShot(5000, this, &AppWindow::clearStatusText);

    return;

}

void AppWindow::isRunnableEnd(int i)
{
    qDebug() << i << "\n";
}

void AppWindow::fetchResult(const int type, const QStringList list)
{
    (void) type;

    switch(type){
        case 0: //md5
        this->ui->fieldMd5->setPlainText(parseText(list));
        break;

        case 1: //sha256
            this->ui->fieldSHA256->setPlainText(parseText(list));
        break;

        case 2: //sha512
            this->ui->fieldSHA512->setPlainText(parseText(list));
        break;
    }

    return;
}

void AppWindow::openAboutDialog()
{
    AboutDialog about;
    about.exec();
}

QString AppWindow::parseText(const QStringList &list)
{
    QString res;

#ifdef Q_OS_WINDOWS
    res = list[1];
    res.replace(" ","");
    return res;
#else
    return QString::empty();
#endif
}

void AppWindow::disableFields(bool _val)
{
    if(_val){
        this->ui->fieldFile->setEnabled(!_val);
        this->ui->fieldMd5->setEnabled(!_val);
        this->ui->fieldSHA256->setEnabled(!_val);
        this->ui->fieldSHA512->setEnabled(!_val);
        return;
    }


    this->ui->fieldFile->setEnabled(_val);
    this->ui->fieldMd5->setEnabled(_val);
    this->ui->fieldSHA256->setEnabled(_val);
    this->ui->fieldSHA512->setEnabled(_val);
    return;
}

