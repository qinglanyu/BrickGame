#ifndef CDBGSCREEN_H
#define CDBGSCREEN_H

#include <QWidget>
#include <QTextEdit>

class CDbgScreen : public QTextEdit
{
    Q_OBJECT
public:
    explicit CDbgScreen(QWidget *parent = nullptr);

    void append(const QString &text);

signals:

};

extern CDbgScreen *pDbg;

#endif // CDBGSCREEN_H
