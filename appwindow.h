#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include "runcommand.h"

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

public slots:
    void actionGenerateHashes();
    void actionClearAll();

    void actionCopyMD5();
    void actionCopySHA256();
    void actionCopySHA512();
    void clearStatusText();

private:
    Ui::AppWindow *ui;
    bool isLoaded;
    QString filepath;

    QVector<RunCommand*> hashList;
};
#endif // APPWINDOW_H
