#include "CWidget.h"

#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QScreen *screen = a.primaryScreen();
    QSize scrSize = screen->size();

    qreal l=0, t=0;
    qreal w0=0, h0=0;
    qreal dvcLWRatio = 1;
    dvcScrType scrType = Longer;

#ifndef ANDROID
    //scrSize = QSize(1152, 2048)*.4;
    scrSize = QSize(1152, 2048)*.4;
#endif

    w0 = scrSize.width();
    h0 = scrSize.height();

    dvcLWRatio = h0 / w0;
    scrType = (dvcLWRatio > gIdealLWRatio ? Longer : Wider);
    l = (scrType==Wider) ? ((w0 - h0*gIdealX/gIdealY)/2) : (l);
    t = (scrType==Wider) ? (t) : ((h0 - w0*gIdealY/gIdealX)/2);

    CWidget w(scrSize.width(), scrSize.height(), scrType, l, t);
    w.setWindowFlags(Qt::FramelessWindowHint);

#ifdef ANDROID
    w.showFullScreen();
#else
    w.show();
#endif

    return a.exec();
}
