#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QScrollBar>
#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
#include <QTimer>

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent=nullptr);
    void setLineNumberArea(QListWidget *lineNumberArea);

public slots:
    void updateLineNumberArea();

protected:
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event) override;
    //void paintEvent(QPaintEvent *event) override;

signals:
    void scrollBarValue(int value);

private:
    QListWidget *m_lineNumberArea;
    void sendCurrentScrollBarValue();
};

#endif // CODEEDITOR_H
