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
    int *array;
    int startingPoint;
    int endPoint;
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

    int startingPoint = 0, endpoint = numOfIterations;
        int *array = new int[ui->steps->toPlainText().toInt()];
    for(int i = 0 ; i < numOfIterations*numOfThreads; i++)
               array[i] = rand() % 1000;

    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), 0, FALSE};
    if(ui->syncType->currentIndex() == 1)
        InitializeCriticalSection(CS);
    if(ui->syncType->currentIndex() == 2)
        hMutex = CreateMutex(nullptr, FALSE, nullptr);
    if(ui->syncType->currentIndex() == 3)
        hSemaphore = CreateSemaphore(nullptr, 1, 16, nullptr);

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
            STSA->array = array;
            STSA->numberOfIterations = numOfIterations;
            STSA->startingPoint = startingPoint;
            STSA->endPoint = endpoint;
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
            endpoint += numOfIterations;
            startingPoint += numOfIterations;
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


unsigned int _stdcall MutextThread(LPVOID arg){

    SSTSA* STSA = (SSTSA*)arg;
    int minElem = STSA->array[0];
    for(int i = 1; i < STSA->numberOfIterations; ++i)
    {
        if(minElem > STSA->array[i])
            minElem = STSA->array[i];
    }

    std::fstream fRes;
    std::string filePath = PATH + "minElementSearch.txt";
    fRes.open(filePath, std::ios::out| std::ios::app);
    if(fRes.is_open())
    {
        WaitForSingleObject(hMutex, INFINITE);
        for(int i = 0; i < STSA->numberOfIterations; ++i)
            fRes << STSA->array[i] << std::endl;
        ReleaseMutex(hMutex);
    }else{
        std::cerr << "Error: file could not be opened" << std::endl;
        return 1;
    }
    _endthreadex(minElem);
        return 0;
}

unsigned int _stdcall SemaphoretThread(LPVOID arg){
    SSTSA* STSA = (SSTSA*)arg;
    int minElem = STSA->array[0];
    for(int i = 1; i < STSA->numberOfIterations; ++i)
    {
        if(minElem > STSA->array[i])
            minElem = STSA->array[i];
    }

    std::fstream fRes;
    std::string filePath = PATH + "minElementSearch.txt";
    fRes.open(filePath, std::ios::out| std::ios::app);
    if(fRes.is_open())
    {
        WaitForSingleObject(hSemaphore, INFINITE);
        for(int i = 0; i < STSA->numberOfIterations; ++i)
            fRes << STSA->array[i] << std::endl;
        ReleaseSemaphore(hSemaphore, 1, nullptr);
    }else{
        std::cerr << "Error: file could not be opened" << std::endl;
        return 1;
    }
    _endthreadex(minElem);
    return 0;
}

void ThreadsSync::on_runall_clicked()
{
    QElapsedTimer timer;
    timer.start();
    int* minElements = new int[numOfThreads];

    for(int i = 0; i < numOfThreads; ++i){
        ui->availableThreads->item(i, 2)->setText("Running");
        ResumeThread(hThreads[i]);
    }
    WaitForMultipleObjects(numOfThreads, hThreads, TRUE, INFINITE);

    timer.nsecsElapsed();
    ui->time->setText(QString::number(timer.nsecsElapsed()/1000000.0) + " ms");

    for(int i = 0; i < numOfThreads; i++){
        if(ui->chosenTask->currentIndex() == 1){
            DWORD exitCode;
            GetExitCodeThread(hThreads[i], &exitCode);
            minElements[i] = exitCode;
        }
    }

    for(int i = 0; i < numOfThreads; ++i){
         ui->availableThreads->item(i, 2)->setText("Finished");
    }
    int minimalElement = minElements[0];

    for(int i = 1 ; i < numOfThreads; ++i)
        if(minElements[i] < minimalElement)
                minimalElement = minElements[i];
    std::fstream fRes;
    std::string filePath = PATH + "minElementSearch.txt";
    fRes.open(filePath, std::ios::out| std::ios::app);
    fRes << "Minimal element: " << minimalElement << std::endl;
    fRes.close();

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
