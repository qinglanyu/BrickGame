#ifndef CWIDGET_H
#define CWIDGET_H

#include <QWidget>

class QMediaPlayer;
class CGameManager;
class QPushButton;
class CCell;
class CJBtn;
struct _tagThm;

enum dvcScrType {Wider, Longer};
const static qreal gIdealX = 1152;
const static qreal gIdealY = 2048;
const static qreal gIdealLWRatio = gIdealY/gIdealX;

typedef QObject* BTNID;

class CWidget : public QWidget
{
    Q_OBJECT

    enum emScrPage
    {sp_game, sp_setting, sp_exit};

public:
    CWidget(int w, int h, dvcScrType scrType, qreal l=0, qreal t=0, QWidget *parent = nullptr);
    ~CWidget();

public:
    void InitUI();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void DrawGamePage(QPainter &painter);
    void DrawSettingPage(QPainter &painter);
    void DrawExitPage(QPainter &painter);

    void PasteBgrImg(QPainter &painter);
    void DrawStaticText(QPainter &painter);
    void DrawGameText(QPainter &painter);
    void DrawGameBoard(QPainter &painter);
    void DrawGameBoardBgr(QPainter &painter);
    void DrawMovingShape(QPainter &painter);
    void DrawPreviewBoard(QPainter &painter);
    void DrawPreviewBoardBgr(QPainter &painter);
    void paintCell(QPainter &painter, CCell &cell, bool moving=false);
    void DrawBoyHandsDown(QPainter &painter);
    void DrawAllRects(QPainter &painter);
    void DrawBackground();
    void DrawSoundSymb(QPainter &painter);
    void DrawPausedSymb(QPainter &painter);

    void ShowThmSelBtns(const bool bShow = true);
    void HideThmSelBtns();
    void SetTheme(const int nTheme);


protected:
    void InitGameBtns();
    void InitToolBtns();
    void InitToolBtn(QPushButton* pBtn, const QString &objName);
    void InitDirBtns();
    void InitDirBtn(QPushButton* pBtn, const QString &objName);
    void InitRotateBtn();
    void InitSettingBtns();

    void InitConnectors();
    void InitPositions();
    void InitGameMgrPos();

    bool ChangeScrPage(BTNID btn);

Q_SIGNALS:
    void sigInitGameMgr(QPoint gbTopLeft, QPoint pbTopLeft, int gbWidth, int gbHeight);
    void sigAction(int gameAction);
    void sigPushButton(int moveDirection);

public slots:
    void slotRefresh();
    void slotBtnClicked();
    void slotOnVibrate(int ms);
    void slotPlayAudio(int nIndex);

private:
    int m_width, m_height;
    dvcScrType m_scrType;
    qreal m_lMargin, m_tMargin;
    qreal m_dvcXFactor, m_dvcYFactor;
    QPoint m_wndPos;

    QMediaPlayer *m_player;
    CGameManager *m_pGameMgr;

    QFont m_digitFont;
    QFont m_digitFntItalic;
    QPushButton *m_pStartBtn;
    QPushButton *m_pSoundBtn;
    QPushButton *m_pSettingBtn;
    QPushButton *m_pExitBtn;
    QPushButton *m_pUpBtn;
    QPushButton *m_pDownBtn;
    QPushButton *m_pLeftBtn;
    QPushButton *m_pRightBtn;
    QPushButton *m_pRotateBtn;
    CJBtn *m_pShare;
    CJBtn *m_pLdrBd;
    CJBtn *m_pGifts;
    CJBtn *m_pSigIn;
    CJBtn *m_pThmY;
    CJBtn *m_pThmR;
    CJBtn *m_pThmBk;
    CJBtn *m_pThmBl;
    CJBtn *m_pThmGr;

    BTNID startBtn, soundBtn, setBtn, exitBtn, uBtn, lBtn, rBtn, dBtn, RBtn;

    QPixmap *m_pxbgr;

    bool m_bVoiceOn;
    bool m_bPaused;
    bool m_bVibrateOn;

    QRectF m_titleRt;
    QRectF m_gmbdRt, m_prbdRt;
    QRect  m_updateRt;
    QRectF m_scoreStrRt, m_scoreValRt;
    QRectF m_hiScoreStrRt, m_hiScoreValRt;
    QRectF m_goalStrRt, m_goalValRt;
    QRectF m_lvlStrRt, m_lvlValRt;
    QRectF m_spdStrRt, m_spdValRt;
    QRectF m_flashRt, m_soundRt, m_pauseRt, m_pausedStrRt;

    emScrPage m_scrPage;
    _tagThm *m_theme;
};
#endif // CWIDGET_H
