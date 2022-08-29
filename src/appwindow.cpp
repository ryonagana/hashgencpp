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
#include "progress_dialog.h"
#include "bin_check.h"


AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AppWindow)
{

    this->config.loadFile("config.ini");



#if defined(Q_OS_LINUX)
    QStringList missing_files = BinaryCheck::checkFilesOn_Linux();
    if(missing_files.size() > 0){
        QMessageBox::critical(this, "Error Missing File", "Some Files are Missing:\n\n" + missing_files.join("") + "\n\n Please Install the missing packages");
    }
    this->error_counter = missing_files.count();

#elif defined(Q_OS_WINDOWS)
    QStringList missing_files = BinaryCheck::checkFilesOn_Win32();
    if(missing_files.size() > 0){
        QMessageBox::critical(this, "Error Missing File", "Some Files are Missing:\n\n" + missing_files.join("") + "\n\nPlease Use minimum Windows 7 SP3");
    }
    this->error_counter = missing_files.count();
#endif


    ui->setupUi(this);
    this->initializeApp();
    this->isLoaded = false;


    this->processArgs();


    this->ui->btClear->setDisabled(true);

    this->progress_complete = 0;



}

AppWindow::~AppWindow()
{
    delete ui;
}


void AppWindow::processArgs()
{
   QStringList args = QApplication::arguments();
   QFileInfo info;

   if(args.count() < 2) return;

   this->filepath = args[1];

   info.setFile(this->filepath);


   this->ui->fieldFile->setPlainText(info.fileName());
   this->ui->btClear->setDisabled(false);
   this->populateWorkers(this->filepath);

   this->ui->btGenerate->setDefaultAction(this->ui->acRegenerate);
   this->runWorkers();
   this->isLoaded = true;
   this->disableFields(false);

}


void AppWindow::initializeApp()
{
    QString style = this->loadStyle(":/dark/window.qss");
    this->setStyleSheet(style);
    this->initComponents();
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

    QObject::connect(this->ui->acGenerateHash, &QAction::triggered, this, &AppWindow::actionGenerateHashes);
    this->ui->btGenerate->setDefaultAction(this->ui->acGenerateHash);

    QObject::connect(this->ui->acClear, &QAction::triggered, this, &AppWindow::actionClearAll);
    this->ui->btClear->setDefaultAction(this->ui->acClear);


    progress = new ProgressDialog(this);


    QObject::connect(this->ui->acClipboard_MD5,&QAction::triggered, this, &AppWindow::actionCopyMD5);
    QObject::connect(this->ui->acClipboard_SHA256,&QAction::triggered, this, &AppWindow::actionCopySHA256);
    QObject::connect(this->ui->acClipboard_SHA512,&QAction::triggered, this, &AppWindow::actionCopySHA512);
    QObject::connect(this->ui->acRegenerate, &QAction::triggered, this, &AppWindow::actionReloadhashes);

    QObject::connect(this->ui->actionAbout, &QAction::triggered, this, &AppWindow::openAboutDialog);

    this->ui->btClipboardMD5->setDefaultAction(this->ui->acClipboard_MD5);
    this->ui->btClipboardSHA256->setDefaultAction(this->ui->acClipboard_SHA256);
    this->ui->btClipboardSHA512->setDefaultAction(this->ui->acClipboard_SHA512);

    QObject::connect(this->ui->acQuit, &QAction::triggered, this, &AppWindow::closeApp);


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


void AppWindow::populateWorkersOnWindows(const QString &filepath)
{
    QFileInfo info;
    info.setFile(filepath);

    if(!isLoaded){

        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "MD5",
                                                                           HashType::MD5));

        hashList.back()->setSize(info.size());

        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA256",
                                                                           HashType::SHA256));
        hashList.back()->setSize(info.size());

        hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA512",
                                                                           HashType::SHA512));
        hashList.back()->setSize(info.size());

    }
    return;
}

void AppWindow::populateWorkersOnLinux(const QString &filepath)
{
    QFileInfo info;
    info.setFile(filepath);

    if(!isLoaded){
        hashList.push_back(new ProcessWorker("md5sum", QStringList() << "-b" << filepath, HashType::MD5));
        hashList.push_back(new ProcessWorker("sha256sum", QStringList() << "-b" << filepath,HashType::SHA256));
        hashList.push_back(new ProcessWorker("sha512sum", QStringList() << "-b" << filepath, HashType::SHA512));
    }

    return;
}

void AppWindow::populateWorkers(const QString &filepath)
{
#if defined(Q_OS_WINDOWS)
    populateWorkersOnWindows(filepath);
#elif defined(Q_OS_LINUX)
    populateWorkersOnLinux(filepath);
#endif

}

void AppWindow::runWorkers()
{

    QThreadPool::globalInstance()->setMaxThreadCount(10);


    for(auto& h : hashList){
        QObject::connect(&h->getSignals(), &ProcessSignals::progress, this, &AppWindow::isRunnableEnd);
        QObject::connect(&h->getSignals(), &ProcessSignals::result, this, &AppWindow::fetchResult);
        QThreadPool::globalInstance()->start(h);
    }

    if(!isLoaded){
        progress->exec();
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
#if defined(Q_OS_LINUX)
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


    float percent = 0.001f;
    progress_complete += 1;

    percent = (progress_complete / this->hashList.size()) * 100;
    this->progress->getProgressbar()->setValue((int)percent);

    if((int)percent > 99){
        this->progress->getProgressbar()->setValue(0);
        this->progress->close();
    }

}

void AppWindow::fetchResult(const int type, const QStringList list)
{
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

void AppWindow::closeApp()
{
    this->close();
}

void AppWindow::openAboutDialog()
{
    AboutDialog about;
    about.exec();
}

int AppWindow::getError_counter() const
{
    return error_counter;
}



QString AppWindow::parseText(const QStringList &list)
{
    QString res;

#ifdef Q_OS_WINDOWS
    res = list[1];
    res.replace(" ","");
    return res;
#elif Q_OS_LINUX
    res = list[0];
    QStringList str_sep = res.split(" ");
    return str_sep[0];
#else
    return "<Not Defined>";
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



