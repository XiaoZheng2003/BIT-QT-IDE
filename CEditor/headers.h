#ifndef HEADERS_H
#define HEADERS_H

#include <QDir>
#include <QMap>
#include <QChar>
#include <QFile>
#include <QFont>
#include <QList>
#include <QLabel>
#include <QStack>
#include <QTimer>
#include <QDialog>
#include <QObject>
#include <QString>
#include <QThread>
#include <QWidget>
#include <QPainter>
#include <QProcess>
#include <QDateTime>
#include <QByteArray>
#include <QCompleter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCodec>
#include <QCloseEvent>
#include <QFileDialog>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QStyleOption>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QTreeWidgetItem>
#include <QDesktopServices>
#include <QStringListModel>
#include <QFileIconProvider>
#include <QRegularExpression>
#include <QSyntaxHighlighter>

enum class EditType{
    Redo,
    Undo,
    Cut,
    Copy,
    Paste,
    SelectAll
};

#endif // HEADERS_H
