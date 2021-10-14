#include "CWidget.h"
#include "CGameManager.h"
#include "CJBtn.h"
#include "Games/Tetris/CTetris.h"

#include <QPainter>
#include <QDebug>
#include <QPixmap>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QFontDatabase>
#include <QApplication>

#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#ifdef ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>

using namespace QtAndroid;

#define CHECK_EXCEPTION() \
    if(env->ExceptionCheck())\
    {\
        qDebug() << "exception occured";\
        env->ExceptionDescribe();\
        env->ExceptionClear();\
    }
#endif

CWidget::CWidget(int w, int h, dvcScrType scrType, qreal l, qreal t, QWidget *parent)
    : QWidget(parent)
    , m_width(w)
    , m_height(h)
    , m_scrType(scrType)
    , m_lMargin(l)
    , m_tMargin(t)
    , m_pxbgr(nullptr)
    , m_bVoiceOn(true)
    , m_bPaused(true)
    , m_bVibrateOn(true)
    , m_scrPage(sp_game)
    , m_theme(&themes[0])
{
    this->setFixedSize(w, h);

    // init media player
    m_player = new QMediaPlayer;
    m_pGameMgr = new CGameManager;

    InitUI();

    InitConnectors();

    InitGameMgrPos();

    SetTheme(0);
    slotRefresh();
    slotPlayAudio(ai_bgm);
}

CWidget::~CWidget()
{
    if(m_pGameMgr != nullptr)
    {
        delete m_pGameMgr;
        m_pGameMgr = nullptr;
    }

    if(m_pxbgr != nullptr)
    {
        delete m_pxbgr;
        m_pxbgr = nullptr;
    }
}

void CWidget::InitUI()
{
    m_dvcXFactor = qreal(m_width)/gIdealX;
    m_dvcYFactor = qreal(m_height)/gIdealY;

    static QString fontRes(":/font/font/digital-7.ttf");
    static int fontId = QFontDatabase::addApplicationFont(fontRes);
    static QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);

    static QString fontItalic(":/font/font/digital-7 (italic).ttf");
    static int fontItalicId = QFontDatabase::addApplicationFont(fontItalic);
    static QString fontItalicName = QFontDatabase::applicationFontFamilies(fontItalicId).at(0);

#ifdef ANDROID
    int fontPointSz = int(22*m_dvcXFactor);
#else
    int fontPointSz = int(36*m_dvcXFactor);
#endif

    m_digitFont = QFont(fontName, fontPointSz);
    m_digitFntItalic = QFont(fontItalicName, fontPointSz);

    InitPositions();

    InitGameBtns();
}
#include <QTime>
void CWidget::paintEvent(QPaintEvent *e)
{
    QRect eventRt = e->rect();
    if(eventRt != m_updateRt && eventRt != rect())
        return;

    QTime t0;
    t0.start();
    QPainter painter(this);

    switch (m_scrPage)
    {
    case sp_game:   DrawGamePage(painter); break;
    case sp_setting:DrawSettingPage(painter); break;
    case sp_exit:   DrawExitPage(painter); break;
    default:break;
    }

    qDebug() << "paintEvent cost " << t0.elapsed() << endl;
}

void CWidget::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();

    switch (key)
    {
    case Qt::Key_Up:
    case Qt::Key_W:
        emit sigPushButton(CGameManager::MD_Up);
        break;

    case Qt::Key_Left:
    case Qt::Key_A:
        emit sigPushButton(CGameManager::MD_Left);
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        emit sigPushButton(CGameManager::MD_Right);
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        emit sigPushButton(CGameManager::MD_Down);
        break;

    case Qt::Key_Space:
        emit sigPushButton(CGameManager::MD_Floor);
        break;
    }

    QWidget::keyPressEvent(event);
}

void CWidget::mousePressEvent(QMouseEvent *event)
{
#ifdef ANDROID
    Q_UNUSED(event)
#else
    m_wndPos = event->globalPos();
#endif
}

void CWidget::mouseMoveEvent(QMouseEvent *event)
{
#ifdef ANDROID
    Q_UNUSED(event)
#else
    QPoint offset = event->globalPos() - m_wndPos;
    m_wndPos = event->globalPos();
    move(offset + QPoint(x(), y()));
#endif
}

void CWidget::mouseReleaseEvent(QMouseEvent *event)
{
#ifdef ANDROID
    Q_UNUSED(event)
#else
    QPoint offset = event->globalPos() - m_wndPos;
    move(offset + QPoint(x(), y()));
#endif
}

void CWidget::DrawGamePage(QPainter &painter)
{
    DrawGameText(painter);
    DrawGameBoard(painter);
    DrawMovingShape(painter);
    DrawPreviewBoard(painter);
    DrawSoundSymb(painter);
    DrawPausedSymb(painter);
}

void CWidget::DrawSettingPage(QPainter &painter)
{
    painter.fillRect(m_updateRt, gameboardBgr);
    static QPixmap Share(":/img/img/ShareBtn.png");
    static QPixmap Leaderboard(":/img/img/LeaderBoard.png");
    static QPixmap Gift(":/img/img/gift.png");
    static QPixmap SignIn(":/img/img/SignIn.png");

#ifdef ANDROID
    int settingPtSz = int(12 * m_dvcXFactor);
#else
    int settingPtSz = int(22 * m_dvcXFactor);
#endif

    QFont font("Arial", settingPtSz);
    font.setItalic(true);
    painter.setFont(font);

    QSize sz(112, 112);  sz *= m_dvcXFactor;
    int w = 112 * m_dvcXFactor;
    int h = w;
    int m = (m_updateRt.width() - 3*w)/4;
    int x0 = m_updateRt.x();
    int x = x0+m;
    int y = 175*m_dvcXFactor;

    QRect rt1 = QRect(x, 180*m_dvcXFactor, w, h);  x = x0 + 2*m + w;
    QRect rt2 = QRect(x, 180*m_dvcXFactor, 128*m_dvcXFactor, 100*m_dvcXFactor);  x = x0 + 3*m + 2*w;
    QRect rt3 = QRect(x, y, 128*m_dvcXFactor, 128*m_dvcXFactor);
    painter.drawPixmap(rt1, Share, Share.rect());
    painter.drawPixmap(rt2, Leaderboard, Leaderboard.rect());
    painter.drawPixmap(rt3, Gift, Gift.rect());
    x = x0 + (m_updateRt.width() - w)/2;
    y = 383*m_dvcXFactor;
    QRect rt4 = QRect(x, y, w, h);
    painter.drawPixmap(rt4, SignIn, SignIn.rect());

    x = rt1.x();
    y = rt1.bottom();
    QRect rt5 = QRect(x, y, w, h);
    painter.drawText(rt5, Qt::AlignCenter, "Share");

    x = rt1.right();
    QRect rt6 = QRect(x, y, w+2*m, h);
    painter.drawText(rt6, Qt::AlignCenter, "Leaderboard");

    x = rt2.right();
    QRect rt7 = QRect(x, y, w+2*m, h);
    painter.drawText(rt7, Qt::AlignCenter, "More Games");

    x = x0, y = 464*m_dvcXFactor, w = m_updateRt.width(), h *= 1.5;
    QRect rt8 = QRect(x, y, w, h);
    painter.drawText(rt8, Qt::AlignCenter, "SignIn Google\nPlay servies");

    y = 602*m_dvcXFactor;
    QRect rt9 = QRect(x, y, w, h);

    font.setPointSize(settingPtSz*1.5);
    painter.setFont(font);
    painter.drawText(rt9, Qt::AlignCenter, "Select your favorite theme");

    /*QVector<QRect> rts;
    rts << rt1 << rt2 << rt3 << rt4 << rt5 << rt6 << rt7 << rt8 << rt9;
    painter.setPen(Qt::red);
    painter.drawRects(rts);*/
}

void CWidget::DrawExitPage(QPainter &painter)
{
    painter.fillRect(m_updateRt, gameboardBgr);

    static QFont exitFont(m_digitFont);
    exitFont.setPointSize(40);
    painter.setFont(exitFont);
    const static QString text("EXIT  GAME\n----------\n| YES | NO  |\n----------\n");
    painter.drawText(m_updateRt.adjusted(0,100,0,-100), Qt::AlignCenter, text);
}

void CWidget::PasteBgrImg(QPainter &painter)
{
    QPixmap pixmap(m_theme->bgr);

    qreal h0 = m_width*1290/gIdealX;
    QPointF pt0(0,0), pt1(m_width, h0);
    painter.drawPixmap(QRectF(pt0, pt1), pixmap, QRectF(0,0,gIdealX, 1290));

    pt0 = QPoint(0, h0);
    pt1 = QPoint(m_width, m_height);
    QPixmap px2 = pixmap.copy(QRect(0,1290, gIdealX, gIdealY));
    painter.drawPixmap(QRectF(pt0, pt1), px2, px2.rect());


    /*qreal h0 = m_width*1080/gIdealX;
    qreal h1 = (1290-1080)*m_width/gIdealX;
    qreal h2 = (565-48)*m_width/1080;
    qreal vMargin = (m_height - h0 - h1 - h2)/3;

    QPointF pt0(0,0), pt1(m_width, h0);
    painter.drawPixmap(QRectF(pt0, pt1), pixmap, QRectF(0,0,gIdealX,1080));

    pt0 = QPoint(0, h0);
    pt1 = QPoint(m_width, h0+vMargin);
    QPixmap px2 = pixmap.copy(QRect(0,1080, gIdealX, 1085));
    painter.drawPixmap(QRectF(pt0, pt1), px2, px2.rect());

    QPixmap px3 = pixmap.copy(QRect(0,1080, gIdealX, 1290));
    pt0 = QPoint(0, pt1.y());
    pt1 = QPoint(m_width, h0+vMargin+px3.height()*m_width/gIdealX);
    painter.drawPixmap(QRectF(pt0, pt1), px3, px3.rect());

    pt0 = QPoint(0, pt1.y());
    pt1 = QPoint(m_width, m_height);
    QPixmap px4 = pixmap.copy(QRect(0,1290, gIdealX,gIdealY));
    painter.drawPixmap(QRectF(pt0, pt1), px4, px4.rect());*/
}

void CWidget::DrawStaticText(QPainter &painter)
{
    // draw button tips
#ifdef ANDROID
    int fontPointSize = int(20 * m_dvcXFactor);
    int settingPtSz = int(12 * m_dvcXFactor);
#else
    int fontPointSize = int(40 * m_dvcXFactor);
    int settingPtSz = int(22 * m_dvcXFactor);
#endif
    painter.setPen(Qt::black);
    painter.setFont(m_digitFntItalic);
    //painter.drawText(m_titleRt, Qt::AlignTop|Qt::AlignHCenter, "PERSEUS");
    painter.drawText(m_titleRt, Qt::AlignTop|Qt::AlignHCenter, "GOOD BOY");

    QFont font("Arial", fontPointSize);
    font.setItalic(true);
    font.setPointSize(settingPtSz);
    font.setBold(true);
    painter.setFont(font);

    QRect btnRt = m_pStartBtn->frameGeometry();
    QPoint topLeft = btnRt.bottomLeft() + QPoint(-btnRt.width()/2, int(10*m_dvcYFactor));
    QRectF btnTipRect(topLeft, btnRt.size()*2);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Start\nPause");

    int nSpace = m_pSoundBtn->frameGeometry().x() - m_pStartBtn->frameGeometry().x();
    topLeft += QPoint(nSpace, 0);
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Sound\nVibrate");

    topLeft += QPoint(nSpace, 0);
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Setting");

    topLeft += QPoint(nSpace, 0);
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Exit\nGame");

    nSpace = int(10*m_dvcXFactor);
    topLeft = m_pUpBtn->frameGeometry().bottomLeft() + QPoint(0, 0);
    btnTipRect.setWidth(m_pUpBtn->width());
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Level");

    topLeft = m_pLeftBtn->frameGeometry().bottomLeft() + QPoint(0, int(nSpace*.6));
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Prev\nGame");

    topLeft = m_pRightBtn->frameGeometry().bottomLeft() + QPoint(0, int(nSpace*.6));
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Next\nGame");

    topLeft = m_pDownBtn->frameGeometry().bottomLeft() + QPoint(0, nSpace);
    btnTipRect.moveTo(topLeft);
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Speed");

    topLeft = m_pRotateBtn->frameGeometry().bottomLeft() + QPoint(0, nSpace);
    btnTipRect.moveTo(topLeft);
    btnTipRect.setWidth(m_pRotateBtn->width());
    painter.drawText(btnTipRect, Qt::AlignTop|Qt::AlignHCenter, "Enter");

    qreal margin = 30*m_dvcXFactor;
    qreal x0 = m_pLeftBtn->frameGeometry().right()+margin;
    qreal x1 = m_pRightBtn->frameGeometry().left()-margin;
    qreal y0 = m_pUpBtn->frameGeometry().bottom()+margin*1.5;
    qreal y1 = m_pDownBtn->frameGeometry().top()-margin*.2;

    QRectF rt(QPointF(x0, y0), QPointF(x1, y1));
    QPixmap arrows(":/img/img/ic-arrows.png");
    painter.drawPixmap(rt, arrows, arrows.rect());
}

void CWidget::DrawGameText(QPainter &painter)
{
    int nScore = (m_pGameMgr==nullptr? 0 : m_pGameMgr->GetScore());
    int nLevel = (m_pGameMgr==nullptr? 0 : m_pGameMgr->GetLevel());
    int nSpeed = (m_pGameMgr==nullptr? 0 : m_pGameMgr->GetSpeed());
    int nGoal  = 0;

#ifdef ANDROID
    int fontPointSz = int(18*m_dvcXFactor);
#else
    int fontPointSz = int(36*m_dvcXFactor);
#endif

    painter.setPen(Qt::black);
    m_digitFont.setPointSize(fontPointSz);
    painter.setFont(m_digitFont);
    painter.drawText(m_scoreStrRt, Qt::AlignCenter, "SCORE");
    painter.drawText(m_hiScoreStrRt, Qt::AlignCenter, "HI-SCORE");
    painter.drawText(m_goalStrRt, Qt::AlignCenter, "G O A L");
    painter.drawText(m_lvlStrRt, Qt::AlignRight|Qt::AlignVCenter, "LEVEL");
    painter.drawText(m_spdStrRt, Qt::AlignRight|Qt::AlignVCenter, "SPEED");

    m_digitFntItalic.setPointSize(fontPointSz);
    painter.setFont(m_digitFntItalic);
    painter.drawText(m_scoreValRt, Qt::AlignCenter, QString::number(nScore));
    painter.drawText(m_hiScoreValRt, Qt::AlignCenter, QString::number(nScore));
    painter.drawText(m_lvlValRt, Qt::AlignCenter, QString::number(nLevel));
    painter.drawText(m_spdValRt, Qt::AlignCenter, QString::number(nSpeed));
    painter.drawText(m_goalValRt, Qt::AlignCenter, QString("%1/20").arg(nGoal));
}

void CWidget::DrawGameBoard(QPainter &painter)
{
    if(m_pGameMgr == nullptr) return;

    GAMEBOARD pGmBd = m_pGameMgr->GetGameBoard();
    if(pGmBd == nullptr) return;

    int nRowsInGmBd = m_pGameMgr->GetGameBoardRowCount();
    int nColsInGmBd = m_pGameMgr->GetGameBoardColCount();

    // draw filled cells
    for(int i=0; i<nRowsInGmBd; ++i)
        for(int j=0; j<nColsInGmBd; ++j)
            if(pGmBd[i][j].IsFilled())
                paintCell(painter, pGmBd[i][j]);
}

void CWidget::DrawGameBoardBgr(QPainter &painter)
{
    if(m_pGameMgr == nullptr) return;

    GAMEBOARD pGmBd = m_pGameMgr->GetGameBoard();
    if(pGmBd == nullptr) return;

    int nRowsInGmBd = m_pGameMgr->GetGameBoardRowCount();
    int nColsInGmBd = m_pGameMgr->GetGameBoardColCount();

    for(int i=0; i<nRowsInGmBd; ++i)
        for(int j=0; j<nColsInGmBd; ++j)
            paintCell(painter, pGmBd[i][j]);
}

void CWidget::DrawMovingShape(QPainter &painter)
{
    GAMEBOARD pGmBd = m_pGameMgr->GetGameBoard();
    if(pGmBd == nullptr) return;

    CShape *pShape = m_pGameMgr->GetMovingShape();
    if(!pShape) return;

    CShapeState *pState = pShape->getCurrentState();
    if(!pState) return;

    CCell cell;
    for(int ii=0; ii<4; ++ii)
    {
        int xpos = pState->m_cellPoint[ii].x() + pShape->getShapePosition()->x();
        int ypos = pState->m_cellPoint[ii].y() + pShape->getShapePosition()->y();

        if(0<= xpos && xpos < m_pGameMgr->GetGameBoardRowCount() &&
                0<=ypos && ypos < m_pGameMgr->GetGameBoardColCount())
        {
            cell = pGmBd[xpos][ypos];
            cell.SetColor(pShape->getColor());
            paintCell(painter, cell, true);
        }
    }
}

void CWidget::DrawPreviewBoard(QPainter &painter)
{
    if(m_pGameMgr == nullptr) return;

    PREVIEWBOARD pPreviewBoard = m_pGameMgr->GetPreviewBoard();
    if(pPreviewBoard == nullptr) return;

    int nPreviewBdSz = m_pGameMgr->GetPreviewBoardSize();

    for(int i=0; i<nPreviewBdSz; ++i)
        for(int j=0; j<nPreviewBdSz; ++j)
            if(pPreviewBoard[i][j].IsFilled())
                paintCell(painter, pPreviewBoard[i][j]);
}

void CWidget::DrawPreviewBoardBgr(QPainter &painter)
{
    if(m_pGameMgr == nullptr) return;

    PREVIEWBOARD pPreviewBoard = m_pGameMgr->GetPreviewBoard();
    if(pPreviewBoard == nullptr) return;

    int nPreviewBdSz = m_pGameMgr->GetPreviewBoardSize();
    for(int i=0; i<nPreviewBdSz; ++i)
        for(int j=0; j<nPreviewBdSz; ++j)
            paintCell(painter, pPreviewBoard[i][j]);
}

void CWidget::paintCell(QPainter &painter, CCell &cell, bool moving)
{
#define USE_DRAW
#ifdef USE_PAINTER
    /* paint cell */
    QRectF cellF(cell);
    QColor lineColor((moving || cell.IsFilled())? Qt::black : shadow);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(lineColor);
    painter.setPen(pen);
    painter.setBrush(lineColor);

    // draw cell boundry
    QPointF t0(cell.topLeft()), t1(cell.bottomLeft()), t2(cell.bottomRight()), t3(cell.topRight());
    QLineF lines[4] = { QLineF(t0, t1), QLineF(t1, t2), QLineF(t2, t3), QLineF(t3, t0) };
    painter.drawLines(lines, 4);

    // fill cell center
    QColor cellColor = ((moving || cell.IsFilled()) ? cell.GetColor() : shadow);
    qreal lMargin = cellF.width()*0.12;
    painter.fillRect(cellF.adjusted(lMargin,lMargin, -lMargin, -lMargin), cellColor);

#elif defined(USE_DRAW)
    /* draw cell */
    static QPixmap px1(":/img/img/dot1.png");
    static QPixmap px2(":/img/img/dot2.png");
    static QRect rt1 = px1.rect();
    static QRect rt2 = px2.rect();
    bool bDeep = (moving || cell.IsFilled());
    painter.drawPixmap(cell, bDeep ? px1 : px2, bDeep? rt1 : rt2);
#endif
}

void CWidget::DrawBoyHandsDown(QPainter &painter)
{
    QPixmap handdown(":/img/img/Boy1.png");
    qreal pxW = handdown.width();
    qreal pxH = handdown.height();
    qreal factor = m_flashRt.height() / pxH;
    handdown = handdown.scaled(int(pxW*factor),int(pxH*factor),Qt::KeepAspectRatio);
    QPointF pt = m_flashRt.topLeft() + QPointF((m_flashRt.width()-pxW*factor)/2*1.2, 0);
    painter.drawPixmap(pt, handdown);
}

void CWidget::DrawAllRects(QPainter &painter)
{
    painter.setPen(Qt::red);

    QVector<QRectF> rts;
    rts << m_titleRt << m_gmbdRt << m_prbdRt << m_updateRt << m_scoreStrRt
        << m_scoreValRt << m_hiScoreStrRt << m_hiScoreValRt << m_lvlStrRt
        << m_lvlValRt  << m_spdStrRt << m_spdValRt << m_flashRt << m_soundRt
        << m_pauseRt   << m_goalStrRt << m_goalValRt << m_pausedStrRt;
    painter.drawRects(rts);
}

void CWidget::DrawBackground()
{
    if(m_pxbgr == nullptr)
        m_pxbgr = new QPixmap(m_width, m_height);

    QPainter painter(m_pxbgr);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::transparent);

    PasteBgrImg(painter);

    painter.fillRect(m_gmbdRt, gameboardBgr);
    painter.fillRect(m_prbdRt, gameboardBgr);

    DrawGameBoardBgr(painter);

    DrawPreviewBoardBgr(painter);

    DrawStaticText(painter);

    DrawBoyHandsDown(painter);

    painter.end();

    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, QBrush(*m_pxbgr));
    setPalette(pal);
}

void CWidget::DrawSoundSymb(QPainter &painter)
{
    static QPixmap sound(":/img/img/sound.png");
    static QPixmap mute(":/img/img/mute.png");
    QPixmap *symb = (m_bVoiceOn ? &sound : &mute);
    qreal pxW = symb->width();
    qreal pxH = symb->height();
    qreal factor = m_soundRt.height() / pxH;
    *symb = symb->scaled(int(pxW*factor),int(pxH*factor),Qt::KeepAspectRatio);
    QPointF pt = m_soundRt.topLeft() + QPointF((m_soundRt.width()-pxW*factor)/2*1.5, 0);
    painter.drawPixmap(pt, *symb);
}

void CWidget::DrawPausedSymb(QPainter &painter)
{
    static QPixmap goon(":/img/img/goon.png");
    static QPixmap paused(":/img/img/PauseImage.png");
    QPixmap *symb = (m_bPaused ? &paused : &goon);
    qreal pxW = symb->width();
    qreal pxH = symb->height();
    qreal factor = m_pauseRt.height() / pxH;
    *symb = symb->scaled(int(pxW*factor), int(pxH*factor), Qt::KeepAspectRatio);
    QPointF pt = m_pauseRt.topLeft() + QPointF((m_pauseRt.width()-pxW*factor)/2*.8, 0);
    painter.drawPixmap(pt, *symb);

    int sz = m_digitFont.pointSize();

    {
#ifdef ANDROID
    int fontPointSz = int(14*m_dvcXFactor);
#else
    int fontPointSz = int(28*m_dvcXFactor);
#endif
        m_digitFont.setPointSize(fontPointSz);
        painter.setFont(m_digitFont);
        painter.drawText(m_pausedStrRt, Qt::AlignCenter, "PAUSE");
    }

    m_digitFont.setPointSize(sz);
}

void CWidget::ShowThmSelBtns(const bool bShow)
{
    m_pThmY->setEnabled(bShow);
    m_pThmR->setEnabled(bShow);
    m_pThmBk->setEnabled(bShow);
    m_pThmBl->setEnabled(bShow);
    m_pThmGr->setEnabled(bShow);

    m_pThmY->setVisible(bShow);
    m_pThmR->setVisible(bShow);
    m_pThmBk->setVisible(bShow);
    m_pThmBl->setVisible(bShow);
    m_pThmGr->setVisible(bShow);
}

void CWidget::HideThmSelBtns()
{
    ShowThmSelBtns(false);
}

void CWidget::SetTheme(const int nTheme)
{
    m_theme = &themes[nTheme];
    DrawBackground();
    static_cast<CJBtn*>(m_pRotateBtn)->setStyleImg(m_theme->ctlBtnNormal, m_theme->ctlBtnPressed);
    static_cast<CJBtn*>(m_pUpBtn)->setStyleImg(m_theme->ctlBtnNormal, m_theme->ctlBtnPressed);
    static_cast<CJBtn*>(m_pDownBtn)->setStyleImg(m_theme->ctlBtnNormal, m_theme->ctlBtnPressed);
    static_cast<CJBtn*>(m_pLeftBtn)->setStyleImg(m_theme->ctlBtnNormal, m_theme->ctlBtnPressed);
    static_cast<CJBtn*>(m_pRightBtn)->setStyleImg(m_theme->ctlBtnNormal, m_theme->ctlBtnPressed);

    static_cast<CJBtn*>(m_pStartBtn)->setStyleImg(m_theme->toolBtnNormal, m_theme->toolBtnPressed);
    static_cast<CJBtn*>(m_pSoundBtn)->setStyleImg(m_theme->toolBtnNormal, m_theme->toolBtnPressed);
    static_cast<CJBtn*>(m_pSettingBtn)->setStyleImg(m_theme->toolBtnNormal, m_theme->toolBtnPressed);
    static_cast<CJBtn*>(m_pExitBtn)->setStyleImg(m_theme->toolBtnNormal, m_theme->toolBtnPressed);

    update();
}

void CWidget::InitGameBtns()
{
    InitToolBtns();
    InitDirBtns();
    InitRotateBtn();
    InitSettingBtns();
}

void CWidget::InitToolBtns()
{
    m_pStartBtn = new CJBtn(this);
    m_pSoundBtn = new CJBtn(this);
    m_pSettingBtn = new CJBtn(this);
    m_pExitBtn = new CJBtn(this);
    startBtn = m_pStartBtn, soundBtn = m_pSoundBtn, setBtn = m_pSettingBtn, exitBtn = m_pExitBtn;

    InitToolBtn(m_pStartBtn, tr("startBtn"));
    InitToolBtn(m_pSoundBtn, tr("soundBtn"));
    InitToolBtn(m_pSettingBtn, tr("settingBtn"));
    InitToolBtn(m_pExitBtn, tr("exitBtn"));

    qreal h0 = m_width*1290/gIdealX;
    m_pStartBtn->move(565*m_width/1080, h0);
    m_pSoundBtn->move(696*m_width/1080, h0);
    m_pSettingBtn->move(827*m_width/1080, h0);
    m_pExitBtn->move(958*m_width/1080, h0);
}

void CWidget::InitToolBtn(QPushButton *pBtn, const QString &objName)
{
    pBtn->setObjectName(objName);
    int sz = 80*m_width/1080;
    pBtn->setFixedSize(sz, sz);

    CJBtn *pJBtn = static_cast<CJBtn*>(pBtn);
    pJBtn->setRepeat(false);
    connect(pJBtn, SIGNAL(sigVibrate(int)), this, SLOT(slotOnVibrate(int)));

    // connect signal and slot
    connect(pBtn, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
}

void CWidget::InitDirBtns()
{
    uBtn = m_pUpBtn = new CJBtn(this);
    dBtn = m_pDownBtn = new CJBtn(this);
    lBtn = m_pLeftBtn = new CJBtn(this);
    rBtn = m_pRightBtn = new CJBtn(this);

    InitDirBtn(m_pUpBtn, tr("upBtn"));
    InitDirBtn(m_pDownBtn, tr("downBtn"));
    InitDirBtn(m_pLeftBtn, tr("leftBtn"));
    InitDirBtn(m_pRightBtn, tr("rightBtn"));

    m_pUpBtn->move(218*m_width/1080, 1277*m_width/1080+m_tMargin);
    m_pLeftBtn->move(48*m_width/1080, 1439*m_width/1080+m_tMargin);
    m_pRightBtn->move(377*m_width/1080, 1439*m_width/1080+m_tMargin);
    m_pDownBtn->move(218*m_width/1080, 1595*m_width/1080+m_tMargin);
}

void CWidget::InitDirBtn(QPushButton *pBtn, const QString &objName)
{
    pBtn->setObjectName(objName);
    pBtn->setFocusPolicy(Qt::NoFocus);
    int sz = 220*m_width/1080;
    pBtn->setFixedSize(sz, sz);

    CJBtn *pJBtn = static_cast<CJBtn*>(pBtn);
    connect(pJBtn, SIGNAL(sigVibrate(int)), this, SLOT(slotOnVibrate(int)));

    // connect signal and slot
    connect(pBtn, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
}

void CWidget::InitRotateBtn()
{
    m_pRotateBtn = new CJBtn(this);
    RBtn = m_pRotateBtn;
    CJBtn* pJBtn = static_cast<CJBtn*>(m_pRotateBtn);
    connect(pJBtn, SIGNAL(sigVibrate(int)), this, SLOT(slotOnVibrate(int)));

    int sz = 310*m_width/1080;
    m_pRotateBtn->setFixedSize(sz, sz);
    m_pRotateBtn->setObjectName(tr("rotateBtn"));
    m_pRotateBtn->setFocusPolicy(Qt::NoFocus);
    m_pRotateBtn->move(698*m_width/1080, 1412*m_width/1080+m_tMargin);

    // connect signal and slot
    connect(m_pRotateBtn, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
}

void CWidget::InitSettingBtns()
{
    m_pThmY = new CJBtn(this);
    m_pThmR = new CJBtn(this);
    m_pThmBk = new CJBtn(this);
    m_pThmBl = new CJBtn(this);
    m_pThmGr = new CJBtn(this);

    m_pThmY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pThmR->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pThmBk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pThmBl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pThmGr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_pThmY->setStyleImg(QString(":/img/img/1.png"), QString(":/img/img/1.png"));
    m_pThmR->setStyleImg(QString(":/img/img/2.png"), QString(":/img/img/2.png"));
    m_pThmBk->setStyleImg(QString(":/img/img/3.png"), QString(":/img/img/3.png"));
    m_pThmBl->setStyleImg(QString(":/img/img/4.png"), QString(":/img/img/4.png"));
    m_pThmGr->setStyleImg(QString(":/img/img/5.png"), QString(":/img/img/5.png"));

    m_pThmY->setObjectName(tr("thmYBtn"));
    m_pThmR->setObjectName(tr("thmRBtn"));
    m_pThmBk->setObjectName(tr("thmBkBtn"));
    m_pThmBl->setObjectName(tr("thmBlBtn"));
    m_pThmGr->setObjectName(tr("thmGrBtn"));

    connect(m_pThmY, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
    connect(m_pThmR, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
    connect(m_pThmBk, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
    connect(m_pThmBl, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));
    connect(m_pThmGr, SIGNAL(clicked()), this, SLOT(slotBtnClicked()));

    HideThmSelBtns();

    QWidget *pframe = new QWidget(this);
    pframe->move(m_updateRt.topLeft());
    pframe->setFixedSize(m_updateRt.size());

    QVBoxLayout *pVBox = new QVBoxLayout();
    QHBoxLayout *pHBox3 = new QHBoxLayout();
    pVBox->addStretch(1);
    pVBox->addLayout(pHBox3);
    pframe->setLayout(pVBox);

    pHBox3->addWidget(m_pThmY);
    pHBox3->addWidget(m_pThmR);
    pHBox3->addWidget(m_pThmBk);
    pHBox3->addWidget(m_pThmBl);
    pHBox3->addWidget(m_pThmGr);

    pVBox->setStretch(0, 9);
    pVBox->setStretch(1, 4);
}

void CWidget::InitConnectors()
{
    // UI to game manager
    connect(this, SIGNAL(sigInitGameMgr(QPoint, QPoint, int, int)), m_pGameMgr, SLOT(slotInitGameManager(QPoint, QPoint, int, int)));
    connect(this, SIGNAL(sigAction(int)), m_pGameMgr, SLOT(slotGameAction(int)));
    connect(this, SIGNAL(sigPushButton(int)), m_pGameMgr, SLOT(slotMoveDirection(int)));

    // game manager to UI
    connect(m_pGameMgr, SIGNAL(sigUpdateUI()), this, SLOT(slotRefresh()));
    connect(m_pGameMgr, SIGNAL(sigPlayAudio(int)), this, SLOT(slotPlayAudio(int)));
}

void CWidget::InitPositions()
{
    m_titleRt = QRectF(QPointF(466, 88)*m_dvcXFactor, QPointF(690,148)*m_dvcXFactor);

    m_gmbdRt = QRectF(QPointF(241, 183)*m_dvcXFactor, QPointF(655,1004)*m_dvcXFactor);
    m_prbdRt = QRectF(QPointF(733, 470)*m_dvcXFactor, QPointF(900,637)*m_dvcXFactor);
    m_updateRt = QRect(QPoint(220, 157)*m_dvcXFactor, QPoint(943,1033)*m_dvcXFactor);

    m_scoreStrRt = QRect(QPoint(665, 176)*m_dvcXFactor, QPoint(938,221)*m_dvcXFactor);  // w=938-665=273,h=220-176=45
    m_scoreValRt = QRect(QPoint(665, 225)*m_dvcXFactor, QPoint(938,270)*m_dvcXFactor);  // w=273, m=4, h=45
    m_hiScoreStrRt=QRect(QPoint(665, 274)*m_dvcXFactor, QPoint(938,319)*m_dvcXFactor);  // w=273, m=4, h=45
    m_hiScoreValRt=QRect(QPoint(665, 323)*m_dvcXFactor, QPoint(938,368)*m_dvcXFactor);  // w=273, m=4, h=45
    m_goalStrRt = QRect(QPoint(665, 372)*m_dvcXFactor, QPoint(938,417)*m_dvcXFactor);   // w=273, m=4, h=45
    m_goalValRt = QRect(QPoint(665, 421)*m_dvcXFactor, QPoint(938,466)*m_dvcXFactor);   // w=273, m=4, h=45

    m_lvlStrRt= QRect(QPoint(665, 650)*m_dvcXFactor, QPoint(840, 695)*m_dvcXFactor);    // w=273, h=45
    m_lvlValRt= QRect(QPoint(850, 650)*m_dvcXFactor, QPoint(938, 695)*m_dvcXFactor);
    m_spdStrRt= QRect(QPoint(665, 705)*m_dvcXFactor, QPoint(840, 750)*m_dvcXFactor);    // w=273, m=10, h=45
    m_spdValRt= QRect(QPoint(850, 705)*m_dvcXFactor, QPoint(938, 750)*m_dvcXFactor);
    m_flashRt= QRect(QPoint(665, 765)*m_dvcXFactor, QPoint(938, 905)*m_dvcXFactor);     // w=273, m=5, h=150
    m_soundRt= QRect(QPoint(665, 920)*m_dvcXFactor, QPoint(797, 985)*m_dvcXFactor);     // w=136, m=15, h=65
    m_pauseRt= QRect(QPoint(797, 918)*m_dvcXFactor, QPoint(938, 980)*m_dvcXFactor);     // w=136, m=13, h=65
    m_pausedStrRt=QRect(QPoint(780, 985)*m_dvcXFactor, QPoint(938, 1030)*m_dvcXFactor); // w=136, m=5, h=45
}

void CWidget::InitGameMgrPos()
{
    QPoint tl(int(m_gmbdRt.left()), int(m_gmbdRt.top()));
    QPoint ptl(int(m_prbdRt.left()), int(m_prbdRt.top()));
    int width = int(m_gmbdRt.width());
    int height= int(m_gmbdRt.height());
    emit sigInitGameMgr(tl, ptl, width, height);
}

bool CWidget::ChangeScrPage(BTNID btn)
{
    emScrPage currentPage = m_scrPage;

    if(btn == setBtn)
    {
        m_scrPage = (m_scrPage != sp_game) ? sp_game : sp_setting;
    }
    else if(btn == exitBtn)
    {
        m_scrPage = (m_scrPage != sp_game) ? sp_game : sp_exit;
    }
    else if((btn == (QObject*)m_pThmY) || (btn == (QObject*)m_pThmR) || (btn == (QObject*)m_pThmBk) || (btn == (QObject*)m_pThmBl) || (btn == (QObject*)m_pThmGr))
    {
        m_scrPage = sp_setting;
    }
    else
    {
        m_scrPage = sp_game;
    }

    ShowThmSelBtns(m_scrPage == sp_setting);

    return (currentPage != m_scrPage);
}

void CWidget::slotRefresh()
{
    repaint(m_updateRt);
}

void CWidget::slotBtnClicked()
{
    BTNID btn = sender();
    qDebug() << "push button " << btn->objectName() << " triggered." << endl;

    if(ChangeScrPage(btn))
    {
        repaint(m_updateRt);
        return;
    }

    if(btn == uBtn)
    {
        emit sigPushButton(CGameManager::MD_Up);
    }
    else if(btn == dBtn)
    {
        emit sigPushButton(CGameManager::MD_Down);
    }
    else if(btn == lBtn)
    {
        emit sigPushButton(CGameManager::MD_Left);
    }
    else if(btn == rBtn)
    {
        emit sigPushButton(CGameManager::MD_Right);
    }
    else if(btn == startBtn)
    {
        emit sigAction(CGameManager::GA_NewGame);
        repaint(m_updateRt);
        slotPlayAudio(ai_btnClick);
    }
    else if(btn == soundBtn)
    {
        static int volume = m_player->volume();
        m_bVoiceOn = !m_bVoiceOn;
        m_player->setVolume((m_bVoiceOn? volume : 0));

        repaint(m_updateRt);
        slotPlayAudio(ai_btnClick);
    }
    else if(btn == setBtn)
    {
        slotPlayAudio(ai_btnClick);
    }
    else if(btn == exitBtn)
    {
        QApplication::exit();
    }
    else if(btn == RBtn)
    {
        emit sigPushButton(CGameManager::MD_Rotate);
    }
    else if(btn == (QObject*)m_pThmY)
    {
        SetTheme(TI_Yellow);
    }
    else if(btn == (QObject*)m_pThmR)
    {
        SetTheme(TI_Pink);
    }
    else if(btn == (QObject*)m_pThmBk)
    {
        SetTheme(TI_Bk);
    }
    else if(btn == (QObject*)m_pThmBl)
    {
        SetTheme(TI_Bl);
    }
    else if(btn == (QObject*)m_pThmGr)
    {
        SetTheme(TI_Gr);
    }
    else
    {

    }
}

void CWidget::slotOnVibrate(int ms)
{
#ifdef ANDROID
    if(m_bVibrateOn)
    {
        qDebug() << "slotOnVibrate()" << endl;
        QAndroidJniEnvironment env;
        QAndroidJniObject activity = androidActivity();
        QAndroidJniObject name = QAndroidJniObject::getStaticObjectField(
                    "android/content/Context",
                    "VIBRATOR_SERVICE",
                    "Ljava/lang/String;"
                    );
        CHECK_EXCEPTION()
        QAndroidJniObject vibrateService = activity.callObjectMethod(
                    "getSystemService",
                    "(Ljava/lang/String;)Ljava/lang/Object;",
                    name.object<jstring>());
        CHECK_EXCEPTION()
        jlong duration = ms;
        vibrateService.callMethod<void>("vibrate", "(J)V", duration);
        CHECK_EXCEPTION()
    }
#else
    Q_UNUSED(ms)
#endif
}

void CWidget::slotPlayAudio(int nIndex)
{
    m_player->setMedia(QUrl(audioUrl[nIndex]));
    m_player->play();
}

