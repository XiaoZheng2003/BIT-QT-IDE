#include "astyle.h"
#include "ui_astyle.h"

AStyle *AStyle::m_aStyle = nullptr;

AStyle::AStyle(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AStyle)
{
    ui->setupUi(this);
    m_isDefault = true;
    m_optionList.insert("default","llvm");
}

AStyle::~AStyle()
{
    delete ui;
}

AStyle *AStyle::getInstance(QWidget *parent)
{
    if(m_aStyle == nullptr){
        m_aStyle = new AStyle(parent);
    }
    return m_aStyle;
}

void AStyle::aStyleFile(int id, Tab *targetTab, const QString &text)
{
    AStyleThread *aStyleThread = new AStyleThread(id, m_aStyle->m_isDefault, m_aStyle->m_optionList, text);
    connect(aStyleThread, &AStyleThread::aStyleFinished, targetTab, &Tab::receiveAStyledText);
    connect(aStyleThread,&AStyleThread::sendError,targetTab,&Tab::handleAStyleError);
    connect(aStyleThread, &QThread::finished, aStyleThread, &QThread::deleteLater);
    aStyleThread->start();
}

void AStyle::on_buttonBox_accepted()
{
    if(ui->defaultStyle->isEnabled()){
        m_optionList.insert("default",ui->defaultStyle->currentText().toLower());
    }
    else{
//        QProcess *aStyleProcess = new QProcess();
//        QString command = ".\\clang-format.exe -style=llvm -dump-config > .clang-format";
//        command += ui->defaultStyle->currentText().toLower();
//        command += " -i .clang-format";
//        aStyleProcess->setProgram(command);
//        aStyleProcess->start();
    }
}

AStyleThread::AStyleThread(int id, bool isDefault, const QHash<QString,QString> &optionList, const QString &text)
{
    m_id = id;
    m_isDefault = isDefault;
    m_optionList = optionList;
    m_text=text;
}

void AStyleThread::run()
{
    if(m_isDefault){
        //临时文件
        QString tempFileName = "temp" + QString::number(m_id) + ".cpp";
        QFile *tempFile = new QFile(tempFileName);
        QFileInfo info(tempFileName);
        //输出错误文件
        QString errorFileName = "error" + QString::number(m_id) + ".txt";
        if(!tempFile->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit sendError(0, "");
            return;
        }
        //将文本保存在临时文件中，采用UTF-8编码
        QByteArray strBytes;
        strBytes = m_text.toUtf8();
        tempFile->write(strBytes,strBytes.length());
        tempFile->close();
        QProcess *aStyleProcess = new QProcess();
        QStringList parameterList; // 参数列表
        //格式化临时文件的命令及参数
        parameterList << "-style=" +  m_optionList.value("default")  << "-i" << tempFileName;
        QString command = ".\\clang-format.exe";
        //设置报错输出文件
        aStyleProcess->setStandardOutputFile(errorFileName);
        aStyleProcess->setStandardErrorFile(errorFileName);
        aStyleProcess->start(command, parameterList);
        aStyleProcess->waitForFinished();


        int exitCode = aStyleProcess->exitCode();

        QFile *errorFile = new QFile(errorFileName);

        if (exitCode == 0)//格式化成功
        {
            if(!tempFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                emit sendError(0, "");
                tempFile->remove();
                tempFile->deleteLater();
                errorFile->remove();
                errorFile->deleteLater();
                return;
            }
            //成功读取格式化成功的文件
            QByteArray strBytes = tempFile->readAll();
            tempFile->close();
            tempFile->remove();
            tempFile->deleteLater();
            errorFile->remove();
            errorFile->deleteLater();
            //转换为utf-8编码
            QTextCodec::ConverterState state;
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            QString strtext = codec->toUnicode( strBytes.constData(), strBytes.size(), &state);
            if (state.invalidChars > 0)
                strtext = QTextCodec::codecForName( "GBK" )->toUnicode(strBytes);
            else
                strtext = strBytes;
            emit aStyleFinished(strtext);
        }
        else
        {
            // 读取报错信息
            errorFile->open(QIODevice::ReadOnly | QIODevice::Text);
            QByteArray strBytes = errorFile->readAll();
            //转换为utf-8编码
            QTextCodec::ConverterState state;
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            QString strtext = codec->toUnicode( strBytes.constData(), strBytes.size(), &state);
            if (state.invalidChars > 0)
                strtext = QTextCodec::codecForName( "GBK" )->toUnicode(strBytes);
            else
                strtext = strBytes;
            qDebug()<<strtext;
            emit sendError(1, strtext);
        }
        tempFile->close();
        errorFile->close();
        tempFile->remove();
        tempFile->deleteLater();
        errorFile->remove();
        errorFile->deleteLater();
    }
    quit();
}
