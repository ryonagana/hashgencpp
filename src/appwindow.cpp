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
#include "ProgressDialog.h"
#include "BinaryCheck.h"


AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::AppWindow)
{

    this->m_config.loadFile("config.ini");



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
    this->m_error_counter = missing_files.count();
#endif


    m_ui->setupUi(this);
    this->initializeApp();
    this->m_isLoaded = false;
    this->processArgs();
    this->m_ui->btClear->setDisabled(true);
    this->m_progress_complete = 0;



}

AppWindow::~AppWindow()
{
    delete m_ui;
}


void AppWindow::processArgs()
{
   QStringList args = QApplication::arguments();
   QFileInfo info;

   if(args.count() < 2) return;

   this->m_filepath = args[1];

   info.setFile(this->m_filepath);


   this->m_ui->fieldFile->setPlainText(info.fileName());
   this->m_ui->btClear->setDisabled(false);
   this->populateWorkers(this->m_filepath);

   this->m_ui->btGenerate->setDefaultAction(this->m_ui->acRegenerate);
   this->runWorkers();
   this->m_isLoaded = true;
   this->disableFields(false);

}


void AppWindow::initializeApp()
{
    QString style = this->loadStyle(":/dark/window.qss");
    this->setStyleSheet(style);
    this->initComponents();
    m_dirty = false;
    this->m_ui->acSave->setDisabled(!this->m_dirty);
    this->m_ui->acSaveAs->setDisabled(!this->m_dirty);
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

    QObject::connect(this->m_ui->acGenerateHash, &QAction::triggered, this, &AppWindow::actionGenerateHashes);
    this->m_ui->btGenerate->setDefaultAction(this->m_ui->acGenerateHash);

    QObject::connect(this->m_ui->acClear, &QAction::triggered, this, &AppWindow::actionClearAll);
    this->m_ui->btClear->setDefaultAction(this->m_ui->acClear);


    m_progress = new ProgressDialog(this);


    QObject::connect(this->m_ui->acClipboard_MD5,&QAction::triggered, this, &AppWindow::actionCopyMD5);
    QObject::connect(this->m_ui->acClipboard_SHA256,&QAction::triggered, this, &AppWindow::actionCopySHA256);
    QObject::connect(this->m_ui->acClipboard_SHA512,&QAction::triggered, this, &AppWindow::actionCopySHA512);
    QObject::connect(this->m_ui->acRegenerate, &QAction::triggered, this, &AppWindow::actionReloadhashes);
    QObject::connect(this->m_ui->actionAbout, &QAction::triggered, this, &AppWindow::openAboutDialog);
    QObject::connect(this->m_ui->acSaveAs, &QAction::triggered, this, &AppWindow::actionSaveAs);
    QObject::connect(this->m_ui->acSave, &QAction::triggered, this, &AppWindow::actionSave);
    QObject::connect(this->m_ui->actionChecksumFromFile, &QAction::triggered, this, &AppWindow::actionLoadHashFile);

    this->m_ui->btClipboardMD5->setDefaultAction(this->m_ui->acClipboard_MD5);
    this->m_ui->btClipboardSHA256->setDefaultAction(this->m_ui->acClipboard_SHA256);
    this->m_ui->btClipboardSHA512->setDefaultAction(this->m_ui->acClipboard_SHA512);
    this->m_ui->btChecksum->setDefaultAction(this->m_ui->actionChecksumFromFile);

    QObject::connect(this->m_ui->acQuit, &QAction::triggered, this, &AppWindow::closeApp);


    this->m_ui->checkMD5->setChecked(true);
    this->m_ui->checkSHA256->setChecked(true);
    this->m_ui->checkSHA512->setChecked(true);

    this->disableFields(true);
}

bool AppWindow::fileNotLoaded()
{
   if(!this->m_isLoaded){
       QMessageBox::critical(this, "Error", "Please load a file before copy", QMessageBox::StandardButton::Ok);
       return false;
   }

   return true;
}


void AppWindow::populateWorkersOnWindows(const QString &filepath)
{
    QFileInfo info;
    info.setFile(filepath);

    if(!m_isLoaded){

        m_hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "MD5",
                                                                           HashType::MD5));

        m_hashList.back()->setSize(info.size());

        m_hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA256",
                                                                           HashType::SHA256));
        m_hashList.back()->setSize(info.size());

        m_hashList.push_back(new ProcessWorker("certUtil",
                                                                           QStringList() << "-hashfile" << filepath << "SHA512",
                                                                           HashType::SHA512));
        m_hashList.back()->setSize(info.size());

    }
    return;
}

void AppWindow::populateWorkersOnLinux(const QString &filepath)
{
    QFileInfo info;
    info.setFile(filepath);

    if(!m_isLoaded){
        m_hashList.push_back(new ProcessWorker("md5sum", QStringList() << "-b" << filepath, HashType::MD5));
        m_hashList.push_back(new ProcessWorker("sha256sum", QStringList() << "-b" << filepath,HashType::SHA256));
        m_hashList.push_back(new ProcessWorker("sha512sum", QStringList() << "-b" << filepath, HashType::SHA512));
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


    for(auto& h : m_hashList){
        QObject::connect(&h->getSignals(), &ProcessSignals::progress, this, &AppWindow::isRunnableEnd);
        QObject::connect(&h->getSignals(), &ProcessSignals::result, this, &AppWindow::fetchResult);
        QThreadPool::globalInstance()->start(h);
    }

    if(!m_isLoaded){
        m_progress->exec();
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
    QStringList file_selected = dialog.selectedFiles();
    this->doGenerateHash(file_selected[0]);

    /*
    path = dialog.selectedFiles();
    this->m_filepath = path[0];

    info.setFile(this->m_filepath);
#if defined(Q_OS_LINUX)
    if(info.isSymLink()){
        QMessageBox::critical(this, "Error", "Symlink is not Allowed!", QMessageBox::StandardButton::Ok);
        this->actionClearAll();
        return;
    }
#endif

    this->m_ui->fieldFile->setPlainText(info.fileName());
    this->m_ui->btClear->setDisabled(false);
    this->populateWorkers(this->m_filepath);

    this->m_ui->btGenerate->setDefaultAction(this->m_ui->acRegenerate);
    this->runWorkers();
    this->m_isLoaded = true;
    this->m_dirty = true;
    this->disableFields(false);

    this->updateWindowStatus();
    */

}

void AppWindow::actionClearAll()
{
    if(!this->fileNotLoaded()){
        return;
    }


    this->m_isLoaded = false;
    this->m_ui->fieldMd5->clear();
    this->m_ui->fieldSHA256->clear();
    this->m_ui->fieldSHA512->clear();
    this->m_ui->fieldFile->clear();
    this->m_ui->btClear->setDisabled(true);
    this->m_ui->btGenerate->setDefaultAction(this->m_ui->acGenerateHash);

    // all  pointers is automagically deleted after threadpool runs
    // it changes  the pointer ownership to itself  when it  finishes the runnable process
    // frees automatically, no need to worries about memory leak here
    this->m_hashList.clear();
    this->disableFields(true);
    return;
}

void AppWindow::actionCopyMD5()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    c->setText(this->m_ui->fieldMd5->toPlainText());
    this->m_ui->labelStatus->setText("Hash MD5 copied to clipboard (use CTRL + V shortcut tp paste");
    QTimer::singleShot(5000, this, &AppWindow::clearStatusText);
}

void AppWindow::actionCopySHA256()
{
    if(!this->fileNotLoaded()) return;
    QClipboard *c = QApplication::clipboard();
    this->m_ui->labelStatus->setText("Hash SHA256 copied to clipboard (use CTRL + V shortcut tp paste");
    c->setText(this->m_ui->fieldSHA256->toPlainText());
}

void AppWindow::actionCopySHA512()
{
    if(!this->fileNotLoaded()) return;

    QClipboard *c = QApplication::clipboard();
    this->m_ui->labelStatus->setText("Hash SHA512 copied to clipboard (use CTRL + V shortcut tp paste");
    c->setText(this->m_ui->fieldSHA512->toPlainText());
}

void AppWindow::clearStatusText()
{
    this->m_ui->labelStatus->clear();
}

void AppWindow::actionReloadhashes()
{
    if(!this->m_isLoaded) return;

    this->m_hashList.clear();
    this->populateWorkers(this->m_filepath);
    this->runWorkers();

    this->m_ui->labelStatus->setText("Hashes Regenerated");
    QTimer::singleShot(5000, this, &AppWindow::clearStatusText);

    return;

}

void AppWindow::actionSave()
{
    if(m_dirty){
        actionSaveAs();
    }

}

void AppWindow::actionSaveAs()
{
    if(!m_isLoaded){
        QMessageBox::critical(this, "Error", "Please generate a hash before save", QMessageBox::StandardButton::Ok);
        return;
    }

    QFileDialog dialog;

    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::ViewMode::List);

    if(!dialog.exec()){
        return;
    }


    QStringList filename = dialog.selectedFiles();



    QFile fp(filename[0]);

    if(!fp.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::critical(this, "Error", "File Not Found", QMessageBox::StandardButton::Ok);
        return;
    }


    QStringList hashes;

    hashes << this->m_ui->fieldMd5->toPlainText();
    hashes << "|";
    hashes << this->m_ui->fieldSHA256->toPlainText();
    hashes << "|";
    hashes << this->m_ui->fieldSHA512->toPlainText();

    for(int i = 0; i < hashes.length();i++){
        QByteArray bytes;
        bytes.append(hashes[i]);
        fp.write(bytes, bytes.length());
    }

    fp.close();

    QStringList msg;

    msg << "File: " << fp.fileName() << " written with sucess";
    updateStatusText(msg.join(" "),10000);


}

void AppWindow::actionLoadHashFile()
{
    QFileDialog diag;

    diag.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    diag.setFileMode(QFileDialog::FileMode::ExistingFile);
    diag.setViewMode(QFileDialog::ViewMode::List);
    diag.setNameFilter("All (*.sha256 *.sha512 *.md5)");

    if(!diag.exec()){
        return;
    }
    QStringList filename = diag.selectedFiles();

    QFile fp(filename[0]);

    if(!fp.open(QFile::OpenModeFlag::ReadOnly)){
        updateStatusText("Error Trying to load checksum file",10000);
        return;
    }

    QString content = fp.readAll();
    QStringList split_file = content.split(" ");
    this->m_ui->fieldChecksum->setPlainText(split_file[0]);
    fp.close();
    this->doGenerateHash(split_file[1]);
}

void AppWindow::isRunnableEnd(int i)
{
    qDebug() << i << "\n";


    float percent = 0.001f;
    m_progress_complete += 1;

    percent = (m_progress_complete / this->m_hashList.size()) * 100;
    this->m_progress->getProgressbar()->setValue((int)percent);

    if((int)percent > 99){
        this->m_progress->getProgressbar()->setValue(0);
        this->m_progress->close();
    }

}

void AppWindow::fetchResult(const int type, const QStringList list)
{
    switch(type){
        case 0: //md5
        this->m_ui->fieldMd5->setPlainText(parseText(list));
        break;

        case 1: //sha256
            this->m_ui->fieldSHA256->setPlainText(parseText(list));
        break;

        case 2: //sha512
            this->m_ui->fieldSHA512->setPlainText(parseText(list));
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

void AppWindow::updateWindowStatus()
{
    this->m_ui->acSave->setDisabled(this->m_isLoaded);
    this->m_ui->acSaveAs->setDisabled(!this->m_dirty);
}

void AppWindow::updateStatusText(const QString message, const int delay_time)
{
    this->m_ui->labelStatus->setText(message);
    QTimer::singleShot(delay_time, this, &AppWindow::clearStatusText);
}



int AppWindow::getError_counter() const
{
    return m_error_counter;
}



QString AppWindow::parseText(const QStringList &list)
{
    QString res;

#ifdef Q_OS_WINDOWS
    res = list[1];
    res.replace(" ","");
    return res;
#elif defined(Q_OS_LINUX)
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
        this->m_ui->fieldFile->setEnabled(!_val);
        this->m_ui->fieldMd5->setEnabled(!_val);
        this->m_ui->fieldSHA256->setEnabled(!_val);
        this->m_ui->fieldSHA512->setEnabled(!_val);
        return;
    }


    this->m_ui->fieldFile->setEnabled(_val);
    this->m_ui->fieldMd5->setEnabled(_val);
    this->m_ui->fieldSHA256->setEnabled(_val);
    this->m_ui->fieldSHA512->setEnabled(_val);
    return;
}


void AppWindow::doGenerateHash(const QString &filename)
{
    QFileInfo info;
    info.setFile(filename);

    if(!info.exists()){
        updateStatusText("File Not Found!",10000);
        return;
    }

    this->m_filepath = filename;

#if defined(Q_OS_LINUX)
    if(info.isSymLink()){
        QMessageBox::critical(this, "Error", "Symlink is not Allowed!", QMessageBox::StandardButton::Ok);
        this->actionClearAll();
        return;
    }
#endif

    this->m_ui->fieldFile->setPlainText(info.fileName());
    this->m_ui->btClear->setDisabled(false);
    this->populateWorkers(this->m_filepath);

    this->m_ui->btGenerate->setDefaultAction(this->m_ui->acRegenerate);
    this->runWorkers();
    this->m_isLoaded = true;
    this->m_dirty = true;
    this->disableFields(false);

    this->updateWindowStatus();
}


