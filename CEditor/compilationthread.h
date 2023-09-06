#ifndef COMPILATIONTHREAD_H
#define COMPILATIONTHREAD_H

#include "headers.h"

class CompilationThread : public QThread
{
    Q_OBJECT

public:
    CompilationThread(int flag, QString filePath, QStringList arguments, QObject *parent = nullptr);

signals:
    void compilationFinished(int exitCode, const QString &outputText);//0-compile,1-run

protected:
    void run() override;

private:
    QString filePath;
    QStringList arguments;
    int flag;
};

#endif // COMPILATIONTHREAD_H


