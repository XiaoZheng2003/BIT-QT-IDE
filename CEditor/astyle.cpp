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
    m_chineseToEnglish.insert("不缩进","None");
    m_chineseToEnglish.insert("缩进嵌套的命名空间","Inner");
    m_chineseToEnglish.insert("缩进所有命名空间","All");
    connect(ui->defaultStyle,&QComboBox::currentTextChanged,this,[=](const QString &text){
        if(text == "自定义"){
            return;
        }
        m_optionList.insert("default",text);
        qDebug()<<text;
        outputPreset();
        readPrests();
    });
    outputPreset();
}

AStyle::~AStyle()
{
    delete ui;
}

void AStyle::outputPreset()
{
    QProcess *presetProcess = new QProcess();
    QStringList parameterList;
    QString outputFileName = ".clang-format";
    QFile outputFile(outputFileName);
    outputFile.remove();
    parameterList << "-style=" +  m_optionList.value("default")  << "-dump-config";
    QString command = ".\\clang-format.exe";
    presetProcess->setStandardOutputFile(outputFileName);
    presetProcess->setStandardErrorFile(outputFileName);
    presetProcess->start(command, parameterList);
    presetProcess->waitForFinished();
    presetProcess->close();
    presetProcess->deleteLater();
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

void AStyle::readPrests()
{
    QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>(QString());
    QList<QSpinBox *> spinBoxes = this->findChildren<QSpinBox *>(QString());
    QComboBox *comboBox = ui->NamespaceIndentation;
    QFile presetFile(".clang-format");
    if(presetFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&presetFile);
        QString line;
        while(!in.atEnd()){
            line = in.readLine();
            for(int i=0;i<checkBoxes.count();i++){
                if(line.contains(checkBoxes[i]->objectName())){
                    QString value = line.split(" ").last();
                    if(value=="true"){
                        checkBoxes[i]->setChecked(true);
                    }
                    else{
                        checkBoxes[i]->setChecked(false);
                    }
                }
            }
            for(int i=0;i<spinBoxes.count();i++){
                if(line.contains(spinBoxes[i]->objectName())){
                    int value = line.split(" ").last().toInt();
                    spinBoxes[i]->setValue(value);
                }
            }
            if(line.contains(comboBox->objectName())){
                QString value = line.split(" ").last();
                for(int i = 0; i < comboBox->count();i++){
                    if(value == m_chineseToEnglish.value(comboBox->itemText(i))){
                        comboBox->setCurrentIndex(i);
                    }
                }
            }
        }
        presetFile.close();
    }
}

void AStyle::setPresets()
{
    QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>(QString());
    QList<QSpinBox *> spinBoxes = this->findChildren<QSpinBox *>(QString());
    QComboBox *comboBox = ui->NamespaceIndentation;
    QFile presetFile(".clang-format");
    QFile tempFile("temp.clang-format");
    if(presetFile.open(QIODevice::ReadOnly | QIODevice::Text) && tempFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream in(&presetFile);
        QTextStream out(&tempFile);
        QString line;
        QString output;
        while(!in.atEnd()){
            line = in.readLine();
            output = line + "\n";
            for(int i=0;i<checkBoxes.count();i++){
                if(line.contains(checkBoxes[i]->objectName())){
                    int pos = line.lastIndexOf(" ",-1);
                    if(checkBoxes[i]->isChecked()){
                        output = line.replace(pos + 1, line.length() - pos - 1, "true") + "\n";
                    }
                    else{
                        output = line.replace(pos + 1, line.length() - pos - 1, "false") + "\n";
                    }
                }
            }
            for(int i=0;i<spinBoxes.count();i++){
                if(line.contains(spinBoxes[i]->objectName())){
                    int pos = line.lastIndexOf(" ",-1);
                    output = line.replace(pos + 1, line.length() - pos - 1, QString::number(spinBoxes[i]->value())) + "\n";
                }
            }
            if(line.contains(comboBox->objectName())){
                QString value = line.split(" ").last();
                int pos = line.lastIndexOf(" ",-1);
                output = line.replace(pos + 1, line.length() - pos - 1, m_chineseToEnglish.value(comboBox->currentText())) + "\n";
            }
            out<<output;
        }
        out.flush();
        presetFile.close();
        presetFile.remove();
        tempFile.close();
        tempFile.rename(".clang-format");
    }
}

void AStyle::on_buttonBox_accepted()
{
    setPresets();
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
        parameterList << "-style=file"  << "-i" << tempFileName;
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
            if(!errorFile->open(QIODevice::ReadOnly | QIODevice::Text)){
                emit sendError(0,"");
                tempFile->remove();
                tempFile->deleteLater();
                errorFile->remove();
                errorFile->deleteLater();
                return;
            }
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
        tempFile->remove();
        tempFile->deleteLater();
        errorFile->close();
        errorFile->remove();
        errorFile->deleteLater();
    }
    else{

    }
    quit();
}

void AStyle::on_editPresetButton_clicked()
{
    ui->defaultStyle->setCurrentText("自定义");
    this->close();
    emit openAStylePresetFile(".clang-format");
}

