#include "threadssync.h"
#include "ui_threadssync.h"
//#include <string>
#include <iostream>
#include <fstream>
//#include <QApplication>
#include <QElapsedTimer>
//#include <QMessageBox>
//#include <sstream>
#include <windows.h>
#include <cmath>
//#include <QDebug>

std::string SSTUDENTDATA = "Yaroslav Kachmar 12700969";
std::string PATH = "C:\\Users\\Yaroslav\\Documents\\build-ThreadsSynchronisationInWindows-Desktop_Qt_5_14_1_MinGW_32_bit-Debug\\";

HANDLE hMutex, hSemaphore;
volatile unsigned RESOURSEINUSEFLAG = FALSE;
LPCRITICAL_SECTION CS;

ThreadsSync::ThreadsSync(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ThreadsSync)
{
    ui->setupUi(this);
}

ThreadsSync::~ThreadsSync()
{
    for(int i = 0; i < numOfThreads; ++i){
       CloseHandle(hThreads[i]);
        }
    delete[] hThreads;
    CloseHandle(hMutex);
    if(ui->syncType->currentIndex() == 1)
        DeleteCriticalSection(CS);
    delete ui;
}

struct SDefaultThreadStartArguments
{
    int numberOfIterations;
};

struct SSortingThreadStartArguments
{
    int numberOfIterations;
};

using SDTSA = SDefaultThreadStartArguments;
using SSTSA = SSortingThreadStartArguments;

void ThreadsSync::on_create_clicked()
{
    numOfThreads = ui->numOfThreads->currentText().toInt();
    hThreads = new HANDLE[numOfThreads];
    ui->availableThreads->setRowCount(numOfThreads);
    ui->availableThreads->setColumnCount(3);
    ui->availableThreads->setHorizontalHeaderItem(0, new QTableWidgetItem("Thread ID"));
    ui->availableThreads->setHorizontalHeaderItem(1, new QTableWidgetItem("Priority"));
    ui->availableThreads->setHorizontalHeaderItem(2, new QTableWidgetItem("State"));

    int numOfIterations = ui->steps->toPlainText().toInt()/numOfThreads;

    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), 0, FALSE};
    if(ui->syncType->currentIndex() == 1)
        InitializeCriticalSection(CS);
    if(ui->syncType->currentIndex() == 3)
        hSemaphore = CreateSemaphore(nullptr, 0, 15, nullptr);
    if(ui->syncType->currentIndex() == 2)
        hMutex = CreateMutex(nullptr, FALSE, nullptr);


    for(int i = 0; i < numOfThreads; ++i){
        unsigned ThreadID;
        switch(ui->chosenTask->currentIndex())
        {
        case 0:{
            SDTSA* DTSA = new SDTSA();
            DTSA->numberOfIterations = numOfIterations;
            switch(ui->syncType->currentIndex())
            {
            case 0:
            {
                hThreads[i] = (HANDLE)_beginthreadex(&sa, 4096, &InterlockdThread, (PVOID)DTSA, CREATE_SUSPENDED, &ThreadID);
                break;
            }
            case 1:
            {
                hThreads[i] = (HANDLE)_beginthreadex(&sa, 4096, &CSdThread, (PVOID)DTSA, CREATE_SUSPENDED, &ThreadID);
                break;
            }
            }
            break;
        }
        case 1:{
            SSTSA* STSA = new SSTSA();
            STSA->numberOfIterations = numOfIterations;
            switch(ui->syncType->currentIndex())
            {
            case 2:
            {
                hThreads[i] = (HANDLE)_beginthreadex(&sa, 4096, &MutextThread, (PVOID)STSA, CREATE_SUSPENDED, &ThreadID);
                break;
            }
            case 3:
            {
                hThreads[i] = (HANDLE)_beginthreadex(&sa, 4096, &SemaphoretThread, (PVOID)STSA, CREATE_SUSPENDED, &ThreadID);
                break;
            }
            }
            break;
        }
        default:;
        }
        if(hThreads[i] == INVALID_HANDLE_VALUE) {
              std::cerr << "Invalid handle value" << std::endl;
           return;
        }
        ui->availableThreads->setItem(i,0,new QTableWidgetItem(QString::number(ThreadID)));
        ui->availableThreads->setItem(i,1,new QTableWidgetItem(priority(GetThreadPriority(hThreads[i]))));
        ui->availableThreads->setItem(i,2,new QTableWidgetItem("Suspended"));
    }
}

unsigned int _stdcall InterlockdThread(LPVOID arg)
{
    SDTSA* DTSA = (SDTSA*)arg;
    for(int i = 0;i < (DTSA->numberOfIterations); ++i){
        while(InterlockedExchange(&RESOURSEINUSEFLAG, TRUE) == TRUE)
            Sleep(0);
        for (unsigned j = 0; j < SSTUDENTDATA.length();++j) {
            std::cout.put(SSTUDENTDATA[j]);
        }
        std::cout << ' ' << GetCurrentThreadId() << std::endl;
        InterlockedExchange(&RESOURSEINUSEFLAG, FALSE);
    }
    return 0;
}

unsigned int _stdcall CSdThread(LPVOID arg)
{
    SDTSA* DTSA = (SDTSA*)arg;
    for(int i = 0;i < (DTSA->numberOfIterations); ++i){
        TryEnterCriticalSection(CS);
        for (unsigned j = 0; j < SSTUDENTDATA.length();++j) {
            std::cout.put(SSTUDENTDATA[j]);
        }
        std::cout << ' ' << GetCurrentThreadId() << std::endl;
        LeaveCriticalSection(CS);
    }
    return 0;
}

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

unsigned int _stdcall MutextThread(LPVOID arg){
    SSTSA* STSA = (SSTSA*)arg;
    int *array = new int[STSA->numberOfIterations];

    for(int i = 0 ; i < STSA->numberOfIterations; i++)
       array[i] = rand() % 1000;
    for(int i = 0; i < STSA->numberOfIterations; ++i)
        {
            bool flag = false;
            for(int j = 0; j < STSA->numberOfIterations-i-1; ++j)
            {
                if(array[j+1] > array[j])
                {
                    flag = true;
                    swap(&array[j],&array[j+1]);
                }
            }
            if(!flag) break;
        }
    WaitForSingleObject(hMutex, INFINITE);
    for(int i = 0; i < STSA->numberOfIterations; ++i)
        std::cout << array[i] << std::endl;
    ReleaseMutex(hMutex);
    return 0;
}

unsigned int _stdcall SemaphoretThread(LPVOID arg){
    SSTSA* STSA = (SSTSA*)arg;
    int *array = new int[STSA->numberOfIterations];

    for(int i = 0 ; i < STSA->numberOfIterations; i++)
       array[i] = rand() % 1000;
    for(int i = 0; i < STSA->numberOfIterations; ++i)
        {
            bool flag = false;
            for(int j = 0; j < STSA->numberOfIterations-i-1; ++j)
            {
                if(array[j+1] > array[j])
                {
                    flag = true;
                    swap(&array[j],&array[j+1]);
                }
            }
            if(!flag) break;
        }
    WaitForSingleObject(hSemaphore, INFINITE);
    for(int i = 0; i < STSA->numberOfIterations; ++i)
        std::cout << array[i] << std::endl;
    ReleaseSemaphore(hSemaphore, 1, nullptr);
    return 0;
}

QString ThreadsSync::priority(int code){
    switch (code)
    {
        case 1: return "Above normal";
        case -1: return "Below normal";
        case 0: return "Normal";
        case -15: return "Idle";
        case 2: return "High";
    default: return "";
    }
}

void ThreadsSync::on_runwithouttime_clicked()
{
    for(int i = 0; i < numOfThreads; ++i){
        ui->availableThreads->item(i, 2)->setText("Running");
        ResumeThread(hThreads[i]);
    }
}

/*

void UiControl::on_runall_clicked()
{
    QElapsedTimer timer;
    timer.start();

    for(int i = 0; i < numOfThreads; ++i){
        ui->availableThreads->item(i, 2)->setText("Running");
        ResumeThread(hThreads[i]);
        WaitForSingleObject(hThreads[i], INFINITE);
    }
    timer.nsecsElapsed();
    ui->time->setText(QString::number(timer.nsecsElapsed()/1000000.0) + " ms");

    for(int i = 0; i < numOfThreads; ++i){
         ui->availableThreads->item(i, 2)->setText("Finished");
    }
}
void UiControl::on_runwithouttime_clicked()
{
    for(int i = 0; i < numOfThreads; ++i){
        ui->availableThreads->item(i, 2)->setText("Running");
        ResumeThread(hThreads[i]);
    }
}

}*/

void ThreadsSync::on_priority_currentIndexChanged(int index)
{
    switch(index){
    case 0: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()],-15);
            ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("Idle");
            break;}
    case 1: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()], -1);
         ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("Below normal");
            break;}
    case 2: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()], 0);
         ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("Normal");
            break;}
    case 3: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()], 1);
        ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("Above normal");
            break;}
    case 4: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()], 2);
         ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("High");
            break;}
    case 5: {
        SetThreadPriority(hThreads[ui->availableThreads->currentIndex().row()], 15);
        ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 1)->setText("Real time");
            break;}
    }
}

void ThreadsSync::on_run_clicked()
{
    ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 2)->setText("Running");
    qApp->processEvents();
    ResumeThread(hThreads[ui->availableThreads->currentIndex().row()]);
    WaitForSingleObject(hThreads[ui->availableThreads->currentIndex().row()], INFINITE);
    ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 2)->setText("Finished");
    ui->availableThreads->item(ui->availableThreads->currentIndex().row(),1)->setText(priority(GetPriorityClass(hThreads[ui->availableThreads->currentIndex().row()])));
}

void ThreadsSync::on_Pause_clicked()
{
    ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 2)->setText("Suspended");
     SuspendThread(hThreads[ui->availableThreads->currentIndex().row()]);
}

void ThreadsSync::on_killThread_clicked()
{
    TerminateThread(hThreads[ui->availableThreads->currentIndex().row()],0);
    ui->availableThreads->item(ui->availableThreads->currentIndex().row(), 2)->setText("Terminated");
}

void ThreadsSync::on_runall_clicked()
{
    QElapsedTimer timer;
    timer.start();

    for(int i = 0; i < numOfThreads; ++i){
        ui->availableThreads->item(i, 2)->setText("Running");
        ResumeThread(hThreads[i]);
    }
    for(int i = 0; i < numOfThreads; i++)
        WaitForSingleObject(hThreads[i], INFINITE);

    timer.nsecsElapsed();
    ui->time->setText(QString::number(timer.nsecsElapsed()/1000000.0) + " ms");

    for(int i = 0; i < numOfThreads; ++i){
         ui->availableThreads->item(i, 2)->setText("Finished");
    }
}
