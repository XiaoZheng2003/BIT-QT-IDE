#ifndef ASTYLE_H
#define ASTYLE_H

#include <QDialog>

namespace Ui {
class AStyle;
}

class AStyle : public QDialog
{
    Q_OBJECT

public:
    explicit AStyle(QWidget *parent = nullptr);
    ~AStyle();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::AStyle *ui;
};

#endif // ASTYLE_H
