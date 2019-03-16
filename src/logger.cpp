#include "logger.h"

Logger::Logger(QObject *parent, QString fileName) : QObject(parent) {
    m_showDate = true;

    if (!fileName.isEmpty()) {
        file = new QFile;
        file->setFileName(fileName);
        file->open(QIODevice::Append | QIODevice::Text);
    }
    
    write("=========Startup==========");
}

void Logger::write(const QString &value) {
    if (!file)
        return;
        
    QString text = value;
    text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + text;
    QTextStream out(file);
    out.setCodec("UTF-8");
    if (file != 0) {
        out << text << endl;
    }
}

Logger::~Logger() {
    if (file != 0)
        file->close();
}