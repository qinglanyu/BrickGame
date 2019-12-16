#ifndef GAMEGLOBAL_H
#define GAMEGLOBAL_H

#include <QColor>
#include <QPointF>

/* game board size definition */
#define COLCOUNT 10
#define ROWCOUNT 20
#define PRVBCOUNT 4

/* max count of shape color */
#define MAX_SHAPE_COLOR_COUNT 10

/* color definitons */
static QColor bgr = QColor("#D6B706");
static QColor white0(155, 155, 155), white1(166, 166, 166), white(255, 255, 255);
static QColor gray0(50, 50, 50, 128), gray1(80, 80, 80, 128), gray2(128, 128, 128, 128);
static QColor sinkBgr("#899775"), gameboardBgr("#899675"), shadow("#838D72");
static QColor miniBtnClr("#00FF42");
static QColor btnClr("#00AEEF");
static QString audioUrl[] =
{
    "qrc:/audio/Audio/BackgroundMusic.mp3",
    "qrc:/audio/Audio/BulletEnemy.mp3",
    "qrc:/audio/Audio/ButtonClick.mp3",
    "qrc:/audio/Audio/ExpolosionEnemy.mp3",
    "qrc:/audio/Audio/ExpolosionPlayer.mp3",
    "qrc:/audio/Audio/GameStart.mp3",
    "qrc:/audio/Audio/GamOver.mp3",
    "qrc:/audio/Audio/GridUp.mp3",
    "qrc:/audio/Audio/HighSpeed.mp3",
    "qrc:/audio/Audio/Land.mp3",
    "qrc:/audio/Audio/LineCleared.mp3",
    "qrc:/audio/Audio/move.mp3",
    "qrc:/audio/Audio/Rotate.mp3",
    "qrc:/audio/Audio/Warning.mp3",
    "qrc:/audio/Audio/Win.mp3"
};

enum AudioIndex
{
    ai_bgm = 0,
    ai_bulletEnemy,
    ai_btnClick,
    ai_expolosionEnemy,
    ai_expolosionPlayer,
    ai_gamestart,
    ai_gameover,
    ai_gridup,
    ai_highspd,
    ai_land,
    ai_lineclr,
    ai_move,
    ai_rotate,
    ai_warning,
    ai_win
};

typedef struct _tagThm
{
    QString bgr;
    QString ctlBtnNormal;
    QString ctlBtnPressed;
    QString toolBtnNormal;
    QString toolBtnPressed;
}THEME;

static THEME themes[] =
{
    {
        ":/img/img/Casing_Yellow.png",
        ":/img/img/ControlBtnNormalBlue_Yellow.png",
        ":/img/img/ControlBtnPressedBlue_Yellow.png",
        ":/img/img/SettingBtnNormalGreen_Yellow.png",
        ":/img/img/SettingBtnPressedGreen_Yellow.png"
    },
    {
        ":/img/img/Casing_Pink.png",
        ":/img/img/ControlBtnNormalYellow_Pink.png",
        ":/img/img/ControlBtnPressedYellow_Pink.png",
        ":/img/img/SettingBtnNormalGreen_Pink.png",
        ":/img/img/SettingBtnPressedGreen_Pink.png"
    },
    {
        ":/img/img/Casing_Black.png",
        ":/img/img/ControlBtnNormalYellow_Black.png",
        ":/img/img/ControlBtnPressedYellow_Black.png",
        ":/img/img/SettingBtnNormalGreen_Black.png",
        ":/img/img/SettingBtnPressedGreen_Black.png"
    },
    {
        ":/img/img/Casing_Blue.png",
        ":/img/img/ControlBtnNormalYellow_Blue.png",
        ":/img/img/ControlBtnPressedYellow_Blue.png",
        ":/img/img/SettingBtnNormalGreen_Blue.png",
        ":/img/img/SettingBtnPressedGreen_Blue.png"
    },
    {
        ":/img/img/Casing_Green.png",
        ":/img/img/ControlBtnNormalYellow_Green.png",
        ":/img/img/ControlBtnPressedYellow_Green.png",
        ":/img/img/SettingBtnNormalBlue_Green.png",
        ":/img/img/SettingBtnPressedBlue_Green.png"
    }
};

enum ThemeIndex
{
    TI_Yellow = 0,
    TI_Pink,
    TI_Bk,
    TI_Bl,
    TI_Gr
};


#endif // GAMEGLOBAL_H
