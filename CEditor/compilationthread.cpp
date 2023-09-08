#include "compilationthread.h"

CompilationThread::CompilationThread(int flag, QString filePath, QStringList arguments, QObject *parent)
    : QThread(parent), filePath(filePath), arguments(arguments), flag(flag){}

void CompilationThread::run()
{
    QString outputText = "";
    int exitCode;
    if(flag == 0){
        // 可执行文件路径
        QString exePath = filePath;
        exePath.replace(".cpp", ".exe");
        // 信息文件
        QFileInfo fileInfo(filePath);
        QString outputFileName = fileInfo.path()+"info.txt";
        QProcess compileProcess;
        QStringList parameterList; // 参数列表
        parameterList << filePath << "-o" << exePath << "-g";
        // 设置标准输出和标准错误输出到临时文件
        compileProcess.setStandardOutputFile(outputFileName);
        compileProcess.setStandardErrorFile(outputFileName);
        // 编译
        compileProcess.start("g++", parameterList);
        compileProcess.waitForFinished();

        exitCode = compileProcess.exitCode();

        QFile infoFile(outputFileName);
        if (exitCode == 0)
        {
            QFileInfo info(exePath);
            qint64 size = info.size();
            const QStringList sizeUnits = {"B", "KB", "MB", "GB"};
            int unitIndex = 0;
            while (size > 1024 && unitIndex < sizeUnits.size() - 1) {
                size /= 1024;
                unitIndex++;
            }
            outputText="编译成功\n""输出文件名："+exePath+"\n"
                       "创建时间："+info.birthTime().toString("yyyy-MM-dd hh:mm:ss") +"\n"
                       "最后修改时间："+info.lastModified().toString("yyyy-MM-dd hh:mm:ss")+"\n"
                       "文件大小："+QString::number(size)+sizeUnits[unitIndex]+"\n";
        }
        else
        {
            // 读取编译错误信息
            infoFile.open(QIODevice::ReadOnly | QIODevice::Text);
            outputText=infoFile.readAll();
            infoFile.close();
        }
        // 删除临时文件
        infoFile.remove();
    }
    else if(flag == 1){
        QString cmd = QString("%1").arg(filePath) + " && pause";
        exitCode=system(cmd.toStdString().c_str());
    }
    else if(flag == 2){
        // 可执行文件路径
        QString exePath = filePath;
        // 信息文件
        QFileInfo fileInfo(filePath);
        QString outputFileName = fileInfo.path()+"info.txt";
        QProcess compileProcess;
        // 设置标准输出和标准错误输出到临时文件
        compileProcess.setStandardOutputFile(outputFileName);
        compileProcess.setStandardErrorFile(outputFileName);
        // 编译
        compileProcess.start("g++", arguments);
        compileProcess.waitForFinished();

        exitCode = compileProcess.exitCode();

        QFile infoFile(outputFileName);
        if (exitCode == 0)
        {
            QFileInfo info(exePath);
            qint64 size = info.size();
            const QStringList sizeUnits = {"B", "KB", "MB", "GB"};
            int unitIndex = 0;
            while (size > 1024 && unitIndex < sizeUnits.size() - 1) {
                size /= 1024;
                unitIndex++;
            }
            outputText="编译成功\n""输出文件名："+exePath+"\n"
                       "创建时间："+info.birthTime().toString("yyyy-MM-dd hh:mm:ss") +"\n"
                       "最后修改时间："+info.lastModified().toString("yyyy-MM-dd hh:mm:ss")+"\n"
                       "文件大小："+QString::number(size)+sizeUnits[unitIndex]+"\n";
        }
        else
        {
            // 读取编译错误信息
            infoFile.open(QIODevice::ReadOnly | QIODevice::Text);
            outputText=infoFile.readAll();
            infoFile.close();
        }
        // 删除临时文件
        infoFile.remove();
    }
    // 发射完成信号，通知主界面
    emit compilationFinished(exitCode, outputText); // 这里可以设置编译结果的文本
    quit();
}
