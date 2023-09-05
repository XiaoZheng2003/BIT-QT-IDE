#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;//引入高亮规则


    keywordFormat.setForeground(Qt::darkBlue);//设置关键字颜色
    keywordFormat.setFontWeight(QFont::Bold);//设置关键字粗体样式
    QStringList keywordPatterns;
    keywordPatterns << "\\bif\\b" << "\\belse\\b" << "\\bwhile\\b"
                    << "\\bsigned\\b" << "\\bthrow\\b" << "\\bunion\\b"<< "\\bthis\\b"
                    << "\\bint\\b" << "\\bchar\\b" << "\\bdouble\\b"<< "\\bunsigned\\b"
                    << "\\bconst\\b" << "\\bgoto\\b" << "\\bvirtual\\b"
                    << "\\bfor\\b" << "\\bfloat\\b" << "\\bbreak\\b"
                    << "\\bauto\\b" << "\\bclass\\b" << "\\boperator\\b"
                    << "\\bcase\\b" << "\\bdo\\b" << "\\blong\\b" << "\\btypedef\\b"
                    << "\\bstatic\\b" << "\\bfriend\\b" << "\\btemplate\\b"
                    << "\\bdefault\\b" << "\\bnew\\b" << "\\bvoid\\b"
                    << "\\benum\\b"<< "\\binline\\b"<< "\\btry\\b"<< "\\bshort\\b"
                    << "\\bcontinue\\b"<< "\\bsizeof\\b"<< "\\bswitch\\b"
                    << "\\bprivate\\b"<< "\\bprotected\\b"<< "\\basm\\b"
                    << "\\bwhile\\b"<< "\\bcatch\\b"<< "\\bdelete\\b"
                    << "\\bpublic\\b"<< "\\bvolatile\\b"<< "\\bstruct\\b"
                    << "\\bregister\\b" << "\\bextern\\b" << "\\breturn\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }



    //类名
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    //函数名
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt:: darkYellow);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);


    //头文件
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\"[^\n]*\"");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("^\\s*#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    //多行注释
    multiLineCommentFormat.setForeground(Qt::darkGreen);


//    quotationFormat.setForeground(Qt::green);
//    rule.pattern = QRegularExpression("\".*\"");
//    rule.format = quotationFormat;
//    highlightingRules.append(rule);



    //定义多行注释的标志
   // commentStartExpression = QRegularExpression("/\\*");
     commentStartExpression = QRegularExpression ("^\\s*/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    //单行注释
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    //遍历每个规则
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);
    //多行注释的处理
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
