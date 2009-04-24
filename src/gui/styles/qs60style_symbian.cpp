/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include "qs60style.h"
#include "qs60style_p.h"
#include "qpainter.h"
#include "qstyleoption.h"
#include "qstyle.h"
#include "private/qwindowsurface_s60_p.h"
#include "private/qt_s60_p.h"
#include "private/qcore_symbian_p.h"
#include "qapplication.h"

#include <w32std.h>
#include <aknsconstants.h>
#include <aknconsts.h>
#include <aknsitemid.h>
#include <aknsutils.h>
#include <aknsdrawutils.h>
#include <aknsskininstance.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <avkon.mbg>
#include <AknFontAccess.h>
#include <AknLayoutFont.h>
#include <aknutils.h>

#if !defined(QT_NO_STYLE_S60) || defined(QT_PLUGIN)

QT_BEGIN_NAMESPACE

enum TDrawType {
    EDrawIcon,
    EDrawBackground,
    ENoDraw
};

enum TSupportRelease {
    ES60_3_1      = 0x0001,
    ES60_3_2      = 0x0002,
    ES60_5_0      = 0x0004,
    // Add all new releases here
    ES60_AllReleases = ES60_3_1 | ES60_3_2 | ES60_5_0
};

typedef struct {
    const TAknsItemID &skinID;
    TDrawType drawType;
    int supportInfo;
    int fallbackGraphicID;
    int newMajorSkinId;
    int newMinorSkinId;
} partMapEntry;

class QS60StyleModeSpecifics
{
public:
    static QPixmap skinnedGraphics(QS60StyleEnums::SkinParts stylepart,
        const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static QPixmap skinnedGraphics(QS60StylePrivate::SkinFrameElements frameElement, const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static QPixmap colorSkinnedGraphics(const QS60StyleEnums::SkinParts &stylepart,
        const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static QColor colorValue(const TAknsItemID &colorGroup, int colorIndex);
    static QPixmap fromFbsBitmap(CFbsBitmap *icon, CFbsBitmap *mask, QS60StylePrivate::SkinElementFlags flags, QImage::Format format);

private:
    static QPixmap createSkinnedGraphicsL(QS60StyleEnums::SkinParts part,
        const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static QPixmap createSkinnedGraphicsL(QS60StylePrivate::SkinFrameElements frameElement, const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static QPixmap colorSkinnedGraphicsL(const QS60StyleEnums::SkinParts &stylepart,
        const QSize &size, QS60StylePrivate::SkinElementFlags flags);
    static void frameIdAndCenterId(QS60StylePrivate::SkinFrameElements frameElement, TAknsItemID &frameId, TAknsItemID &centerId);
    static void checkAndUnCompressBitmapL(CFbsBitmap*& aOriginalBitmap);
    static void checkAndUnCompressBitmap(CFbsBitmap*& aOriginalBitmap);
    static void unCompressBitmapL(const TRect& aTrgRect, CFbsBitmap* aTrgBitmap, CFbsBitmap* aSrcBitmap);
    static void colorGroupAndIndex(QS60StyleEnums::SkinParts skinID,
        TAknsItemID &colorGroup, int colorIndex);
    static bool checkSupport(const int supportedRelease);
    static TAknsItemID checkAndUpdateReleaseSpecificGraphics(int part);
    // Array to match the skin ID, fallback graphics and Qt widget graphics.
    static const partMapEntry m_partMap[];
};

const partMapEntry QS60StyleModeSpecifics::m_partMap[] = {
    /* SP_QgnGrafBarFrameCenter */      {KAknsIIDQgnGrafBarFrameCenter,         EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_bar_frame_center  ,-1,-1},
    /* SP_QgnGrafBarFrameSideL */       {KAknsIIDQgnGrafBarFrameSideL,          EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_bar_frame_side_l  ,-1,-1},
    /* SP_QgnGrafBarFrameSideR */       {KAknsIIDQgnGrafBarFrameSideR,          EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_bar_frame_side_r  ,-1,-1},
    /* SP_QgnGrafBarProgress */         {KAknsIIDQgnGrafBarProgress,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_bar_progress      ,-1,-1},
    /* SP_QgnGrafTabActiveL */          {KAknsIIDQgnGrafTabActiveL,             EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_active_l      ,-1,-1},
    /* SP_QgnGrafTabActiveM */          {KAknsIIDQgnGrafTabActiveM,             EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_active_m      ,-1,-1},
    /* SP_QgnGrafTabActiveR */          {KAknsIIDQgnGrafTabActiveR,             EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_active_r      ,-1,-1},
    /* SP_QgnGrafTabPassiveL */         {KAknsIIDQgnGrafTabPassiveL,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_passive_l     ,-1,-1},
    /* SP_QgnGrafTabPassiveM */         {KAknsIIDQgnGrafTabPassiveM,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_passive_m     ,-1,-1},
    /* SP_QgnGrafTabPassiveR */         {KAknsIIDQgnGrafTabPassiveR,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_graf_tab_passive_r     ,-1,-1},
    /* SP_QgnIndiCheckboxOff */         {KAknsIIDQgnIndiCheckboxOff,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_checkbox_off      ,-1,-1},
    /* SP_QgnIndiCheckboxOn */          {KAknsIIDQgnIndiCheckboxOn,             EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_checkbox_on       ,-1,-1},
    /* SP_QgnIndiMarkedAdd */           {KAknsIIDQgnIndiMarkedAdd,              EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_marked_add        ,-1,-1},
    /* SP_QgnIndiNaviArrowLeft */       {KAknsIIDQgnGrafScrollArrowLeft,        EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_navi_arrow_left   ,-1,-1},
    /* SP_QgnIndiNaviArrowRight */      {KAknsIIDQgnGrafScrollArrowRight,       EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_navi_arrow_right  ,-1,-1},
    /* SP_QgnIndiRadiobuttOff */        {KAknsIIDQgnIndiRadiobuttOff,           EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_radiobutt_off     ,-1,-1},
    /* SP_QgnIndiRadiobuttOn */         {KAknsIIDQgnIndiRadiobuttOn,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_radiobutt_on      ,-1,-1},
    /* SP_QgnIndiSliderEdit */          {KAknsIIDQgnIndiSliderEdit,             EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_slider_edit       ,-1,-1},
    /* SP_QgnIndiSubMenu */             {KAknsIIDQgnIndiSubmenu,                EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_indi_submenu           ,-1,-1},
    /* SP_QgnNoteErased */              {KAknsIIDQgnNoteErased,                 EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_erased            ,-1,-1},
    /* SP_QgnNoteError */               {KAknsIIDQgnNoteError,                  EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_error             ,-1,-1},
    /* SP_QgnNoteInfo */                {KAknsIIDQgnNoteInfo,                   EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_info              ,-1,-1},
    /* SP_QgnNoteOk */                  {KAknsIIDQgnNoteOk,                     EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_ok                ,-1,-1},
    /* SP_QgnNoteQuery */               {KAknsIIDQgnNoteQuery,                  EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_query             ,-1,-1},
    /* SP_QgnNoteWarning */             {KAknsIIDQgnNoteWarning,                EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_note_warning           ,-1,-1},
    /* SP_QgnPropFileSmall */           {KAknsIIDQgnPropFileSmall,              EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_prop_file_small        ,-1,-1},
    /* SP_QgnPropFolderCurrent */       {KAknsIIDQgnPropFolderCurrent,          EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_prop_folder_current    ,-1,-1},
    /* SP_QgnPropFolderSmall */         {KAknsIIDQgnPropFolderSmall,            EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_prop_folder_small      ,-1,-1},
    /* SP_QgnPropFolderSmallNew */      {KAknsIIDQgnPropFolderSmallNew,         EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_prop_folder_small_new  ,-1,-1},
    /* SP_QgnPropPhoneMemcLarge */      {KAknsIIDQgnPropPhoneMemcLarge,         EDrawIcon,   ES60_AllReleases,  EMbmAvkonQgn_prop_phone_memc_large  ,-1,-1},

// No fallback graphics for screen elements (it is guaranteed that the root skin contains these).
    /* SP_QsnBgScreen */                {KAknsIIDQsnBgScreen,                   EDrawBackground,   ES60_AllReleases, -1,-1,-1},

    /* SP_QsnCpScrollBgBottom */        {KAknsIIDQsnCpScrollBgBottom,           EDrawIcon,  ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnCpScrollBgMiddle */        {KAknsIIDQsnCpScrollBgMiddle,           EDrawIcon,  ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnCpScrollBgTop */           {KAknsIIDQsnCpScrollBgTop,              EDrawIcon,  ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnCpScrollHandleBottom */    {KAknsIIDQsnCpScrollHandleBottom,       EDrawIcon,  ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnCpScrollHandleMiddle */    {KAknsIIDQsnCpScrollHandleMiddle,       EDrawIcon,  ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnCpScrollHandleTop */       {KAknsIIDQsnCpScrollHandleTop,          EDrawIcon,  ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrButtonTbCornerTl */      {KAknsIIDQsnFrButtonTbCornerTl,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerTr */      {KAknsIIDQsnFrButtonTbCornerTr,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerBl */      {KAknsIIDQsnFrButtonTbCornerBl,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerBr */      {KAknsIIDQsnFrButtonTbCornerBr,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideT */         {KAknsIIDQsnFrButtonTbSideT,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideB */         {KAknsIIDQsnFrButtonTbSideB,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideL */         {KAknsIIDQsnFrButtonTbSideL,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideR */         {KAknsIIDQsnFrButtonTbSideR,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCenter */        {KAknsIIDQsnFrButtonTbCenter,           ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrButtonTbCornerTlPressed */{KAknsIIDQsnFrButtonTbCornerTlPressed, ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerTrPressed */{KAknsIIDQsnFrButtonTbCornerTrPressed, ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerBlPressed */{KAknsIIDQsnFrButtonTbCornerBlPressed, ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCornerBrPressed */{KAknsIIDQsnFrButtonTbCornerBrPressed, ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideTPressed */  {KAknsIIDQsnFrButtonTbSideTPressed,     ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideBPressed */  {KAknsIIDQsnFrButtonTbSideBPressed,     ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideLPressed */  {KAknsIIDQsnFrButtonTbSideLPressed,     ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbSideRPressed */  {KAknsIIDQsnFrButtonTbSideRPressed,     ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrButtonTbCenterPressed */ {KAknsIIDQsnFrButtonTbCenterPressed,    ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrCaleCornerTl */          {KAknsIIDQsnFrCaleCornerTl,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleCornerTr */          {KAknsIIDQsnFrCaleCornerTr,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleCornerBl */          {KAknsIIDQsnFrCaleCornerBl,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleCornerBr */          {KAknsIIDQsnFrCaleCornerBr,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleGSideT */            {KAknsIIDQsnFrCaleSideT,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleGSideB */            {KAknsIIDQsnFrCaleSideB,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleGSideL */            {KAknsIIDQsnFrCaleSideL,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleGSideR */            {KAknsIIDQsnFrCaleSideR,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleCenter */            {KAknsIIDQsnFrCaleCenter,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrCaleHeadingCornerTl */   {KAknsIIDQsnFrCaleHeadingCornerTl,      ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingCornerTr */   {KAknsIIDQsnFrCaleHeadingCornerTr,      ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingCornerBl */   {KAknsIIDQsnFrCaleHeadingCornerBl,      ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingCornerBr */   {KAknsIIDQsnFrCaleHeadingCornerBr,      ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingSideT */      {KAknsIIDQsnFrCaleHeadingSideT,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingSideB */      {KAknsIIDQsnFrCaleHeadingSideB,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingSideL */      {KAknsIIDQsnFrCaleHeadingSideL,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingSideR */      {KAknsIIDQsnFrCaleHeadingSideR,         ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrCaleHeadingCenter */     {KAknsIIDQsnFrCaleHeadingCenter,        ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrInputCornerTl */         {KAknsIIDQsnFrInputCornerTl,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputCornerTr */         {KAknsIIDQsnFrInputCornerTr,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputCornerBl */         {KAknsIIDQsnFrInputCornerBl,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputCornerBr */         {KAknsIIDQsnFrInputCornerBr,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputSideT */            {KAknsIIDQsnFrInputSideT,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputSideB */            {KAknsIIDQsnFrInputSideB,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputSideL */            {KAknsIIDQsnFrInputSideL,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputSideR */            {KAknsIIDQsnFrInputSideR,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrInputCenter */           {KAknsIIDQsnFrInputCenter,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrListCornerTl */          {KAknsIIDQsnFrListCornerTl,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListCornerTr */          {KAknsIIDQsnFrListCornerTr,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListCornerBl */          {KAknsIIDQsnFrListCornerBl,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListCornerBr */          {KAknsIIDQsnFrListCornerBr,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListSideT */             {KAknsIIDQsnFrListSideT,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListSideB */             {KAknsIIDQsnFrListSideB,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListSideL */             {KAknsIIDQsnFrListSideL,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListSideR */             {KAknsIIDQsnFrListSideR,                ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrListCenter */            {KAknsIIDQsnFrListCenter,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},

    /* SP_QsnFrPopupCornerTl */         {KAknsIIDQsnFrPopupCornerTl,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupCornerTr */         {KAknsIIDQsnFrPopupCornerTr,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupCornerBl */         {KAknsIIDQsnFrPopupCornerBl,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupCornerBr */         {KAknsIIDQsnFrPopupCornerBr,            ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupSideT */            {KAknsIIDQsnFrPopupSideT,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupSideB */            {KAknsIIDQsnFrPopupSideB,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupSideL */            {KAknsIIDQsnFrPopupSideL,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupSideR */            {KAknsIIDQsnFrPopupSideR,               ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrPopupCenter */           {KAknsIIDQsnFrPopupCenter,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},


    /* SP_QsnFrPopupPreviewCornerTl */  {KAknsIIDQsnFrPopupCornerTl,            ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c5},
    /* SP_QsnFrPopupPreviewCornerTr */  {KAknsIIDQsnFrPopupCornerTr,            ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c6},
    /* SP_QsnFrPopupPreviewCornerBl */  {KAknsIIDQsnFrPopupCornerBl,            ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c3},
    /* SP_QsnFrPopupPreviewCornerBr */  {KAknsIIDQsnFrPopupCornerBr,            ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c4},
    /* SP_QsnFrPopupPreviewSideT */     {KAknsIIDQsnFrPopupSideT,               ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19ca},
    /* SP_QsnFrPopupPreviewSideB */     {KAknsIIDQsnFrPopupSideB,               ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c7},
    /* SP_QsnFrPopupPreviewSideL */     {KAknsIIDQsnFrPopupSideL,               ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c8},
    /* SP_QsnFrPopupPreviewSideR */     {KAknsIIDQsnFrPopupSideR,               ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c9},
    /* SP_QsnFrPopupPreviewCenter */    {KAknsIIDQsnFrPopupCenter,              ENoDraw,    ES60_3_1,   -1, EAknsMajorSkin, 0x19c2},

    /* SP_QsnFrSetOptCornerTl */        {KAknsIIDQsnFrSetOptCornerTl,           ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptCornerTr */        {KAknsIIDQsnFrSetOptCornerTr,           ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptCornerBl */        {KAknsIIDQsnFrSetOptCornerBl,           ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptCornerBr */        {KAknsIIDQsnFrSetOptCornerBr,           ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptSideT */           {KAknsIIDQsnFrSetOptSideT,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptSideB */           {KAknsIIDQsnFrSetOptSideB,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptSideL */           {KAknsIIDQsnFrSetOptSideL,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptSideR */           {KAknsIIDQsnFrSetOptSideR,              ENoDraw,    ES60_AllReleases,    -1,-1,-1},
    /* SP_QsnFrSetOptCenter */          {KAknsIIDQsnFrSetOptCenter,             ENoDraw,    ES60_AllReleases,    -1,-1,-1},
};

QPixmap QS60StyleModeSpecifics::skinnedGraphics(
    QS60StyleEnums::SkinParts stylepart, const QSize &size,
    QS60StylePrivate::SkinElementFlags flags)
{
    QPixmap themedImage;
    TRAPD( error, {
            const QPixmap skinnedImage = createSkinnedGraphicsL(stylepart, size, flags);
            themedImage = skinnedImage;
    });
    if (error)
        return themedImage = QPixmap();
    return themedImage;
}

QPixmap QS60StyleModeSpecifics::skinnedGraphics(
    QS60StylePrivate::SkinFrameElements frame, const QSize &size, QS60StylePrivate::SkinElementFlags flags)
{
    QPixmap themedImage;
    TRAPD( error, {
            const QPixmap skinnedImage = createSkinnedGraphicsL(frame, size, flags);
            themedImage = skinnedImage;
    });
    if (error)
        return themedImage = QPixmap();
    return themedImage;
}

QPixmap QS60StyleModeSpecifics::colorSkinnedGraphics(
    const QS60StyleEnums::SkinParts &stylepart,
    const QSize &size, QS60StylePrivate::SkinElementFlags flags)
{
    QPixmap colorGraphics;
    TRAPD(error, colorGraphics = colorSkinnedGraphicsL(stylepart, size, flags));
    return error ? QPixmap() : colorGraphics;
}

QPixmap QS60StyleModeSpecifics::colorSkinnedGraphicsL(
    const QS60StyleEnums::SkinParts &stylepart,
    const QSize &size, QS60StylePrivate::SkinElementFlags flags)
{
    const int stylepartIndex = (int)stylepart;
    const TAknsItemID skinId = m_partMap[stylepartIndex].skinID;
    const TInt fallbackGraphicID = m_partMap[stylepartIndex].fallbackGraphicID;
    TAknsItemID colorGroup = KAknsIIDQsnIconColors;
    int colorIndex = 0;
    colorGroupAndIndex(stylepart, colorGroup, colorIndex);

    const bool rotatedBy90or270 =
        (flags & (QS60StylePrivate::SF_PointEast | QS60StylePrivate::SF_PointWest));
    const TSize targetSize =
        rotatedBy90or270?TSize(size.height(), size.width()):TSize(size.width(), size.height());
    CFbsBitmap *icon = 0;
    CFbsBitmap *iconMask = 0;
    const TInt fallbackGraphicsMaskID =
        fallbackGraphicID == KErrNotFound?KErrNotFound:fallbackGraphicID+1; //masks are auto-generated as next in mif files
    MAknsSkinInstance* skinInstance = AknsUtils::SkinInstance();
    AknsUtils::CreateColorIconLC(
        skinInstance, skinId, colorGroup, colorIndex, icon, iconMask, KAvkonBitmapFile, fallbackGraphicID , fallbackGraphicsMaskID, KRgbBlack);
    User::LeaveIfError(AknIconUtils::SetSize(icon, targetSize, EAspectRatioNotPreserved));
    User::LeaveIfError(AknIconUtils::SetSize(iconMask, targetSize, EAspectRatioNotPreserved));
    QPixmap result = fromFbsBitmap(icon, iconMask, flags, qt_TDisplayMode2Format(icon->DisplayMode()));
    CleanupStack::PopAndDestroy(2); //icon, iconMask
    return result;
}

QColor QS60StyleModeSpecifics::colorValue(const TAknsItemID &colorGroup, int colorIndex)
{
    TRgb skinnedColor;
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    AknsUtils::GetCachedColor(skin, skinnedColor, colorGroup, colorIndex);
    return QColor(skinnedColor.Red(),skinnedColor.Green(),skinnedColor.Blue());
}

QPixmap QS60StyleModeSpecifics::fromFbsBitmap(CFbsBitmap *icon, CFbsBitmap *mask, QS60StylePrivate::SkinElementFlags flags, QImage::Format format)
{
    Q_ASSERT(icon);
    const TSize iconSize = icon->SizeInPixels();
    const int iconBytesPerLine = CFbsBitmap::ScanLineLength(iconSize.iWidth, icon->DisplayMode());
    const int iconBytesCount = iconBytesPerLine * iconSize.iHeight;

    QImage iconImage(qt_TSize2QSize(iconSize), format);
    if (iconImage.isNull())
        return QPixmap();

    checkAndUnCompressBitmap(icon);
    if (!icon) //checkAndUnCompressBitmap might set icon to NULL
        return QPixmap();

    icon->LockHeap();
    const uchar *const iconBytes = (uchar*)icon->DataAddress();
    // The icon data needs to be copied, since the color format will be
    // automatically converted to Format_ARGB32 when setAlphaChannel is called.
    memcpy(iconImage.bits(), iconBytes, iconBytesCount);
    icon->UnlockHeap();
    if (mask) {
        checkAndUnCompressBitmap(mask);
        if (mask) { //checkAndUnCompressBitmap might set mask to NULL
            const TSize maskSize = icon->SizeInPixels();
            const int maskBytesPerLine = CFbsBitmap::ScanLineLength(maskSize.iWidth, mask->DisplayMode());
            mask->LockHeap();
            const uchar *const maskBytes = (uchar *)mask->DataAddress();
            // Since no other bitmap should be locked, we can just "borrow" the mask data for setAlphaChannel
            const QImage maskImage(maskBytes, maskSize.iWidth, maskSize.iHeight, maskBytesPerLine, QImage::Format_Indexed8);
            if (!maskImage.isNull())
                iconImage.setAlphaChannel(maskImage);
            mask->UnlockHeap();
        }
    }

    QTransform imageTransform;
    if (flags & QS60StylePrivate::SF_PointEast) {
        imageTransform.rotate(90);
    } else if (flags & QS60StylePrivate::SF_PointSouth) {
        imageTransform.rotate(180);
        iconImage = iconImage.transformed(imageTransform);
    } else if (flags & QS60StylePrivate::SF_PointWest) {
        imageTransform.rotate(270);
    }
    if (imageTransform.isRotating())
        iconImage = iconImage.transformed(imageTransform);

    return QPixmap::fromImage(iconImage);
}

QPixmap QS60StylePrivate::backgroundTexture()
{
    static QPixmap result;
    // Poor mans caching. + Making sure that there is always only one background image in memory at a time

/*
    TODO: 1) Hold the background QPixmap as pointer in a static class member.
             Also add a deleteBackground() function and call that in ~QS60StylePrivate()
          2) Don't cache the background at all as soon as we have native pixmap support
*/

    if (!m_backgroundValid) {
        result = QPixmap();
        result = part(QS60StyleEnums::SP_QsnBgScreen,
            QSize(S60->screenWidthInPixels, S60->screenHeightInPixels), SkinElementFlags());
        m_backgroundValid = true;
    }
    return result;
}

bool QS60StylePrivate::isTouchSupported()
{
    return bool(AknLayoutUtils::PenEnabled());
}

void qt_s60_fill_background(QPainter *painter, const QRegion &rgn, const QPoint &offset)
{
    const QPixmap backgroundTexture(QS60StylePrivate::backgroundTexture());
    const QPaintDevice *target = painter->device();
    if (target->devType() == QInternal::Widget) {
        const QWidget *widget = static_cast<const QWidget *>(target);
        const CCoeControl *control = widget->effectiveWinId();
        const TPoint globalPos = control ? control->PositionRelativeToScreen() : TPoint(0,0);
        const QRegion translated = rgn.translated(offset);
        const QVector<QRect> &rects = translated.rects();
        for (int i = 0; i < rects.size(); ++i) {
            const QRect rect(rects.at(i));
            painter->drawPixmap(rect.topLeft(), backgroundTexture,
                                rect.translated(globalPos.iX, globalPos.iY));
        }
    }
}

QPixmap QS60StyleModeSpecifics::createSkinnedGraphicsL(
    QS60StyleEnums::SkinParts part,
    const QSize &size, QS60StylePrivate::SkinElementFlags flags)
{
    if (!size.isValid())
        return QPixmap();

    // Check release support and change part, if necessary.
    const TAknsItemID skinId = checkAndUpdateReleaseSpecificGraphics((int)part);
    const int stylepartIndex = (int)part;
    const TDrawType drawType = m_partMap[stylepartIndex].drawType;
    Q_ASSERT(drawType != ENoDraw);
    const TInt fallbackGraphicID = m_partMap[stylepartIndex].fallbackGraphicID;
    const bool rotatedBy90or270 =
        (flags & (QS60StylePrivate::SF_PointEast | QS60StylePrivate::SF_PointWest));
    TSize targetSize =
        rotatedBy90or270 ? TSize(size.height(), size.width()) : qt_QSize2TSize(size);

    MAknsSkinInstance* skinInstance = AknsUtils::SkinInstance();

    QPixmap result;

    switch (drawType) {
    case EDrawIcon:
    {
        CFbsBitmap *icon = 0;
        CFbsBitmap *iconMask = 0;
        const TInt fallbackGraphicsMaskID =
            fallbackGraphicID == KErrNotFound?KErrNotFound:fallbackGraphicID+1; //masks are auto-generated as next in mif files
//        QS60WindowSurface::unlockBitmapHeap();
        AknsUtils::CreateIconLC(skinInstance, skinId, icon, iconMask, KAvkonBitmapFile, fallbackGraphicID , fallbackGraphicsMaskID);
        User::LeaveIfError(AknIconUtils::SetSize(icon, targetSize, EAspectRatioNotPreserved));
        User::LeaveIfError(AknIconUtils::SetSize(iconMask, targetSize, EAspectRatioNotPreserved));
        result = fromFbsBitmap(icon, iconMask, flags, qt_TDisplayMode2Format(icon->DisplayMode()));
        CleanupStack::PopAndDestroy(2); // iconMask, icon
//        QS60WindowSurface::lockBitmapHeap();
        break;
    }
    case EDrawBackground:
    {
//        QS60WindowSurface::unlockBitmapHeap();
        CFbsBitmap *background = new (ELeave) CFbsBitmap(); //offscreen
        CleanupStack::PushL(background);
        User::LeaveIfError(background->Create(targetSize, EColor16MA));

        // todo: push background into CleanupStack
        CFbsBitmapDevice* dev = CFbsBitmapDevice::NewL(background);
        CleanupStack::PushL(dev);
        CFbsBitGc* gc = NULL;
        User::LeaveIfError(dev->CreateContext(gc));
        CleanupStack::PushL(gc);

        CAknsBasicBackgroundControlContext* bgContext = CAknsBasicBackgroundControlContext::NewL(
            skinId,
            targetSize,
            EFalse);
        CleanupStack::PushL(bgContext);

        const TBool drawn = AknsDrawUtils::DrawBackground(
            skinInstance,
            bgContext,
            NULL,
            *gc,
            TPoint(),
            targetSize,
            KAknsDrawParamDefault | KAknsDrawParamRGBOnly);

        if (drawn)
            result = fromFbsBitmap(background, NULL, flags, QImage::Format_RGB32);

        CleanupStack::PopAndDestroy(4, background); //background, dev, gc, bgContext
//        QS60WindowSurface::lockBitmapHeap();
        break;
    }
    }

    return result; // TODO: Let fromFbsBitmap return a QPixmap
}

QPixmap QS60StyleModeSpecifics::createSkinnedGraphicsL(QS60StylePrivate::SkinFrameElements frameElement,
    const QSize &size, QS60StylePrivate::SkinElementFlags flags)
{
    if (!size.isValid())
        return QPixmap();

    const bool rotatedBy90or270 =
        (flags & (QS60StylePrivate::SF_PointEast | QS60StylePrivate::SF_PointWest));
    TSize targetSize =
        rotatedBy90or270 ? TSize(size.height(), size.width()) : qt_QSize2TSize(size);

    MAknsSkinInstance* skinInstance = AknsUtils::SkinInstance();

    QPixmap result;

//        QS60WindowSurface::unlockBitmapHeap();
    static const bool canDoEColor16MAP = !(QSysInfo::s60Version() == QSysInfo::SV_S60_3_1 || QSysInfo::s60Version() == QSysInfo::SV_S60_3_2);
    static const TDisplayMode displayMode = canDoEColor16MAP ? TDisplayMode(13) : EColor16MA; // 13 = EColor16MAP
    static const TInt drawParam = canDoEColor16MAP ? KAknsDrawParamDefault : KAknsDrawParamNoClearUnderImage|KAknsDrawParamRGBOnly;

    CFbsBitmap *frame = new (ELeave) CFbsBitmap(); //offscreen
    CleanupStack::PushL(frame);
    User::LeaveIfError(frame->Create(targetSize, displayMode));

    CFbsBitmapDevice* bitmapDev = CFbsBitmapDevice::NewL(frame);
    CleanupStack::PushL(bitmapDev);
    CFbsBitGc* bitmapGc = NULL;
    User::LeaveIfError(bitmapDev->CreateContext(bitmapGc));
    CleanupStack::PushL(bitmapGc);

    frame->LockHeap();
    memset(frame->DataAddress(), 0, frame->SizeInPixels().iWidth * frame->SizeInPixels().iHeight * 4);  // 4: argb bytes
    frame->UnlockHeap();

    const TRect outerRect(TPoint(0, 0), targetSize);
    TRect innerRect = outerRect;
    innerRect.Shrink(
        QS60StylePrivate::pixelMetric(PM_Custom_FrameCornerWidth),
        QS60StylePrivate::pixelMetric(PM_Custom_FrameCornerHeight)
    );

    TAknsItemID frameSkinID, centerSkinID;
    frameSkinID = centerSkinID = checkAndUpdateReleaseSpecificGraphics(QS60StylePrivate::m_frameElementsData[frameElement].center);
    frameIdAndCenterId(frameElement, frameSkinID, centerSkinID);
    const TBool drawn = AknsDrawUtils::DrawFrame( skinInstance,
                           *bitmapGc, outerRect, innerRect,
                           frameSkinID, centerSkinID,
                           drawParam );

    if (canDoEColor16MAP) {
        if (drawn)
            result = fromFbsBitmap(frame, NULL, flags, QImage::Format_ARGB32_Premultiplied);
    } else {
        TDisplayMode maskDepth = EGray2;
        // Query the skin item for possible frame graphics mask details.
        if (skinInstance) {
            CAknsMaskedBitmapItemData* skinMaskedBmp = static_cast<CAknsMaskedBitmapItemData*>(
                    skinInstance->GetCachedItemData(frameSkinID,EAknsITMaskedBitmap));
            if (skinMaskedBmp && skinMaskedBmp->Mask())
                maskDepth = skinMaskedBmp->Mask()->DisplayMode();
            }
        if (maskDepth != ENone) {
            CFbsBitmap *frameMask = new (ELeave) CFbsBitmap(); //offscreen
            CleanupStack::PushL(frameMask);
            User::LeaveIfError(frameMask->Create(targetSize, maskDepth));

            CFbsBitmapDevice* maskBitmapDevice = CFbsBitmapDevice::NewL(frameMask);
            CleanupStack::PushL(maskBitmapDevice);
            CFbsBitGc* maskBitGc = NULL;
            User::LeaveIfError(maskBitmapDevice->CreateContext(maskBitGc));
            CleanupStack::PushL(maskBitGc);

            if (drawn) {
                //ensure that the mask is really transparent
                maskBitGc->Activate( maskBitmapDevice );
                maskBitGc->SetPenStyle(CGraphicsContext::ENullPen);
                maskBitGc->SetBrushStyle(CGraphicsContext::ESolidBrush);
                maskBitGc->SetBrushColor(KRgbWhite);
                maskBitGc->Clear();
                maskBitGc->SetBrushStyle(CGraphicsContext::ENullBrush);

                AknsDrawUtils::DrawFrame(skinInstance,
                                           *maskBitGc, outerRect, innerRect,
                                           frameSkinID, centerSkinID,
                                           KAknsSDMAlphaOnly |KAknsDrawParamNoClearUnderImage);
                result = fromFbsBitmap(frame, frameMask, flags, QImage::Format_ARGB32);
            }
            CleanupStack::PopAndDestroy(3, frameMask);
            }
        }
    CleanupStack::PopAndDestroy(3, frame); //frame, bitmapDev, bitmapGc

    return result; // TODO: Let fromFbsBitmap return a QPixmap
}

void QS60StyleModeSpecifics::frameIdAndCenterId(QS60StylePrivate::SkinFrameElements frameElement, TAknsItemID &frameId, TAknsItemID &centerId)
{
// There are some major mix-ups in skin declarations for some frames.
// First, the frames are not declared in sequence.
// Second, the parts use different major than the frame-master.

    switch(frameElement) {
        case QS60StylePrivate::SF_ToolTip:
            if (QSysInfo::s60Version()==QSysInfo::SV_S60_5_0 || QSysInfo::s60Version()==QSysInfo::SV_S60_3_2) {
                centerId.Set(EAknsMajorSkin, 0x5300);
                frameId.Set(EAknsMajorGeneric, 0x19c2);
            } else {
                centerId.Set(KAknsIIDQsnFrPopupCenter);
                frameId.iMinor = centerId.iMinor - 9;
            }
            break;
        default:
            // center should be correct here
            frameId.iMinor = centerId.iMinor - 9;
            break;
    }
}

bool QS60StyleModeSpecifics::checkSupport(const int supportedRelease)
{
    const QSysInfo::S60Version currentRelease = QSysInfo::s60Version();
    return ( (currentRelease == QSysInfo::SV_S60_3_1 && supportedRelease & ES60_3_1) ||
             (currentRelease == QSysInfo::SV_S60_3_2 && supportedRelease & ES60_3_2) ||
             (currentRelease == QSysInfo::SV_S60_5_0 && supportedRelease & ES60_5_0));
}

TAknsItemID QS60StyleModeSpecifics::checkAndUpdateReleaseSpecificGraphics(int part)
{
    TAknsItemID newSkinId;
    if (!checkSupport(m_partMap[(int)part].supportInfo))
        newSkinId.Set(m_partMap[(int)part].newMajorSkinId, m_partMap[(int)part].newMinorSkinId);
    else
        newSkinId.Set(m_partMap[(int)part].skinID);
    return newSkinId;
}

void QS60StyleModeSpecifics::checkAndUnCompressBitmap(CFbsBitmap*& aOriginalBitmap)
{
    TRAPD(error, checkAndUnCompressBitmapL(aOriginalBitmap));
    if (error)
        aOriginalBitmap = NULL;
}

void QS60StyleModeSpecifics::checkAndUnCompressBitmapL(CFbsBitmap*& aOriginalBitmap)
{
    if (aOriginalBitmap->IsCompressedInRAM()) {
        const TSize iconSize(aOriginalBitmap->SizeInPixels().iWidth,
            aOriginalBitmap->SizeInPixels().iHeight);
        CFbsBitmap* uncompressedBitmap = new (ELeave) CFbsBitmap();
        CleanupStack::PushL(uncompressedBitmap);
        User::LeaveIfError(uncompressedBitmap->Create(iconSize,
            aOriginalBitmap->DisplayMode()));
        unCompressBitmapL(iconSize, uncompressedBitmap, aOriginalBitmap);
        CleanupStack::Pop(uncompressedBitmap);
        User::LeaveIfError(aOriginalBitmap->Duplicate(
            uncompressedBitmap->Handle()));
        delete uncompressedBitmap;
    }
}

QFont QS60StylePrivate::s60Font_specific(
    QS60StyleEnums::FontCategories fontCategory, int pointSize)
{
    enum TAknFontCategory aknFontCategory = EAknFontCategoryUndefined;
    switch (fontCategory) {
        case QS60StyleEnums::FC_Primary:
            aknFontCategory = EAknFontCategoryPrimary;
            break;
        case QS60StyleEnums::FC_Secondary:
            aknFontCategory = EAknFontCategorySecondary;
            break;
        case QS60StyleEnums::FC_Title:
            aknFontCategory = EAknFontCategoryTitle;
            break;
        case QS60StyleEnums::FC_PrimarySmall:
            aknFontCategory = EAknFontCategoryPrimarySmall;
            break;
        case QS60StyleEnums::FC_Digital:
            aknFontCategory = EAknFontCategoryDigital;
            break;
        case QS60StyleEnums::FC_Undefined:
        default:
            break;
    }

    // Create AVKON font according the given parameters
    CWsScreenDevice* dev = CCoeEnv::Static()->ScreenDevice();
    TAknFontSpecification spec(aknFontCategory, TFontSpec(), NULL);
    if (pointSize > 0) {
        const TInt pixelSize = dev->VerticalTwipsToPixels(pointSize * KTwipsPerPoint);
        spec.SetTextPaneHeight(pixelSize + 4); // TODO: Is 4 a reasonable top+bottom margin?
    }

    QFont result;
    TRAPD( error, {
        const CAknLayoutFont* aknFont =
            AknFontAccess::CreateLayoutFontFromSpecificationL(*dev, spec);

        result = qt_TFontSpec2QFontL(aknFont->DoFontSpecInTwips());
        if (result.pointSize() != pointSize)
            result.setPointSize(pointSize); // Correct the font size returned by CreateLayoutFontFromSpecificationL()

        delete aknFont;
    });
    if (error) result = QFont();
    return result;
}

#ifdef QT_S60STYLE_LAYOUTDATA_SIMULATED
void QS60StylePrivate::setActiveLayout()
{
    //todo: how to find layouts that are of same size (QVGA1 vs. QVGA2)
    const QSize activeScreenSize(screenSize());
    int activeLayoutIndex = 0;
    const bool mirrored = !QApplication::isLeftToRight();
    const short screenHeight = (short)activeScreenSize.height();
    const short screenWidth = (short)activeScreenSize.width();
    for (int i=0; i<m_numberOfLayouts; i++) {
        if (screenHeight==m_layoutHeaders[i].height &&
            screenWidth==m_layoutHeaders[i].width &&
            mirrored==m_layoutHeaders[i].mirroring) {
            activeLayoutIndex = i;
            break;
        }
    }
    m_pmPointer = data[activeLayoutIndex];
}
#endif // QT_S60STYLE_LAYOUTDATA_SIMULATED

QS60StylePrivate::QS60StylePrivate()
{
#ifdef QT_S60STYLE_LAYOUTDATA_SIMULATED
    // No need to set active layout, if dynamic metrics API is available
    setActiveLayout();
#endif // QT_S60STYLE_LAYOUTDATA_SIMULATED
}

QS60StylePrivate::~QS60StylePrivate()
{
    m_backgroundValid = false;
}

short QS60StylePrivate::pixelMetric(int metric)
{
#ifdef QT_S60STYLE_LAYOUTDATA_SIMULATED
    Q_ASSERT(metric < MAX_PIXELMETRICS);
    const short returnValue = m_pmPointer[metric];
    if (returnValue==-909)
        return -1;
    return returnValue;
#else
    //todo - call the pixelmetrics API directly
    return 0;
#endif // QT_S60STYLE_LAYOUTDATA_SIMULATED
}

QPixmap QS60StylePrivate::part(QS60StyleEnums::SkinParts part,
    const QSize &size, SkinElementFlags flags)
{
    QS60WindowSurface::unlockBitmapHeap();
    QPixmap result = (flags & SF_ColorSkinned)?
          QS60StyleModeSpecifics::colorSkinnedGraphics(part, size, flags)
        : QS60StyleModeSpecifics::skinnedGraphics(part, size, flags);
    QS60WindowSurface::lockBitmapHeap();

    if (flags & SF_StateDisabled) {
        // TODO: fix this
        QStyleOption opt;
//        opt.palette = q->standardPalette();
        result = QApplication::style()->generatedIconPixmap(QIcon::Disabled, result, &opt);
    }
    return result;
}

QPixmap QS60StylePrivate::frame(SkinFrameElements frame, const QSize &size, SkinElementFlags flags)
{
    QS60WindowSurface::unlockBitmapHeap();
    QPixmap result = QS60StyleModeSpecifics::skinnedGraphics(frame, size, flags);
    QS60WindowSurface::lockBitmapHeap();

    if (flags & SF_StateDisabled) {
        // TODO: fix this
        QStyleOption opt;
//        opt.palette = q->standardPalette();
        result = QApplication::style()->generatedIconPixmap(QIcon::Disabled, result, &opt);
    }
    return result;
}

void QS60StylePrivate::setStyleProperty_specific(const char *name, const QVariant &value)
{
    if (name == QLatin1String("foo")) {
        // BaR
    } else {
        setStyleProperty(name, value);
    }
}

QVariant QS60StylePrivate::styleProperty_specific(const char *name) const
{
    if (name == QLatin1String("foo"))
        return QLatin1String("Bar");
    else
        return styleProperty(name);
}

QColor QS60StylePrivate::s60Color(QS60StyleEnums::ColorLists list,
    int index, const QStyleOption *option)
{
    static const TAknsItemID *idMap[] = {
        &KAknsIIDQsnHighlightColors,
        &KAknsIIDQsnIconColors,
        &KAknsIIDQsnLineColors,
        &KAknsIIDQsnOtherColors,
        &KAknsIIDQsnParentColors,
        &KAknsIIDQsnTextColors
    };
    Q_ASSERT((int)list <= (int)sizeof(idMap)/sizeof(idMap[0]));
    const QColor color = QS60StyleModeSpecifics::colorValue(*idMap[(int) list], index - 1);
    return option ? QS60StylePrivate::stateColor(color, option) : color;
}

// If the public SDK returns compressed images, please let us also uncompress those!
void QS60StyleModeSpecifics::unCompressBitmapL(const TRect& aTrgRect, CFbsBitmap* aTrgBitmap, CFbsBitmap* aSrcBitmap)
{
    if (!aSrcBitmap)
        User::Leave(KErrArgument);
    if (!aTrgBitmap)
        User::Leave(KErrArgument);

   // Note! aSrcBitmap->IsCompressedInRAM() is always ETrue, since this method is called only if that applies!
   ASSERT(aSrcBitmap->IsCompressedInRAM());

    TDisplayMode displayMode = aSrcBitmap->DisplayMode();

    if (displayMode != aTrgBitmap->DisplayMode())
        User::Leave(KErrArgument);

    TSize trgSize = aTrgBitmap->SizeInPixels();
    TSize srcSize = aSrcBitmap->SizeInPixels();

    // calculate the valid drawing area
    TRect drawRect = aTrgRect;
    drawRect.Intersection(TRect(TPoint(0, 0), trgSize));

    if (drawRect.IsEmpty())
        return;

    CFbsBitmap* realSource = new (ELeave) CFbsBitmap();
    CleanupStack::PushL(realSource);
    User::LeaveIfError(realSource->Create(srcSize, displayMode));
    CFbsBitmapDevice* dev = CFbsBitmapDevice::NewL(realSource);
    CleanupStack::PushL(dev);
    CFbsBitGc* gc = NULL;
    User::LeaveIfError(dev->CreateContext(gc));
    CleanupStack::PushL(gc);
    gc->BitBlt(TPoint(0, 0), aSrcBitmap);
    CleanupStack::PopAndDestroy(2); // dev, gc

    // Heap lock for FBServ large chunk is only needed with large bitmaps.
    if (realSource->IsLargeBitmap() || aTrgBitmap->IsLargeBitmap()) {
        aTrgBitmap->LockHeapLC(ETrue); // fbsheaplock
    } else {
        CleanupStack::PushL((TAny*) NULL);
    }

    TUint32* srcAddress = realSource->DataAddress();
    TUint32* trgAddress = aTrgBitmap->DataAddress();

    const TInt xSkip = (srcSize.iWidth << 8) / aTrgRect.Width();
    const TInt ySkip = (srcSize.iHeight << 8) / aTrgRect.Height();

    const TInt drawWidth = drawRect.Width();
    const TInt drawHeight = drawRect.Height();

    TRect offsetRect(aTrgRect.iTl, drawRect.iTl);
    const TInt yPosOffset = ySkip * offsetRect.Height();
    const TInt xPosOffset = xSkip * offsetRect.Width();

    if ((displayMode == EGray256) || (displayMode == EColor256)) {
        TInt srcScanLen8 = CFbsBitmap::ScanLineLength(srcSize.iWidth,
            displayMode);
        TInt trgScanLen8 = CFbsBitmap::ScanLineLength(trgSize.iWidth,
            displayMode);

        TUint8* trgAddress8 = reinterpret_cast<TUint8*> (trgAddress);

        TInt yPos = yPosOffset;
        // skip left and top margins in the beginning
        trgAddress8 += trgScanLen8 * drawRect.iTl.iY + drawRect.iTl.iX;

        for (TInt y = 0; y < drawHeight; y++) {
            TUint8* srcAddress8 = reinterpret_cast<TUint8*> (srcAddress)
                + (srcScanLen8 * (yPos >> 8));

            TInt xPos = xPosOffset;
            for (TInt x = 0; x < drawWidth; x++) {
                *(trgAddress8++) = srcAddress8[xPos >> 8];
                xPos += xSkip;
            }

            yPos += ySkip;

            trgAddress8 += trgScanLen8 - drawWidth;
        }
    } else if (displayMode == EColor4K || displayMode == EColor64K) {
        TInt srcScanLen16 = CFbsBitmap::ScanLineLength(srcSize.iWidth,
            displayMode) >>1;
        TInt trgScanLen16 = CFbsBitmap::ScanLineLength(trgSize.iWidth,
            displayMode) >>1;

        TUint16* trgAddress16 = reinterpret_cast<TUint16*> (trgAddress);

        TInt yPos = yPosOffset;
        // skip left and top margins in the beginning
        trgAddress16 += trgScanLen16 * drawRect.iTl.iY + drawRect.iTl.iX;

        for (TInt y = 0; y < drawHeight; y++) {
            TUint16* srcAddress16 = reinterpret_cast<TUint16*> (srcAddress)
                + (srcScanLen16 * (yPos >> 8));

            TInt xPos = xPosOffset;
            for (TInt x = 0; x < drawWidth; x++) {
                *(trgAddress16++) = srcAddress16[xPos >> 8];
                xPos += xSkip;
            }

            yPos += ySkip;

            trgAddress16 += trgScanLen16 - drawWidth;
        }
    } else if (displayMode == EColor16MU || displayMode == EColor16MA) {
        TInt srcScanLen32 = CFbsBitmap::ScanLineLength(srcSize.iWidth,
            displayMode) >>2;
        TInt trgScanLen32 = CFbsBitmap::ScanLineLength(trgSize.iWidth,
            displayMode) >>2;

        TUint32* trgAddress32 = reinterpret_cast<TUint32*> (trgAddress);

        TInt yPos = yPosOffset;
        // skip left and top margins in the beginning
        trgAddress32 += trgScanLen32 * drawRect.iTl.iY + drawRect.iTl.iX;

        for (TInt y = 0; y < drawHeight; y++) {
            TUint32* srcAddress32 = reinterpret_cast<TUint32*> (srcAddress)
                + (srcScanLen32 * (yPos >> 8));

            TInt xPos = xPosOffset;
            for (TInt x = 0; x < drawWidth; x++) {
                *(trgAddress32++) = srcAddress32[xPos >> 8];
                xPos += xSkip;
            }

            yPos += ySkip;

            trgAddress32 += trgScanLen32 - drawWidth;
        }
    } else { User::Leave(KErrUnknown);}

    CleanupStack::PopAndDestroy(2); // fbsheaplock, realSource
}

QSize QS60StylePrivate::screenSize()
{
    TSize mySize = QS60Data::screenDevice()->SizeInPixels();
    return QSize(mySize.iWidth, mySize.iHeight);
}

void QS60StyleModeSpecifics::colorGroupAndIndex(
    QS60StyleEnums::SkinParts skinID, TAknsItemID &colorGroup, int colorIndex)
{
    switch(skinID) {
        case QS60StyleEnums::SP_QgnIndiRadiobuttOff:
        case QS60StyleEnums::SP_QgnIndiRadiobuttOn:
        case QS60StyleEnums::SP_QgnIndiCheckboxOff:
        case QS60StyleEnums::SP_QgnIndiCheckboxOn:
            colorGroup = KAknsIIDQsnIconColors;
            colorIndex = EAknsCIQsnIconColorsCG1;
            break;
        default:
            break;
    }
}

void QS60Style::handleDynamicLayoutVariantSwitch()
{
    Q_D(QS60Style);
    d->clearCaches();
#ifdef QT_S60STYLE_LAYOUTDATA_SIMULATED
    d->setActiveLayout();
#endif // QT_S60STYLE_LAYOUTDATA_SIMULATED
    d->refreshUI();
    foreach (QWidget *widget, QApplication::allWidgets())
        d->setThemePalette(widget);
}

void QS60Style::handleSkinChange()
{
    Q_D(QS60Style);
    d->clearCaches();
    foreach (QWidget *topLevelWidget, QApplication::allWidgets()){
        d->setThemePalette(topLevelWidget);
        topLevelWidget->update();
    }
}

QT_END_NAMESPACE

#endif // QT_NO_STYLE_S60 || QT_PLUGIN
