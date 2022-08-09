#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H
#include <QObject>
#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>

class ProgressDialog : public QDialog {
    Q_OBJECT
public:
    ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();
    QProgressBar *getProgressbar() const;
    void setProgressbar(QProgressBar *value);

private:
    QProgressBar *progressbar;
};

#endif // PROGRESS_DIALOG_H
