#ifndef LOGGER_H
#define LOGGER_H

#include "precompiled.h"

class Logger : public QObject
{
  Q_OBJECT
public:
  explicit Logger(QObject *parent, QString fileName);
  ~Logger();

private:
  QFile *file      = nullptr;
  bool m_showDate;

signals:

public slots:
  void write(const QString &value);
};

#endif // LOGGER_H