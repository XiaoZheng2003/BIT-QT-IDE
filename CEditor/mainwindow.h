#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tab.h"

#include <QDir>
#include <QList>
#include <QTextCodec>
#include <QByteArray>
#include <QFileDialog>
#include <QMainWindow>
#include "tab.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void returnTextForSave(int indexId, QString str);
    void returnTextForSaveAs(int indexId, QString str);
    void tabTextChanged(int index);

private slots:
    void on_actionOpen_triggered();
    void on_actionNewFile_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSaveas_triggered();
    void on_actionCloseAll_triggered();

    void on_compile_triggered();

    void on_run_triggered();

    void on_c_r_triggered();

signals:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);

private:
    Ui::MainWindow *ui;
    QList<QString> filePath;

    void refreshFilename(int index);
    void initConnection(Tab *tab);
    void closeTab(int index);
    QString getCorrentUnicode(const QByteArray &text);
};
#endif // MAINWINDOW_H
