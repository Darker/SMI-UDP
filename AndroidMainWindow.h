#ifndef ANDROIDMAINWINDOW_H
#define ANDROIDMAINWINDOW_H

#include <QWidget>
#include "FileClient.h"
namespace Ui {
class AndroidMainWindow;
}

class AndroidMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AndroidMainWindow(QWidget *parent = 0);
    ~AndroidMainWindow();
public slots:
    void sendFile();

private:
    Ui::AndroidMainWindow *ui;
    FileClient* currentClient;
};

#endif // ANDROIDMAINWINDOW_H
