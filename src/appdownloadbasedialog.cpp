#include "appdownloadbasedialog.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

void AppDownloadBaseDialog::addProgressBar(int index)
{
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(25);
    m_progressBar->setStyleSheet(
        "QProgressBar { border-radius: 6px; background: #eee; } "
        "QProgressBar::chunk { background: #34C759; }");
    m_layout->insertWidget(index, m_progressBar);
}

AppDownloadBaseDialog::AppDownloadBaseDialog(const QString &appName,
                                             QWidget *parent)
    : QDialog(parent), m_appName(appName), m_downloadProcess(nullptr),
      m_progressTimer(nullptr)
{
    // Common UI: progress bar and action button
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(30, 30, 30, 30);

    QLabel *nameLabel = new QLabel(appName);
    nameLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333;");
    m_layout->addWidget(nameLabel);

    m_actionButton = nullptr; // Derived classes set this
}

void AppDownloadBaseDialog::startDownloadProcess(const QStringList &args,
                                                 const QString &outputDir,
                                                 int index)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString logFilePath =
        QDir::temp().filePath(QString("%1_%2.log").arg(m_appName, timestamp));

    QFile *logFile = new QFile(logFilePath);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error",
                              "Failed to open log file for writing.");
        return;
    }
    logFile->close();

    m_downloadProcess = new QProcess(this);
    // m_downloadProcess->setProcessEnvironment(env);
    // m_downloadProcess->setWorkingDirectory(workingDir);

    m_downloadProcess->setStandardOutputFile(logFilePath, QIODevice::Append);
    m_downloadProcess->setStandardErrorFile(logFilePath, QIODevice::Append);
    m_downloadProcess->start("ipatool", args);
    // TODO: handle errors
    addProgressBar(index);

    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this,
            [this, logFilePath, outputDir]() {
                checkDownloadProgress(logFilePath, m_appName, outputDir);
            });

    connect(m_downloadProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, logFilePath, outputDir](int exitCode,
                                           QProcess::ExitStatus exitStatus) {
                checkDownloadProgress(logFilePath, m_appName, outputDir);
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_downloadProcess->deleteLater();
                //     m_progressBar->setValue(100);
                //     QMessageBox::information(
                //         this, "Download Complete",
                //         QString("Successfully downloaded
                //         %1.").arg(m_appName));
                //     accept();
                // } else {
                //     QMessageBox::critical(
                //         this, "Download Failed",
                //         QString("Failed to download %1. Exit code: %2")
                //             .arg(m_appName)
                //             .arg(exitCode));
                //     reject();
                // }
            });

    m_progressTimer->start(1000);

    if (m_actionButton)
        m_actionButton->setEnabled(false);
}

void AppDownloadBaseDialog::checkDownloadProgress(const QString &logFilePath,
                                                  const QString &appName,
                                                  const QString &outputDir)
{
    QFile logFile(logFilePath);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString fileContents = QString::fromUtf8(logFile.readAll());
    logFile.close();

    int jsonStart = fileContents.indexOf('{');
    qDebug() << "JSON Start:" << jsonStart;
    if (jsonStart != -1) {
        QString jsonString = fileContents.mid(jsonStart);
        QJsonParseError parseError;
        QJsonDocument doc =
            QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            qDebug() << "Parsed JSON successfully";
            QJsonObject jsonObj = doc.object();
            QString level = jsonObj.value("level").toString();
            bool success = jsonObj.value("success").toBool();

            m_progressTimer->stop();
            // if (m_actionButton)
            //     m_actionButton->setEnabled(true);

            if (level == "error") {
                QString errorMsg = jsonObj.contains("error")
                                       ? jsonObj.value("error").toString()
                                       : "Unknown error";
                QMessageBox::critical(
                    this, "Download Failed",
                    QString("Failed to download %1. Error: %2")
                        .arg(m_appName, errorMsg));
                reject();
                return;
            } else if (level == "info" && success) {
                m_progressBar->setValue(100);
                // QMessageBox::information(
                //     this, "Download Successful",
                //     QString("Successfully downloaded %1. Would you like to "
                //             "open the directory?")
                //         .arg(m_appName));
                if (QMessageBox::Yes ==
                    QMessageBox::question(
                        this, "Open Directory",
                        QString("Successfully downloaded. Would you like "
                                "to open the output directory: %1?")
                            .arg(outputDir))) {
                    QDir dir(outputDir);
                    if (!dir.exists()) {
                        QMessageBox::warning(
                            this, "Directory Not Found",
                            QString("The directory %1 does not exist.")
                                .arg(outputDir));
                    } else {
                        QDesktopServices::openUrl(
                            QUrl::fromLocalFile(outputDir));
                    }
                }
                accept();
                return;
            }
        }
    }

    QRegularExpression re(R"(downloading\s+(\d+)%\s+\|)");
    QRegularExpressionMatchIterator i = re.globalMatch(fileContents);
    int percent = -1;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        percent = match.captured(1).toInt();
    }
    if (percent != -1) {
        m_progressBar->setValue(percent);
    }
}