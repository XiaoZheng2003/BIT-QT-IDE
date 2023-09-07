#ifndef ASTYLE_H
#define ASTYLE_H

#include "headers.h"
#include "tab.h"


namespace Ui {
class AStyle;
}

class AStyleThread;

class AStyle : public QDialog
{
    Q_OBJECT

public:
    static AStyle *getInstance(QWidget *parent);
    static void aStyleFile(int id, Tab *targetTab, const QString &text);

public slots:


private slots:
    void on_buttonBox_accepted();

private:
    Ui::AStyle *ui;
    static AStyle *m_aStyle;
    bool m_isDefault;
    QHash<QString,QString> m_optionList; //自定义的格式列表

    explicit AStyle(QWidget *parent = nullptr);
    ~AStyle();
};

class AStyleThread : public QThread
{
    Q_OBJECT
public:
    AStyleThread(int id, bool isDefault, const QHash<QString,QString> &optionList, const QString &text);

protected:
    void run() override;

signals:
    void aStyleFinished(QString str);
    void sendError(int type, QString info);

private:
    int m_id;
    QString m_text;
    bool m_isDefault; // 是否使用预设的格式
    QHash<QString,QString> m_optionList; //自定义的格式列表
};

#endif // ASTYLE_H
