#include "AndroidMainWindow.h"
#include "ui_AndroidMainWindow.h"
#include "SendProgress.h"
AndroidMainWindow::AndroidMainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AndroidMainWindow)
{
    ui->setupUi(this);
}

AndroidMainWindow::~AndroidMainWindow()
{
    delete ui;
}

void AndroidMainWindow::sendFile()
{
    const QString filename(ui->sendFileName->text());
    currentClient = new FileClient(QHostAddress(ui->clientAddr->text()), ui->clientPort->value(), this);

    ui->sendFileName->setDisabled(true);
    ui->pushButton->setDisabled(true);
    QObject::connect(currentClient, &FileClient::fileSent, this, [this]() {
        delete currentClient;
        ui->sendFileName->setDisabled(false);
        ui->pushButton->setDisabled(false);

    });

    // Make user informer
    SendProgress* progress = new SendProgress(currentClient->getSocket());
    QObject::connect(progress, &SendProgress::progressMessage, this, [this](SendProgress::ProgressInfo info) {
        ui->labProgress->setText(QString::number(qRound(info.percentage))+"%");
        ui->labSpeed->setText(SendProgress::size_human(info.fileSpeed)+"/s");
        ui->labSent->setText(SendProgress::size_human(info.sendSpeed)+"/s");
        ui->labReceived->setText(SendProgress::size_human(info.receiveSpeed)+"/s");
    });

    currentClient->sendFile(filename);
}
