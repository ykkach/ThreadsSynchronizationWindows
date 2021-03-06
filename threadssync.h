#ifndef THREADSSYNC_H
#define THREADSSYNC_H

#include <QMainWindow>
#include <iostream>
#include <Windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class ThreadsSync; }
QT_END_NAMESPACE

class ThreadsSync : public QMainWindow
{
    Q_OBJECT

public:
    ThreadsSync(QWidget *parent = nullptr);
    ~ThreadsSync();

private slots:

    void on_create_clicked();


    void on_priority_currentIndexChanged(int index);

    void on_run_clicked();

    void on_Pause_clicked();

    void on_killThread_clicked();

    void on_runall_clicked();

private:
    QString priority(int);
    Ui::ThreadsSync *ui;
    HANDLE *hThreads;
    int numOfThreads;
};

unsigned int _stdcall InterlockdThread(LPVOID);
unsigned int _stdcall CSdThread(LPVOID);
unsigned int _stdcall MutextThread(LPVOID);
unsigned int _stdcall SemaphoretThread(LPVOID);
void swap(int *, int *);
#endif // THREADSSYNC_H
