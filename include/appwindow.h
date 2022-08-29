#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include "worker.h"
#include "hashgen_ini.h"

class ProgressDialog;

QT_BEGIN_NAMESPACE
namespace Ui { class AppWindow; }
QT_END_NAMESPACE

class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    AppWindow(QWidget *parent = nullptr);
    ~AppWindow();

    void initializeApp();
    QString loadStyle(const QString& file);
    void initComponents();
    bool fileNotLoaded();
    void populateWorkers(const QString& filepath);
    void runWorkers();
    QString parseText(const QStringList& list);
    void disableFields(bool _val);


    void populateWorkersOnWindows(const QString &filepath);
    void populateWorkersOnLinux(const QString &filepath);

    int getError_counter() const;

    void processArgs();

public slots:
    void actionGenerateHashes();
    void actionClearAll();
    void actionCopyMD5();
    void actionCopySHA256();
    void actionCopySHA512();
    void clearStatusText();
    void actionReloadhashes();
    void isRunnableEnd(int i);

    void fetchResult(const int type,  const QStringList list);

    void closeApp();

    void openAboutDialog();



private:
    Ui::AppWindow *ui;
    bool isLoaded;
    QString filepath;
    QVector<ProcessWorker*> hashList;
    ProgressDialog *progress;
    int progress_complete;
    HashgenIniConfig config;

    int error_counter;
};
#endif // APPWINDOW_H
