#ifndef OPENPROMPT_H
#define OPENPROMPT_H

#include <QtWidgets>
#include <QtGui>
#include <QDialog>

class openPrompt : public QDialog {
    Q_OBJECT

public:
    openPrompt();

private:
    // Widgets
    QLabel      *label;
    QPushButton *yes;
    QPushButton *no;

signals:
    // Signal
    void load();

private slots:
    // Slots
    void s_yesPressed();
    void s_noPressed();

};

#endif
