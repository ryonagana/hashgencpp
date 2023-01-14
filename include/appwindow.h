#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include "ProcessWorker.h"
#include "HashgenIniConfig.h"

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

    //actions
    // this actions is used only when an action is triggered
    void actionGenerateHashes();
    void actionClearAll();
    void actionCopyMD5();
    void actionCopySHA256();
    void actionCopySHA512();
    void actionReloadhashes();
    void actionSave();
    void actionSaveAs();

    void actionLoadHashFile();

    // main methods
    void clearStatusText();
    void isRunnableEnd(int i);
    void fetchResult(const int type,  const QStringList list);
    void closeApp();
    void openAboutDialog();


    void updateWindowStatus();
    void updateStatusText(const QString message, const int delay_time);









private:
    Ui::AppWindow *m_ui;
    bool m_isLoaded;
    QString m_filepath;
    QVector<ProcessWorker*> m_hashList;
    ProgressDialog *m_progress;
    int m_progress_complete;
    HashgenIniConfig m_config;

    bool m_dirty;
    int m_error_counter;


     void doGenerateHash(const QString& filename);
};
#endif // APPWINDOW_H
