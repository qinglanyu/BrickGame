#include "CDbgScreen.h"

CDbgScreen::CDbgScreen(QWidget *parent)
    : QTextEdit(parent)
{

}

void CDbgScreen::append(const QString &text)
{
    QTextEdit::append(text);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}
