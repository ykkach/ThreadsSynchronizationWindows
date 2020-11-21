#include "threadssync.h"
#include "ui_threadssync.h"

ThreadsSync::ThreadsSync(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ThreadsSync)
{
    ui->setupUi(this);
}

ThreadsSync::~ThreadsSync()
{
    delete ui;
}

