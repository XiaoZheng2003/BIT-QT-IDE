#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tab.h"

#include <QDir>
#include <QMap>
#include <QList>
#include <QProcess>
#include <QDateTime>
#include <QTextCodec>
#include <QByteArray>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QFileIconProvider>

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

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionOpen_triggered();
    void on_actionNewFile_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSaveas_triggered();
    void on_actionCloseAll_triggered();
    void on_actionCompile_triggered();
    void on_actionRun_triggered();
    void on_actionCompileRun_triggered();
    void on_actionOpenProject_triggered();
    void on_compileInfoButton_clicked();
    void on_projectTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_tabWidget_tabCloseRequested(int index);

signals:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);

private:
    Ui::MainWindow *ui;
    QList<QString> filePath;
    QMap<QString,QString> projectNameToPath;
    QMap<QString,QString> fileNameToPath;
    QMap<QString,QStringList> projectToChildren;

    void refreshFilename(int index);
    void initConnection(Tab *tab);
    void openFile(QString openFilePath);
    void closeTab(int index);
    QString getCorrentUnicode(const QByteArray &text);
    void createProjectTree(QTreeWidgetItem *root,QString projectName,QString projectPath);
};
#endif // MAINWINDOW_H
