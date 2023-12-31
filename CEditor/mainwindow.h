﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tab.h"
#include "search.h"
#include "headers.h"
#include "replace.h"
#include "astyle.h"
#include "compilationthread.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
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
    void updateCursorReceive(int row, int col);
    void totalCountReceive(int count);

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
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSearch_triggered();
    void on_actionReplace_triggered();
    void receiveStartSearchDataForMain(QString data,int state,int begin);   //从搜索框接收到开始消息
    void receiveNextSearchDataForMain(QString data,int state);   //从搜索框接收到下一条消息
    void receiveCloseSearchDataForMain();//关闭搜索对话框
    void receiveAllReplaceDataForMain(QString sear,QString rep,int state); //从替换框接受到消息
    void receiveNextReplaceDataForMain(QString sear,QString rep,int state); //从替换框接受到消息
    void on_actionCompileProject_triggered();
    void on_actionRunProject_triggered();
    void on_actionCompileRunProject_triggered();
    void on_actionAutoComplete_triggered();
    void on_actionSelectAll_triggered();
    void on_actionComment_triggered();
    void on_actionJumpLine_triggered();
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_functionTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_actionAstyle_triggered();
    void on_actionAstyleOption_triggered();

signals:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);
    void editOperate(int indexId, EditType type);
    void sendStartSearchDataToTab(QString,int,int state,int);  //把从搜索框接受的信息转发给指定页面
    void sendNextSearchDataToTab(QString,int,int state);  //把从搜索框接受的信息转发给指定页面
    void sendCloseSearchDataToTab();  //把从搜索框接受的信息转发给指定页面
    void sendAllReplaceDataToTab(QString,QString,int,int state);  //把从替换框接受的信息转发给指定页面
    void sendNextReplaceDataToTab(QString,QString,int,int state);  //把从替换框接受的信息转发给指定页面
    void autoComplete(int indexId);
    void commentSelectedLines(int indexId);
    void jumpToLine(int indexId, int line);
    void startAStyle(int indexId);
    void openAStyleOption();

private:
    Ui::MainWindow *ui;
    QList<QString> filePath;
    QMap<QString, QString> projectNameToPath;
    QMap<QString, QString> fileNameToPath;
    QMap<QString, QStringList> projectToChildren;
    Search *search;//搜索对话框
    replace *replace;//替换对话框
    AStyle *m_astyle;//格式化对话框
    QLabel *row_col;
    QLabel *all_row;
    QMap<QString,int> findFunction; // 通过函数名找函数行号

    void refreshFilename(int index);
    void initConnection(Tab *tab);
    void openFile(QString openFilePath);
    void saveAllFile();
    bool closeTab(int index);
    void closeProject(QTreeWidgetItem *project);
    void removeItem(QTreeWidgetItem *item);
    QString getCorrentUnicode(const QByteArray &text);
    void createProjectTree(QTreeWidgetItem *root, QString projectName, QString projectPath);
    void handleCompilationFinished(int exitCode, const QString &outputText);
    void handleRunFinished(int exitCode, const QString &outputText);
    void handleProjectCompilationFinished(int exitCode, const QString &outputText);
    void handleProjectRunFinished(int exitCode, const QString &outputText);
    void initProjectTreeMenu();
    int getItemLevel(QTreeWidgetItem *item);
    void createFunctionTree(int index);
};
#endif // MAINWINDOW_H
