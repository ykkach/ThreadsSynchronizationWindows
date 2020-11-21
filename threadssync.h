#ifndef THREADSSYNC_H
#define THREADSSYNC_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ThreadsSync; }
QT_END_NAMESPACE

class ThreadsSync : public QMainWindow
{
    Q_OBJECT

public:
    ThreadsSync(QWidget *parent = nullptr);
    ~ThreadsSync();

private:
    Ui::ThreadsSync *ui;
};
#endif // THREADSSYNC_H
