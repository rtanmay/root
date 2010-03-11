// @(#)root/gui:$Id$
// Author: Fons Rademakers   06/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/**************************************************************************

    This source is based on Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    Xclass95 is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

**************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGButton, TGTextButton, TGPictureButton, TGCheckButton,              //
// TGRadioButton and TGSplitButton                                      //
//                                                                      //
// This header defines all GUI button widgets.                          //
//                                                                      //
// TGButton is a button abstract base class. It defines general button  //
// behaviour.                                                           //
//                                                                      //
// TGTextButton and TGPictureButton yield an action as soon as they are //
// clicked. These buttons usually provide fast access to frequently     //
// used or critical commands. They may appear alone or placed in a      //
// group.                                                               //
//                                                                      //
// The action they perform can be inscribed with a meaningful tooltip   //
// set by SetToolTipText(const char* text, Long_t delayms=400).         //
//                                                                      //
// The text button has a label indicating the action to be taken when   //
// the button is pressed. The text can be a hot string ("&Exit") that   //
// defines the label "Exit" and keyboard mnemonics Alt+E for button     //
// selection. A button label can be changed by SetText(new_label).      //
//                                                                      //
// Selecting a text or picture button will generate the event:          //
// kC_COMMAND, kCM_BUTTON, button id, user data.                        //
//                                                                      //
// The purpose of TGCheckButton and TGRadioButton is for selecting      //
// different options. Like text buttons, they have text or hot string   //
// as a label.                                                          //
//                                                                      //
// Radio buttons are grouped usually in logical sets of two or more     //
// buttons to present mutually exclusive choices.                       //
//                                                                      //
// Selecting a check button will generate the event:                    //
// kC_COMMAND, kCM_CHECKBUTTON, button id, user data.                   //
//                                                                      //
// Selecting a radio button will generate the event:                    //
// kC_COMMAND, kCM_RADIOBUTTON, button id, user data.                   //
//                                                                      //
// If a command string has been specified (via SetCommand()) then this  //
// command string will be executed via the interpreter whenever a       //
// button is selected. A command string can contain the macros:         //
// $MSG   -- kC_COMMAND, kCM[CHECK|RADIO]BUTTON packed message          //
//           (use GET_MSG() and GET_SUBMSG() to unpack)                 //
// $PARM1 -- button id                                                  //
// $PARM2 -- user data pointer                                          //
// Before executing these macros are expanded into the respective       //
// Long_t's                                                             //
//                                                                      //
// TGSplitButton implements a button with added menu functionality.     //
// There are 2 modes of operation available.                            //
//                                                                      //
// If the button is split, a menu will popup when the menu area of the  //
// button is clicked. Activating a menu item changes the functionality  //
// of the button by having it emit a additional signal when it is       //
// clicked. The signal emitted when the button is clicked, is the       //
// ItemClicked(Int_t) signal with a different fixed value for the       //
// Int_t that corresponds to the id of the activated menu entry.        //
//                                                                      //
// If the button is not split, clicking it will popup the menu and the  //
// ItemClicked(Int_t) signal will be emitted when a menu entry is       //
// acitvated. The value of the Int_t is again equal to the value of     //
// the id of the activated menu entry.                                  //
//                                                                      //
// The mode of operation of a SplitButton can be changed on the fly     //
// by calling the SetSplit(Bool_t) method.                              //
//////////////////////////////////////////////////////////////////////////

#include "TGButton.h"
#include "TGWidget.h"
#include "TGPicture.h"
#include "TGToolTip.h"
#include "TGButtonGroup.h"
#include "TGResourcePool.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TImage.h"
#include "TEnv.h"
#include "TClass.h"
#include "TGMenu.h"
#include "KeySymbols.h"

const TGGC *TGButton::fgHibckgndGC = 0;
const TGGC *TGButton::fgDefaultGC = 0;

const TGFont *TGTextButton::fgDefaultFont = 0;

const TGFont *TGCheckButton::fgDefaultFont = 0;
const TGGC   *TGCheckButton::fgDefaultGC = 0;

const TGFont *TGRadioButton::fgDefaultFont = 0;
const TGGC   *TGRadioButton::fgDefaultGC = 0;

Window_t TGButton::fgReleaseBtn = 0;

ClassImp(TGButton)
ClassImp(TGTextButton)
ClassImp(TGPictureButton)
ClassImp(TGCheckButton)
ClassImp(TGRadioButton)
ClassImp(TGSplitButton)

//______________________________________________________________________________
TGButton::TGButton(const TGWindow *p, Int_t id, GContext_t norm, UInt_t options)
    : TGFrame(p, 1, 1, options)
{
   // Create button base class part.
 
   fWidgetId    = id;
   fWidgetFlags = kWidgetWantFocus;
   fMsgWindow   = p;
   fUserData    = 0;
   fTip         = 0;
   fGroup       = 0;

   fNormGC   = norm;
   fState    = kButtonUp;
   fStayDown = kFALSE;
   fWidgetFlags = kWidgetIsEnabled;

   if (p && p->IsA()->InheritsFrom(TGButtonGroup::Class())) {
      TGButtonGroup *bg = (TGButtonGroup*) p;
      bg->Insert(this, id);
   }

   gVirtualX->GrabButton(fId, kButton1, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask,
                         kNone, kNone);

   AddInput(kEnterWindowMask | kLeaveWindowMask);
   SetWindowName();
}

//______________________________________________________________________________
TGButton::~TGButton()
{
   // Delete button.

   // remove from button group
   if (fGroup) {
      fGroup->Remove(this);
      fGroup = 0;
   }

   if (fTip) delete fTip;
}

//______________________________________________________________________________
void TGButton::SetState(EButtonState state, Bool_t emit)
{
   // Set button state.

   Bool_t was = !IsDown();   // kTRUE if button was off

   if (state == kButtonDisabled)
      fWidgetFlags &= ~kWidgetIsEnabled;
   else 
      fWidgetFlags |= kWidgetIsEnabled;
   if (state != fState) {
      switch (state) {
         case kButtonEngaged:
         case kButtonDown:
            fOptions &= ~kRaisedFrame;
            fOptions |= kSunkenFrame;
            break;
         case kButtonDisabled:
         case kButtonUp:
            fOptions &= ~kSunkenFrame;
            fOptions |= kRaisedFrame;
            break;
      }
      fState = state;
      DoRedraw();
      if (emit || fGroup) EmitSignals(was);
   }
}

//______________________________________________________________________________
void TGButton::SetDown(Bool_t on, Bool_t emit)
{

   // Set button state down according to the parameter 'on'.

   if (GetState() == kButtonDisabled) return;

   SetState(on ? kButtonDown : kButtonUp, emit);
}

//______________________________________________________________________________
void TGButton::SetGroup(TGButtonGroup *group)
{
   // Sets new button-group for this button.

   fGroup = group;
}

//______________________________________________________________________________
Bool_t TGButton::HandleButton(Event_t *event)
{
   // Handle mouse button event.

   Bool_t click = kFALSE;

   if (fTip) fTip->Hide();

   if (fState == kButtonDisabled) return kTRUE;

   Bool_t in = (event->fX >= 0) && (event->fY >= 0) &&
               (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);

   // We don't need to check the button number as GrabButton will
   // only allow button1 events
   if (event->fType == kButtonPress) {
      fgReleaseBtn = 0;

      if (fState == kButtonEngaged) {
         return kTRUE;
      }
      if (in) SetState(kButtonDown, kTRUE);
   } else { // ButtonRelease
      if (fState == kButtonEngaged) {
         if (in) SetState(kButtonUp, kTRUE);
         click = kTRUE;
      } else {
         click = (fState == kButtonDown) && in;
         if (click && fStayDown) {
            if (in) {
               SetState(kButtonEngaged, kTRUE);
               fgReleaseBtn = 0;
            }
         } else {
            if (in) {
               SetState(kButtonUp, kTRUE);
               fgReleaseBtn = fId;
            }
         }
      }
   }
   if (click) {
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }

   return kTRUE;
}

//______________________________________________________________________________
void TGButton::EmitSignals(Bool_t was)
{
   // Emit button signals.

   Bool_t now = !IsDown();       // kTRUE if button now is off

   // emit signals
   if (was && !now) {
      Pressed();                 // emit Pressed  = was off , now on
      if (fStayDown) Clicked();  // emit Clicked
   }
   if (!was && now) {
      Released();                // emit Released = was on , now off
      Clicked();                 // emit Clicked
   }
   if ((was != now) && IsToggleButton()) Toggled(!now); // emit Toggled  = was != now
}

//______________________________________________________________________________
Bool_t TGButton::HandleCrossing(Event_t *event)
{
   // Handle mouse crossing event.

   if (fTip) {
      if (event->fType == kEnterNotify)
         fTip->Reset();
      else
         fTip->Hide();
   }

   if ((fgDbw != event->fWindow) || (fgReleaseBtn == event->fWindow)) return kTRUE;

   if (!(event->fState & (kButton1Mask | kButton2Mask | kButton3Mask)))
      return kTRUE;

   if (fState == kButtonEngaged || fState == kButtonDisabled) return kTRUE;

   if (event->fType == kLeaveNotify) {
      fgReleaseBtn = fId;
      SetState(kButtonUp, kFALSE);
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGButton::SetToolTipText(const char *text, Long_t delayms)
{
   // Set tool tip text associated with this button. The delay is in
   // milliseconds (minimum 250). To remove tool tip call method with
   // text = 0.

   if (fTip) {
      delete fTip;
      fTip = 0;
   }

   if (text && strlen(text))
      fTip = new TGToolTip(fClient->GetDefaultRoot(), this, text, delayms);
}

//______________________________________________________________________________
void TGButton::SetEnabled(Bool_t e)
{
   // Set enabled or disabled state of button

   SetState(e ? kButtonUp : kButtonDisabled);

   if (e) fWidgetFlags |= kWidgetIsEnabled;
   else   fWidgetFlags &= ~kWidgetIsEnabled;
}

//______________________________________________________________________________
const TGGC &TGButton::GetDefaultGC()
{
   // Return default graphics context.

   if (!fgDefaultGC)
      fgDefaultGC = gClient->GetResourcePool()->GetFrameGC();
   return *fgDefaultGC;
}

//______________________________________________________________________________
const TGGC &TGButton::GetHibckgndGC()
{
   // Return graphics context for highlighted frame background.

   if (!fgHibckgndGC) {
      GCValues_t gval;
      gval.fMask = kGCForeground | kGCBackground | kGCTile |
                   kGCFillStyle  | kGCGraphicsExposures;
      gval.fForeground = gClient->GetResourcePool()->GetFrameHiliteColor();
      gval.fBackground = gClient->GetResourcePool()->GetFrameBgndColor();
      gval.fFillStyle  = kFillTiled;
      gval.fTile       = gClient->GetResourcePool()->GetCheckeredPixmap();
      gval.fGraphicsExposures = kFALSE;
      fgHibckgndGC = gClient->GetGC(&gval, kTRUE);
   }
   return *fgHibckgndGC;
}


//______________________________________________________________________________
TGTextButton::TGTextButton(const TGWindow *p, TGHotString *s, Int_t id,
                           GContext_t norm, FontStruct_t font,
                           UInt_t options) : TGButton(p, id, norm, options)
{
   // Create a text button widget. The hotstring will be adopted and deleted
   // by the text button.

   fLabel = s;
   fFontStruct = font;

   Init();
}

//______________________________________________________________________________
TGTextButton::TGTextButton(const TGWindow *p, const char *s, Int_t id,
                           GContext_t norm, FontStruct_t font,
                           UInt_t options) : TGButton(p, id, norm, options)
{
   // Create a text button widget.

   fLabel = new TGHotString(!p && !s ? GetName() : s);
   fFontStruct = font;

   Init();
}

//______________________________________________________________________________
TGTextButton::TGTextButton(const TGWindow *p, const char *s, const char *cmd,
                           Int_t id, GContext_t norm, FontStruct_t font,
                           UInt_t options) : TGButton(p, id, norm, options)
{
   // Create a text button widget and set cmd string at same time.

   fLabel = new TGHotString(s);
   fFontStruct = font;
   fCommand = cmd;

   Init();
}

//______________________________________________________________________________
void TGTextButton::Init()
{
   // Common initialization used by the different ctors.

   int hotchar;

   fTMode       = kTextCenterX | kTextCenterY;
   fHKeycode    = 0;
   fHasOwnFont  = kFALSE;
   fPrevStateOn =
   fStateOn     = kFALSE;
   fWrapLength  = -1;
   fMLeft = fMRight = fMTop = fMBottom = 0;

   TGFont *font = fClient->GetFontPool()->FindFont(fFontStruct);
   if (!font) {
      font = fClient->GetFontPool()->GetFont(fgDefaultFont);
      fFontStruct = font->GetFontStruct();
   }

   fTLayout = font->ComputeTextLayout(fLabel->GetString(), fLabel->GetLength(),
                                      fWrapLength, kTextLeft, 0,
                                      &fTWidth, &fTHeight);

   Resize();
   fWidth = fTWidth;
   fHeight = fTHeight;

   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0) {
         const TGMainFrame *main = (TGMainFrame *) GetMainFrame();
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
   }
   SetWindowAttributes_t wattr;
   wattr.fMask = kWAWinGravity | kWABitGravity;
   wattr.fBitGravity = 5; // center
   wattr.fWinGravity = 1;
   gVirtualX->ChangeWindowAttributes(fId, &wattr);

   SetWindowName();
}

//______________________________________________________________________________
TGTextButton::~TGTextButton()
{
   // Delete a text button widget.

   if (fHKeycode && (fParent->MustCleanup() != kDeepCleanup)) {
      const TGMainFrame *main = (TGMainFrame *) GetMainFrame();
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
      main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
   }
   if (fLabel) delete fLabel;
   if (fHasOwnFont) {
      TGGCPool *pool = fClient->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);
      pool->FreeGC(gc);
   }

   delete fTLayout;
}

//______________________________________________________________________________
void TGTextButton::Layout()
{
   // layout text button

   delete fTLayout;

   TGFont *font = fClient->GetFontPool()->FindFont(fFontStruct);
   if (!font) {
      font = fClient->GetFontPool()->GetFont(fgDefaultFont);
      fFontStruct = font->GetFontStruct();
   }

   fTLayout = font->ComputeTextLayout(fLabel->GetString(), fLabel->GetLength(),
                                      fWrapLength, kTextLeft, 0,
                                      &fTWidth, &fTHeight);
   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
void TGTextButton::SetText(TGHotString *new_label)
{
   // Set new button text.

   int hotchar;
   const TGMainFrame *main = (TGMainFrame *) GetMainFrame();

   if (fLabel) {
      if (fHKeycode) {
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
      delete fLabel;
   }
   
   fLabel = new_label;
   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0)
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
   }

   Layout();
}

//______________________________________________________________________________
void TGTextButton::SetText(const TString &new_label)
{
   // Set new button text.

   SetText(new TGHotString(new_label));
}

//______________________________________________________________________________
void TGTextButton::SetTextJustify(Int_t mode)
{
   // Set text justification. Mode is an OR of the bits:
   // kTextTop, kTextBottom, kTextLeft, kTextRight, kTextCenterX and
   // kTextCenterY.

   fTMode = mode;

   SetWindowAttributes_t wattr;
   wattr.fMask = kWAWinGravity | kWABitGravity;
   wattr.fWinGravity = 1;

   switch (mode) {
      case kTextTop | kTextLeft:
         wattr.fBitGravity = 1; //NorthWestGravity
         break;
      case kTextTop | kTextCenterX:
      case kTextTop:
         wattr.fBitGravity = 2; //NorthGravity
         break;
      case kTextTop | kTextRight:
         wattr.fBitGravity = 3; //NorthEastGravity
         break;
      case kTextLeft | kTextCenterY:
      case kTextLeft:
         wattr.fBitGravity = 4; //WestGravity
         break;
      case kTextCenterY | kTextCenterX:
         wattr.fBitGravity = 5; //CenterGravity
         break;
      case kTextRight | kTextCenterY:
      case kTextRight:
         wattr.fBitGravity = 6; //EastGravity
         break;
      case kTextBottom | kTextLeft:
         wattr.fBitGravity = 7; //SouthWestGravity
         break;
      case kTextBottom | kTextCenterX:
      case kTextBottom:
         wattr.fBitGravity = 8; //SouthGravity
         break;
      case kTextBottom | kTextRight:
         wattr.fBitGravity = 9; //SouthEastGravity
         break;
      default:
         wattr.fBitGravity = 5; //CenterGravity
         break;
   }

   gVirtualX->ChangeWindowAttributes(fId, &wattr);
   Layout();
}

//______________________________________________________________________________
void TGTextButton::DoRedraw()
{
   // Draw the text button.

   int x, y;

   TGFrame::DoRedraw();

   if (fTMode & kTextLeft) {
      x = fMLeft + 4;
   } else if (fTMode & kTextRight) {
      x = fWidth - fTWidth - fMRight - 4;
   } else {
      x = (fWidth - fTWidth + fMLeft - fMRight) >> 1;
   }

   if (fTMode & kTextTop) {
      y = fMTop + 3;
   } else if (fTMode & kTextBottom) {
      y = fHeight - fTHeight - fMBottom - 3;
   } else {
      y = (fHeight - fTHeight + fMTop - fMBottom) >> 1;
   }

   if (fState == kButtonDown || fState == kButtonEngaged) { ++x; ++y; }
   if (fState == kButtonEngaged) {
      gVirtualX->FillRectangle(fId, GetHibckgndGC()(), 2, 2, fWidth-4, fHeight-4);
      gVirtualX->DrawLine(fId, GetHilightGC()(), 2, 2, fWidth-3, 2);
   }

   Int_t hotpos = fLabel->GetHotPos();

   if (fState == kButtonDisabled) {
      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);
      Pixel_t fore = gc->GetForeground();
      Pixel_t hi = GetHilightGC().GetForeground();
      Pixel_t sh = GetShadowGC().GetForeground();

      gc->SetForeground(hi);
      fTLayout->DrawText(fId, gc->GetGC(), x + 1, y + 1, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x + 1, y + 1, hotpos - 1);

      gc->SetForeground(sh);
      fTLayout->DrawText(fId, gc->GetGC(), x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x, y, hotpos - 1);
      gc->SetForeground(fore);
   } else {
      fTLayout->DrawText(fId, fNormGC, x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, fNormGC, x, y, hotpos - 1);
   }
}

//______________________________________________________________________________
Bool_t TGTextButton::HandleKey(Event_t *event)
{
   // Handle key event. This function will be called when the hotkey is hit.

   Bool_t click = kFALSE;
   Bool_t was = !IsDown();   // kTRUE if button was off

   if (event->fType == kGKeyPress) {
      gVirtualX->SetKeyAutoRepeat(kFALSE);
   } else {
      gVirtualX->SetKeyAutoRepeat(kTRUE);
   }

   if (fTip && event->fType == kGKeyPress) fTip->Hide();

   if (fState == kButtonDisabled) return kTRUE;

   // We don't need to check the key number as GrabKey will only
   // allow fHotchar events if Alt button is pressed (kKeyMod1Mask)

   if ((event->fType == kGKeyPress) && (event->fState & kKeyMod1Mask)) {
      if (fState == kButtonEngaged) return kTRUE;
      SetState(kButtonDown);
   } else if ((event->fType == kKeyRelease) && (event->fState & kKeyMod1Mask)) {
      if (fState == kButtonEngaged /*&& !allowRelease*/) return kTRUE;
      click = (fState == kButtonDown);
      if (click && fStayDown) {
         SetState(kButtonEngaged);
      } else {
         SetState(kButtonUp);
      }
   }
   if (click) {
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }
   EmitSignals(was);

   return kTRUE;
}

//______________________________________________________________________________
TGDimension TGTextButton::GetDefaultSize() const
{
   // returns default size

   UInt_t w = GetOptions() & kFixedWidth ? fWidth : fTWidth + fMLeft + fMRight + 8;
   UInt_t h = GetOptions() & kFixedHeight ? fHeight : fTHeight + fMTop + fMBottom + 7;
   return TGDimension(w, h);
}

//______________________________________________________________________________
FontStruct_t TGTextButton::GetDefaultFontStruct()
{
   // Return default font structure.

   if (!fgDefaultFont)
      fgDefaultFont = gClient->GetResourcePool()->GetDefaultFont();
   return fgDefaultFont->GetFontStruct();
}

//______________________________________________________________________________
void TGTextButton::SetFont(FontStruct_t font, Bool_t global)
{
   // Changes text font.
   // If global is kTRUE font is changed globally, otherwise - locally.

   if (font != fFontStruct) {
      FontH_t v = gVirtualX->GetFontHandle(font);
      if (!v) return;

      fFontStruct = font;
      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);

      if (!global) {
         gc = pool->GetGC((GCValues_t*)gc->GetAttributes(), kTRUE); // copy
         fHasOwnFont = kTRUE;
      }

      gc->SetFont(v);

      fNormGC = gc->GetGC();
      Layout();
   }
}

//______________________________________________________________________________
void TGTextButton::SetFont(const char *fontName, Bool_t global)
{
   // Changes text font specified by name.
   // If global is true color is changed globally, otherwise - locally.

   TGFont *font = fClient->GetFont(fontName);
   if (font) {
      SetFont(font->GetFontStruct(), global);
   }
}

//______________________________________________________________________________
void TGTextButton::SetTextColor(Pixel_t color, Bool_t global)
{
   // Changes text color.
   // If global is true color is changed globally, otherwise - locally.

   TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
   TGGC *gc = pool->FindGC(fNormGC);

   if (!global) {
      gc = pool->GetGC((GCValues_t*)gc->GetAttributes(), kTRUE); // copy
      fHasOwnFont = kTRUE;
   }

   gc->SetForeground(color);
   fNormGC = gc->GetGC();
   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
Bool_t TGTextButton::HasOwnFont() const
{
   // Returns kTRUE if text attributes are unique,
   // returns kFALSE if text attributes are shared (global).

   return fHasOwnFont;
}

//______________________________________________________________________________
TGPictureButton::TGPictureButton(const TGWindow *p, const TGPicture *pic,
      Int_t id, GContext_t norm, UInt_t option) : TGButton(p, id, norm, option)
{
   // Create a picture button widget. The picture is not adopted and must
   // later be freed by the user once the picture button is deleted (a single
   // picture reference might be used by other buttons).

   if (!pic) {
      Error("TGPictureButton", "pixmap not found for button %d", id);
      fPic = fClient->GetPicture("mb_question_s.xpm");
   } else {
      fPic = pic;
   }

   if (fPic) {
      fTWidth  = fPic->GetWidth();
      fTHeight = fPic->GetHeight();

      Resize(fTWidth  + (fBorderWidth << 1) + fBorderWidth + 1,
             fTHeight + (fBorderWidth << 1) + fBorderWidth); // *3
   }
   fPicD = 0;
   fOwnDisabledPic = kFALSE;
   SetWindowName();
}

//______________________________________________________________________________
TGPictureButton::TGPictureButton(const TGWindow *p, const TGPicture *pic,
      const char *cmd, Int_t id, GContext_t norm, UInt_t option)
   : TGButton(p, id, norm, option)
{
   // Create a picture button widget and set action command. The picture is
   // not adopted and must later be freed by the user once the picture button
   // is deleted (a single picture reference might be used by other
   // buttons).

   if (!pic) {
      Error("TGPictureButton", "pixmap not found for button\n%s",
            cmd ? cmd : "");
      fPic = fClient->GetPicture("mb_question_s.xpm");
   } else {
      fPic = pic;
   }

   fCommand = cmd;

   if (fPic) {
      fTWidth  = fPic->GetWidth();
      fTHeight = fPic->GetHeight();

      Resize(fTWidth  + (fBorderWidth << 1) + fBorderWidth + 1,
             fTHeight + (fBorderWidth << 1) + fBorderWidth); // *3
   }
   fPicD = 0;
   fOwnDisabledPic = kFALSE;
   SetWindowName();
}

//______________________________________________________________________________
TGPictureButton::TGPictureButton(const TGWindow *p, const char *pic,
   Int_t id, GContext_t norm, UInt_t option) : TGButton(p, id, norm, option)
{
   // Create a picture button. Where pic is the file name of the picture.

   if (!pic || !strlen(pic)) {
      if (p) Error("TGPictureButton", "pixmap not found for button");
      fPic = fClient->GetPicture("mb_question_s.xpm");
   } else {
      fPic = fClient->GetPicture(pic);
   }

   if (fPic) {
      fTWidth  = fPic->GetWidth();
      fTHeight = fPic->GetHeight();

      Resize(fTWidth  + (fBorderWidth << 1) + fBorderWidth + 1,
             fTHeight + (fBorderWidth << 1) + fBorderWidth); // *3
   }
   fPicD = 0;
   fOwnDisabledPic = kFALSE;
   SetWindowName();
}

//______________________________________________________________________________
TGPictureButton::~TGPictureButton()
{
   // Destructor.

   if (fOwnDisabledPic) fClient->FreePicture(fPicD);
}

//______________________________________________________________________________
void TGPictureButton::SetPicture(const TGPicture *new_pic)
{
   // Change a picture in a picture button. The picture is not adopted and
   // must later be freed by the user once the picture button is deleted
   // (a single picture reference might be used by other buttons).

   if (!new_pic) {
      Error("SetPicture", "pixmap not found for button %d\n%s",
            fWidgetId, fCommand.Data());
      return;
   }

   fPic = new_pic;

   if (fState == kButtonDisabled) {
      fClient->FreePicture(fPicD);
      fPicD = 0;
   }

   fTWidth  = fPic->GetWidth();
   fTHeight = fPic->GetHeight();

   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
void TGPictureButton::DoRedraw()
{
   // Redraw picture button.

   if (!fPic) {
      TGFrame::DoRedraw();
      return;
   }

   int x = (fWidth - fTWidth) >> 1;
   int y = (fHeight - fTHeight) >> 1;

   TGFrame::DoRedraw();
   if (fState == kButtonDown || fState == kButtonEngaged) { ++x; ++y; }
   if (fState == kButtonEngaged) {
      gVirtualX->FillRectangle(fId, GetHibckgndGC()(), 2, 2, fWidth-4, fHeight-4);
      gVirtualX->DrawLine(fId, GetHilightGC()(), 2, 2, fWidth-3, 2);
   }

   const TGPicture *pic = fPic;
   if (fState == kButtonDisabled) {
      if (!fPicD) CreateDisabledPicture();
      pic = fPicD ? fPicD : fPic;
   }

   pic->Draw(fId, fNormGC, x, y);
}

//______________________________________________________________________________
void TGPictureButton::CreateDisabledPicture()
{
   // Creates disabled picture.

   TImage *img = TImage::Create();
   TImage *img2 = TImage::Create();

   TString back = gEnv->GetValue("Gui.BackgroundColor", "#c0c0c0");
   img2->FillRectangle(back.Data(), 0, 0, fPic->GetWidth(), fPic->GetHeight());
   img->SetImage(fPic->GetPicture(), fPic->GetMask());
   Pixmap_t mask = img->GetMask();
   img2->Merge(img, "overlay");

   TString name = "disbl_";
   name += fPic->GetName();
   fPicD = fClient->GetPicturePool()->GetPicture(name.Data(), img2->GetPixmap(),
                                                 mask);
   fOwnDisabledPic = kTRUE;
   delete img;
   delete img2;
}

//______________________________________________________________________________
void TGPictureButton::SetDisabledPicture(const TGPicture *pic)
{
   // Changes disabled picture.

   if (!pic) return;

   if (fOwnDisabledPic && fPicD) fClient->FreePicture(fPicD);
   fPicD = pic;
   ((TGPicture*)pic)->AddReference();
   fOwnDisabledPic = kFALSE;
}

//______________________________________________________________________________
TGCheckButton::TGCheckButton(const TGWindow *p, TGHotString *s, Int_t id,
                             GContext_t norm, FontStruct_t font, UInt_t option)
   : TGTextButton(p, s, id, norm, font, option)
{
   // Create a check button widget. The hotstring will be adopted and deleted
   // by the check button.

   Init();
}

//______________________________________________________________________________
TGCheckButton::TGCheckButton(const TGWindow *p, const char *s, Int_t id,
                             GContext_t norm, FontStruct_t font, UInt_t option)
   : TGTextButton(p, s, id, norm, font, option)
{
   // Create a check button widget.

   Init();
}

//______________________________________________________________________________
TGCheckButton::TGCheckButton(const TGWindow *p, const char *s, const char *cmd,
                             Int_t id, GContext_t norm, FontStruct_t font,
                             UInt_t option) : TGTextButton(p, s, cmd, id, norm, font, option)
{
   // Create a check button widget.

   Init();
}

//______________________________________________________________________________
void TGCheckButton::Init()
{
   // Common check button initialization.

   fPrevState =
   fState     = kButtonUp;
   fHKeycode = 0;

   fOn  = fClient->GetPicture("checked_t.xpm");
   fOff = fClient->GetPicture("unchecked_t.xpm");
   fDisOn  = fClient->GetPicture("checked_dis_t.xpm");
   fDisOff = fClient->GetPicture("unchecked_dis_t.xpm");

   Resize();

   if (!fOn) {
      Error("TGCheckButton", "checked_t.xpm not found");
   } else if (!fOff) {     
      Error("TGCheckButton", "unchecked_t.xpm not found");
   } else if (!fDisOn) {     
      Error("TGCheckButton", "checked_dis_t.xpm not found");
   } else if (!fDisOff) {     
      Error("TGCheckButton", "unchecked_dis_t.xpm not found");
   }
   int hotchar;

   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0) {
         const TGMainFrame *main = (TGMainFrame *) GetMainFrame();
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
   }
   SetWindowName();
}

//______________________________________________________________________________
TGCheckButton::~TGCheckButton()
{
   // Delete a check button.
   
   if (fOn)  fClient->FreePicture(fOn);
   if (fOff) fClient->FreePicture(fOff);
   if (fDisOn)  fClient->FreePicture(fDisOn);
   if (fDisOff) fClient->FreePicture(fDisOff);
}

//______________________________________________________________________________
TGDimension TGCheckButton::GetDefaultSize() const
{
   // default size

   UInt_t w = !fTWidth ? fOff->GetWidth() : fTWidth + fOff->GetWidth() + 9;
   UInt_t h = !fTHeight ? fOff->GetHeight() : fTHeight + 2;                      

   w = GetOptions() & kFixedWidth ? fWidth : w;
   h = GetOptions() & kFixedHeight ? fHeight : h;

   return TGDimension(w, h);           
}

//______________________________________________________________________________
void TGCheckButton::SetState(EButtonState state, Bool_t emit)
{
   // Set check button state.

   if (state == kButtonDisabled)
      fWidgetFlags &= ~kWidgetIsEnabled;
   else 
      fWidgetFlags |= kWidgetIsEnabled;
   PSetState(state, emit);
}

//______________________________________________________________________________
void TGCheckButton::EmitSignals(Bool_t /*wasUp*/)
{
   // Emit signals.

   if (fState == kButtonUp)   Released();            // emit Released
   if (fState == kButtonDown) Pressed();             // emit Pressed
   Clicked();                                        // emit Clicked
   Toggled(fStateOn);                                // emit Toggled
}

//______________________________________________________________________________
void TGCheckButton::PSetState(EButtonState state, Bool_t emit)
{
   // Set check button state.

   if (state != fState) {
      if (state == kButtonUp) {
         if (fPrevState == kButtonDisabled) {
            if (fStateOn) {
               fState = kButtonDown;
               fPrevState = kButtonDown;
            } else {
               fState = state;
               fPrevState = state;
            }
         } else if (fPrevState == kButtonDown) {
            fStateOn = kFALSE;
            fState = state;
            fPrevState = state;
         }
      } else if (state == kButtonDown) {
         fStateOn = kTRUE;
         fState = state;
         fPrevState = state;
      } else {
         fState = state;
         fPrevState = state;
      }
      if (emit) {
         // button signals
         EmitSignals();
      }
      DoRedraw();
   }
}

//______________________________________________________________________________
void TGCheckButton::SetDisabledAndSelected(Bool_t enable) 
{
   // Set the state of a check button to disabled and either on or
   // off.

   if (!enable) {
      if (fState == kButtonDisabled && fStateOn) {
         PSetState(kButtonUp, kFALSE);         // enable button
         PSetState(kButtonUp, kFALSE);         // set button up
         PSetState(kButtonDisabled, kFALSE);   // disable button
      } else {
         PSetState(kButtonUp, kFALSE);
         PSetState(kButtonDisabled, kFALSE);
      }
   } else {
      PSetState(kButtonDown, kFALSE);          // set button down
      PSetState(kButtonDisabled, kFALSE);      // disable button
   }
}

//______________________________________________________________________________
Bool_t TGCheckButton::HandleButton(Event_t *event)
{
   // Handle mouse button event.

   Bool_t click = kFALSE;

   if (fTip) fTip->Hide();

   if (fState == kButtonDisabled) return kTRUE;

   Bool_t in = (event->fX >= 0) && (event->fY >= 0) &&
               (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);

   // We don't need to check the button number as GrabButton will
   // only allow button1 events
   if (event->fType == kButtonPress) {
      fgReleaseBtn = 0;
      if (in) {
         fOptions |= kSunkenFrame;
         Pressed();
      }
   } else { // ButtonRelease
      if (in) {
         PSetState((fPrevState == kButtonUp) ? kButtonDown : kButtonUp, kFALSE);
         click = kTRUE;
         fPrevStateOn = fStateOn;
         Released();
      }
      fgReleaseBtn = fId;
      fOptions &= ~kSunkenFrame;
   }
   if (click) {
      Clicked();
      Toggled(fStateOn);
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_CHECKBUTTON),
                  fWidgetId, (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_CHECKBUTTON),
                           fWidgetId, (Long_t) fUserData);
   }
   DoRedraw();
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGCheckButton::HandleCrossing(Event_t *event)
{
   // Handle mouse crossing event.

   if (fTip) {
      if (event->fType == kEnterNotify)
         fTip->Reset();
      else
         fTip->Hide();
   }

   if ((fgDbw != event->fWindow) || (fgReleaseBtn == event->fWindow)) return kTRUE;

   if (!(event->fState & (kButton1Mask | kButton2Mask | kButton3Mask)))
      return kTRUE;

   if (fState == kButtonDisabled) return kTRUE;

   if (event->fType == kEnterNotify) {
      fOptions |= kSunkenFrame;
   } else {
      fOptions &= ~kSunkenFrame;
   }
   DoRedraw();

   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGCheckButton::HandleKey(Event_t *event)
{
   // Handle key event. This function will be called when the hotkey is hit.

   Bool_t click = kFALSE;

   if (event->fType == kGKeyPress)
      gVirtualX->SetKeyAutoRepeat(kFALSE);
   else
      gVirtualX->SetKeyAutoRepeat(kTRUE);

   if (fTip && event->fType == kGKeyPress) fTip->Hide();

   if (fState == kButtonDisabled) return kTRUE;

   // We don't need to check the key number as GrabKey will only
   // allow fHotchar events if Alt button is pressed (kKeyMod1Mask)

   if ((event->fType == kGKeyPress) && (event->fState & kKeyMod1Mask)) {
      PSetState((fPrevState == kButtonUp) ? kButtonDown : kButtonUp, kTRUE);
   } else if ((event->fType == kKeyRelease) && (event->fState & kKeyMod1Mask)) {
      click = (fState != fPrevState);
      fPrevState = fState;
   }
   if (click) {
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_CHECKBUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_CHECKBUTTON), fWidgetId,
                           (Long_t) fUserData);
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGCheckButton::DoRedraw()
{
   // Draw the check button widget.

   int x, y, y0, cw;

   TGFrame::DoRedraw();

   x = 20;
   y = (fHeight - fTHeight) >> 1;

   cw = 13;
   y0 = !fTHeight ? 0 : y + 1;

   if (fStateOn) {
      if (fOn) fOn->Draw(fId, fNormGC, 0, y0);
   } else {
      if (fOff) fOff->Draw(fId, fNormGC, 0, y0);
   }

   Int_t hotpos = fLabel->GetHotPos();

   if (fState == kButtonDisabled) {
      if (fStateOn == kTRUE) {
         if (fDisOn) fDisOn->Draw(fId, fNormGC, 0, y0);
      } else {
         if (fDisOff) fDisOff->Draw(fId, fNormGC, 0, y0);
      }

      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);
      Pixel_t fore = gc->GetForeground();
      Pixel_t hi = GetHilightGC().GetForeground();
      Pixel_t sh = GetShadowGC().GetForeground();

      gc->SetForeground(hi);
      fTLayout->DrawText(fId, gc->GetGC(), x + 1, y + 1, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x, y, hotpos - 1);

      gc->SetForeground(sh);
      fTLayout->DrawText(fId, gc->GetGC(), x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x, y, hotpos - 1);

      gc->SetForeground(fore);
   } else {
      fTLayout->DrawText(fId, fNormGC, x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, fNormGC, x, y, hotpos - 1);
   }
}

//______________________________________________________________________________
FontStruct_t TGCheckButton::GetDefaultFontStruct()
{
   // Return default font structure.

   if (!fgDefaultFont)
      fgDefaultFont = gClient->GetResourcePool()->GetDefaultFont();
   return fgDefaultFont->GetFontStruct();
}

//______________________________________________________________________________
const TGGC &TGCheckButton::GetDefaultGC()
{
   // Return default graphics context.

   if (!fgDefaultGC)
      fgDefaultGC = gClient->GetResourcePool()->GetFrameGC();
   return *fgDefaultGC;
}


//______________________________________________________________________________
TGRadioButton::TGRadioButton(const TGWindow *p, TGHotString *s, Int_t id,
                             GContext_t norm, FontStruct_t font, UInt_t option)
   : TGTextButton(p, s, id, norm, font, option)
{
   // Create a radio button widget. The hotstring will be adopted and deleted
   // by the radio button.

   Init();
}

//______________________________________________________________________________
TGRadioButton::TGRadioButton(const TGWindow *p, const char *s, Int_t id,
                             GContext_t norm, FontStruct_t font, UInt_t option)
   : TGTextButton(p, s, id, norm, font, option)
{
   // Create a radio button widget.

   Init();
}
//______________________________________________________________________________
TGRadioButton::TGRadioButton(const TGWindow *p, const char *s, const char *cmd,
                             Int_t id, GContext_t norm,
                             FontStruct_t font, UInt_t option)
    : TGTextButton(p, s, cmd, id, norm, font, option)
{
   // Create a radio button widget.

   Init();
}

//______________________________________________________________________________
void TGRadioButton::Init()
{
   // Common radio button initialization.

   fPrevState =
   fState     = kButtonUp;
   fHKeycode  = 0;

   fOn  = fClient->GetPicture("rbutton_on.xpm");
   fOff = fClient->GetPicture("rbutton_off.xpm");
   fDisOn  = fClient->GetPicture("rbutton_dis_on.xpm");
   fDisOff = fClient->GetPicture("rbutton_dis_off.xpm");

   if (!fOn || !fOff || !fDisOn || !fDisOff)
      Error("TGRadioButton", "rbutton_*.xpm not found");

   Resize();

   int hotchar;

   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0) {
         const TGMainFrame *main = (TGMainFrame *) GetMainFrame();
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
   }

   if (fParent->IsA()->InheritsFrom(TGButtonGroup::Class())) {
      ((TGButtonGroup*)fParent)->SetRadioButtonExclusive(kTRUE);
   }
   SetWindowName();
}

//______________________________________________________________________________
TGRadioButton::~TGRadioButton()
{
   // Delete a radio button.

   if (fOn)  fClient->FreePicture(fOn);
   if (fOff) fClient->FreePicture(fOff);
   if (fDisOn)  fClient->FreePicture(fDisOn);
   if (fDisOff) fClient->FreePicture(fDisOff);
}

//______________________________________________________________________________
TGDimension TGRadioButton::GetDefaultSize() const
{
   // default size

   UInt_t w = !fTWidth ? fOff->GetWidth() : fTWidth + fOff->GetWidth() + 10;
   UInt_t h = !fTHeight ? fOff->GetHeight() : fTHeight + 2;

   w = GetOptions() & kFixedWidth ? fWidth : w;
   h = GetOptions() & kFixedHeight ? fHeight : h;
                      
   return TGDimension(w, h);           
}
//______________________________________________________________________________
void TGRadioButton::SetState(EButtonState state, Bool_t emit)
{
   // Set radio button state.

   if (state == kButtonDisabled)
      fWidgetFlags &= ~kWidgetIsEnabled;
   else 
      fWidgetFlags |= kWidgetIsEnabled;
   PSetState(state, emit);
}

//______________________________________________________________________________
void TGRadioButton::SetDisabledAndSelected(Bool_t enable) 
{
   // Set the state of a radio button to disabled and either on or
   // off.

   if (!enable) {
      if (fState == kButtonDisabled && fStateOn) {
         PSetState(kButtonUp, kFALSE);         // enable button
         PSetState(kButtonUp, kFALSE);         // set button up
         PSetState(kButtonDisabled, kFALSE);   // disable button
      } else {
         PSetState(kButtonUp, kFALSE);
         PSetState(kButtonDisabled, kFALSE);
      }
   } else {
      PSetState(kButtonDown, kFALSE);          // set button down
      PSetState(kButtonDisabled, kFALSE);      // disable button
   }
}

//______________________________________________________________________________
void TGRadioButton::EmitSignals(Bool_t /*wasUp*/)
{
   // Emit signals.

   if (fState == kButtonUp) Released();              // emit Released
   if (fState == kButtonDown) Pressed();             // emit Pressed
   Clicked();                                        // emit Clicked
   Toggled(fStateOn);                                // emit Toggled
}

//______________________________________________________________________________
void TGRadioButton::PSetState(EButtonState state, Bool_t emit)
{
   // Set radio button state.

   if (state != fState) {
      //      fPrevState = fState = state;
      if (state == kButtonUp) {
         if (fPrevState == kButtonDisabled) {
            if (fStateOn) {
               fState = kButtonDown;
               fPrevState = kButtonDown;
            } else {
               fState = state;
               fPrevState = state;
            }
         } else if (fPrevState == kButtonDown) {
            fStateOn = kFALSE;
            fState = state;
            fPrevState = state;
         }
      } else if (state == kButtonDown) {
         fStateOn = kTRUE;
         fState = state;
         fPrevState = state;
      } else {
         fState = state;
         fPrevState = state;
      }
      if (emit) {
         // button signals
         EmitSignals();
      }
      DoRedraw();
   }
}

//______________________________________________________________________________
Bool_t TGRadioButton::HandleButton(Event_t *event)
{
   // Handle mouse button event.

   Bool_t click = kFALSE;
   Bool_t toggled = kFALSE;

   if (fTip) fTip->Hide();

   if (fState == kButtonDisabled) return kFALSE;


   Bool_t in = (event->fX >= 0) && (event->fY >= 0) &&
               (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);

   if (event->fType == kButtonPress) { // button pressed
      fgReleaseBtn = 0;
      if (in) {
         fOptions |= kSunkenFrame;
         Pressed();
      }
   } else { // ButtonRelease
      if (in) {
         if (!fStateOn) {
            PSetState(kButtonDown, kFALSE);
            toggled = kTRUE;
         }
         fPrevStateOn = fStateOn;
         Released();
         click = kTRUE;
      }
      fOptions &= ~kSunkenFrame;
      fgReleaseBtn = fId;
   }
   if (click) {
      Clicked();
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_RADIOBUTTON),
                  fWidgetId, (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_RADIOBUTTON),
                           fWidgetId, (Long_t) fUserData);
   }
   if (toggled) {
      Toggled(fStateOn);
   }
   DoRedraw();
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGRadioButton::HandleCrossing(Event_t *event)
{
   // Handle mouse crossing event.

   if (fTip) {
      if (event->fType == kEnterNotify)
         fTip->Reset();
      else
         fTip->Hide();
   }

   if ((fgDbw != event->fWindow) || (fgReleaseBtn == event->fWindow)) return kTRUE;

   if (!(event->fState & (kButton1Mask | kButton2Mask | kButton3Mask)))
      return kTRUE;

   if (fState == kButtonDisabled) return kTRUE;

   if (event->fType == kEnterNotify) {
      fOptions |= kSunkenFrame;
   } else {
      fOptions &= ~kSunkenFrame;
   }
   DoRedraw();

   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGRadioButton::HandleKey(Event_t *event)
{
   // Handle key event. This function will be called when the hotkey is hit.

   if (event->fType == kGKeyPress)
      gVirtualX->SetKeyAutoRepeat(kFALSE);
   else
      gVirtualX->SetKeyAutoRepeat(kTRUE);

   if (fTip && event->fType == kGKeyPress)
      fTip->Hide();

   if (fState == kButtonDisabled) return kTRUE;

   // We don't need to check the key number as GrabKey will only
   // allow fHotchar events if Alt button is pressed (kKeyMod1Mask)

   if ((event->fType == kGKeyPress) && (event->fState & kKeyMod1Mask)) {
      PSetState(kButtonDown, kTRUE);
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_RADIOBUTTON),
                  fWidgetId, (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_RADIOBUTTON),
                           fWidgetId, (Long_t) fUserData);
   } else if ((event->fType == kKeyRelease) && (event->fState & kKeyMod1Mask)) {
      fPrevState = fState;
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGRadioButton::DoRedraw()
{
   // Draw a radio button.

   Int_t tx, ty, y0;

   TGFrame::DoRedraw();

   tx = 20;
   ty = (fHeight - fTHeight) >> 1;

//   pw = 12;
   y0 = !fTHeight ? 0 : ty + 1;

   if (fStateOn) {
      if (fOn) fOn->Draw(fId, fNormGC, 0, y0);
   } else {
      if (fOff) fOff->Draw(fId, fNormGC, 0, y0);
   }

   Int_t hotpos = fLabel->GetHotPos();

   if (fState == kButtonDisabled) {
      if (fStateOn == kTRUE) {
         if (fDisOn) fDisOn->Draw(fId, fNormGC, 0, y0);
      } else {
         if (fDisOff) fDisOff->Draw(fId, fNormGC, 0, y0);
      }

      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);
      Pixel_t fore = gc->GetForeground();
      Pixel_t hi = GetHilightGC().GetForeground();
      Pixel_t sh = GetShadowGC().GetForeground();

      gc->SetForeground(hi);
      fTLayout->DrawText(fId, gc->GetGC(), tx + 1, ty + 1, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), tx, ty, hotpos - 1);

      gc->SetForeground(sh);
      fTLayout->DrawText(fId, gc->GetGC(), tx, ty, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), tx, ty, hotpos - 1);

      gc->SetForeground(fore);
   } else {
      fTLayout->DrawText(fId, fNormGC, tx, ty, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, fNormGC, tx, ty, hotpos-1);
   }
}

//______________________________________________________________________________
FontStruct_t TGRadioButton::GetDefaultFontStruct()
{
   // Return default font structure.

   if (!fgDefaultFont)
      fgDefaultFont = gClient->GetResourcePool()->GetDefaultFont();
   return fgDefaultFont->GetFontStruct();
}

//______________________________________________________________________________
const TGGC &TGRadioButton::GetDefaultGC()
{
   // Return default graphics context.

   if (!fgDefaultGC)
      fgDefaultGC = gClient->GetResourcePool()->GetFrameGC();
   return *fgDefaultGC;
}

//______________________________________________________________________________
void TGButton::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a button widget as a C++ statement(s) on output stream out.

   char quote = '"';

   if (fState == kButtonDown) {
      out << "   " << GetName() << "->SetState(kButtonDown);"  << endl;
   }
   if (fState == kButtonDisabled) {
      out << "   " << GetName() << "->SetState(kButtonDisabled);"  << endl;
   }
   if (fState == kButtonEngaged) {
      out << "   " << GetName() << "->SetState(kButtonEngaged);"  << endl;
   }
   if (fBackground != fgDefaultFrameBackground) {
      SaveUserColor(out, option);
      out << "   " << GetName() << "->ChangeBackground(ucolor);" << endl;
   }

   if (fTip) {
      TString tiptext = fTip->GetText()->GetString();
      tiptext.ReplaceAll("\n", "\\n");
      out << "   ";
      out << GetName() << "->SetToolTipText(" << quote
          << tiptext << quote << ");"  << endl;
   }
   if (strlen(fCommand)) {
      out << "   " << GetName() << "->SetCommand(" << quote << fCommand
          << quote << ");" << endl;
   }
}

//______________________________________________________________________________
void TGTextButton::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a text button widget as a C++ statement(s) on output stream out.

   char quote = '"';
   const char *text = fLabel->GetString();
   Int_t hotpos = fLabel->GetHotPos();
   Int_t lentext = fLabel->GetLength();
   char *outext = new char[lentext+2];
   Int_t i=0;
   while (lentext) {
      if (hotpos && (i == hotpos-1)) {
         outext[i] = '&';
         i++;
      }
      outext[i] = *text;
      i++;
      text++;
      lentext--;
   }
   outext[i]=0;

   // font + GC
   option = GetName()+5;         // unique digit id of the name
   char parGC[50], parFont[50];
   sprintf(parFont,"%s::GetDefaultFontStruct()",IsA()->GetName());
   sprintf(parGC,"%s::GetDefaultGC()()",IsA()->GetName());

   if ((GetDefaultFontStruct() != fFontStruct) || (GetDefaultGC()() != fNormGC)) {
      TGFont *ufont = gClient->GetResourcePool()->GetFontPool()->FindFont(fFontStruct);
      if (ufont) {
         ufont->SavePrimitive(out, option);
         sprintf(parFont,"ufont->GetFontStruct()");
      }

      TGGC *userGC = gClient->GetResourcePool()->GetGCPool()->FindGC(fNormGC);
      if (userGC) {
         userGC->SavePrimitive(out, option);
         sprintf(parGC,"uGC->GetGC()");
      }
   }

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out << "   TGTextButton *";
   out << GetName() << " = new TGTextButton(" << fParent->GetName()
       << "," << quote << outext << quote;

   if (GetOptions() == (kRaisedFrame | kDoubleBorder)) {
      if (fFontStruct == GetDefaultFontStruct()) {
         if (fNormGC == GetDefaultGC()()) {
            if (fWidgetId == -1) {
               out << ");" << endl;
            } else {
               out << "," << fWidgetId <<");" << endl;
            }
         } else {
            out << "," << fWidgetId << "," << parGC << ");" << endl;
         }
      } else {
         out << "," << fWidgetId << "," << parGC << "," << parFont << ");" << endl;
      }
   } else {
      out << "," << fWidgetId << "," << parGC << "," << parFont << "," << GetOptionString() << ");" << endl;
   }

   delete [] outext;

   out << "   " << GetName() << "->SetTextJustify(" << fTMode << ");" << endl;
   out << "   " << GetName() << "->SetMargins(" << fMLeft << "," << fMRight << ",";
   out << fMTop << "," << fMBottom << ");" << endl;
   out << "   " << GetName() << "->SetWrapLength(" << fWrapLength << ");" << endl;

   out << "   " << GetName() << "->Resize(" << GetWidth() << "," << GetHeight()
       << ");" << endl;

   TGButton::SavePrimitive(out,option);
}

//______________________________________________________________________________
void TGPictureButton::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a picture button widget as a C++ statement(s) on output stream out.

   if (!fPic) {
      Error("SavePrimitive()", "pixmap not found for picture button %d ", fWidgetId);
      return;
   }

   // GC
   option = GetName()+5;         // unique digit id of the name
   char parGC[50];
   sprintf(parGC,"%s::GetDefaultGC()()",IsA()->GetName());

   if (GetDefaultGC()() != fNormGC) {
      TGGC *userGC = gClient->GetResourcePool()->GetGCPool()->FindGC(fNormGC);
      if (userGC) {
         userGC->SavePrimitive(out, option);
         sprintf(parGC,"uGC->GetGC()");
      }
   }

   char quote = '"';
   const char *picname = fPic->GetName();

   out <<"   TGPictureButton *";

   out << GetName() << " = new TGPictureButton(" << fParent->GetName()
       << ",gClient->GetPicture(" << quote
       << gSystem->ExpandPathName(gSystem->UnixPathName(picname)) << quote << ")";

   if (GetOptions() == (kRaisedFrame | kDoubleBorder)) {
      if (fNormGC == GetDefaultGC()()) {
         if (fWidgetId == -1) {
            out << ");" << endl;
         } else {
            out << "," << fWidgetId << ");" << endl;
         }
      } else {
         out << "," << fWidgetId << "," << parGC << ");" << endl;
      }
   } else {
      out << "," << fWidgetId << "," << parGC << "," << GetOptionString()
          << ");" << endl;
   }

   TGButton::SavePrimitive(out,option);
}

//______________________________________________________________________________
void TGCheckButton::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a check button widget as a C++ statement(s) on output stream out.

   char quote = '"';

   const char *text = fLabel->GetString();
   char hotpos = fLabel->GetHotPos();
   Int_t lentext = fLabel->GetLength();
   char *outext = new char[lentext+2];       // should be +2 because of \0
   Int_t i=0;


   while (lentext) {
      if (hotpos && (i == hotpos-1)) {
         outext[i] = '&';
         i++;
      }
      outext[i] = *text;
      i++;
      text++;
      lentext--;
   }
   outext[i]=0;

   out <<"   TGCheckButton *";
   out << GetName() << " = new TGCheckButton(" << fParent->GetName()
       << "," << quote << outext << quote;

   delete [] outext;

   // font + GC
   option = GetName()+5;         // unique digit id of the name
   char parGC[50], parFont[50];
   sprintf(parFont,"%s::GetDefaultFontStruct()",IsA()->GetName());
   sprintf(parGC,"%s::GetDefaultGC()()",IsA()->GetName());

   if ((GetDefaultFontStruct() != fFontStruct) || (GetDefaultGC()() != fNormGC)) {
      TGFont *ufont = gClient->GetResourcePool()->GetFontPool()->FindFont(fFontStruct);
      if (ufont) {
         ufont->SavePrimitive(out, option);
         sprintf(parFont,"ufont->GetFontStruct()");
      }

      TGGC *userGC = gClient->GetResourcePool()->GetGCPool()->FindGC(fNormGC);
      if (userGC) {
         userGC->SavePrimitive(out, option);
         sprintf(parGC,"uGC->GetGC()");
      }
   }

   if (GetOptions() == kChildFrame) {
      if (fFontStruct == GetDefaultFontStruct()) {
         if (fNormGC == GetDefaultGC()()) {
            if (fWidgetId == -1) {
               out << ");" << endl;
            } else {
               out << "," << fWidgetId << ");" << endl;
            }
         } else {
            out << "," << fWidgetId << "," << parGC << ");" << endl;
         }
      } else {
         out << "," << fWidgetId << "," << parGC << "," << parFont << ");" << endl;
      }
   } else {
      out << "," << fWidgetId << "," << parGC << "," << parFont << "," << GetOptionString() << ");" << endl;
   }

   TGButton::SavePrimitive(out,option);
   if (fState == kButtonDisabled) {
      if (IsDisabledAndSelected())
         out << "   " << GetName() << "->SetDisabledAndSelected(kTRUE);" << endl;
      else
         out << "   " << GetName() << "->SetDisabledAndSelected(kFALSE);" << endl;
   }
   out << "   " << GetName() << "->SetTextJustify(" << fTMode << ");" << endl;
   out << "   " << GetName() << "->SetMargins(" << fMLeft << "," << fMRight << ",";
   out << fMTop << "," << fMBottom << ");" << endl;
   out << "   " << GetName() << "->SetWrapLength(" << fWrapLength << ");" << endl;
}

//______________________________________________________________________________
void TGRadioButton::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a radio button widget as a C++ statement(s) on output stream out.

   char quote = '"';

   const char *text = fLabel->GetString();
   char hotpos = fLabel->GetHotPos();
   Int_t lentext = fLabel->GetLength();
   char *outext = new char[lentext+2];
   Int_t i=0;

   while (lentext) {
      if (hotpos && (i == hotpos-1)) {
         outext[i] = '&';
         i++;
      }
      outext[i] = *text;
      i++; text++; lentext--;
   }
   outext[i]=0;
   out << "   TGRadioButton *";
   out << GetName() << " = new TGRadioButton(" << fParent->GetName()
       << "," << quote << outext << quote;

   delete [] outext;

   // font + GC
   option = GetName()+5;         // unique digit id of the name
   char parGC[50], parFont[50];
   sprintf(parFont,"%s::GetDefaultFontStruct()",IsA()->GetName());
   sprintf(parGC,"%s::GetDefaultGC()()",IsA()->GetName());

   if ((GetDefaultFontStruct() != fFontStruct) || (GetDefaultGC()() != fNormGC)) {
      TGFont *ufont = gClient->GetResourcePool()->GetFontPool()->FindFont(fFontStruct);
      if (ufont) {
         ufont->SavePrimitive(out, option);
         sprintf(parFont,"ufont->GetFontStruct()");
      }

      TGGC *userGC = gClient->GetResourcePool()->GetGCPool()->FindGC(fNormGC);
      if (userGC) {
         userGC->SavePrimitive(out, option);
         sprintf(parGC,"uGC->GetGC()");
      }
   }

   if (GetOptions() == (kChildFrame)) {
      if (fFontStruct == GetDefaultFontStruct()) {
         if (fNormGC == GetDefaultGC()()) {
            if (fWidgetId == -1) {
               out <<");" << endl;
            } else {
               out << "," << fWidgetId << ");" << endl;
            }
         } else {
            out << "," << fWidgetId << "," << parGC << ");" << endl;
         }
      } else {
         out << "," << fWidgetId << "," << parGC << "," << parFont << ");" << endl;
      }
   } else {
      out << "," << fWidgetId << "," << parGC << "," << parFont << "," << GetOptionString() << ");" << endl;
   }

   TGButton::SavePrimitive(out,option);
   if (fState == kButtonDisabled) {
      if (IsDisabledAndSelected())
         out << "   " << GetName() << "->SetDisabledAndSelected(kTRUE);" << endl;
      else
         out << "   " << GetName() << "->SetDisabledAndSelected(kFALSE);" << endl;
   }
   out << "   " << GetName() << "->SetTextJustify(" << fTMode << ");" << endl;
   out << "   " << GetName() << "->SetMargins(" << fMLeft << "," << fMRight << ",";
   out << fMTop << "," << fMBottom << ");" << endl;
   out << "   " << GetName() << "->SetWrapLength(" << fWrapLength << ");" << endl;
}

//______________________________________________________________________________
TGSplitButton::TGSplitButton(const TGWindow *p, TGHotString* menulabel, 
                           TGPopupMenu *popmenu, Bool_t split, Int_t id, 
                           GContext_t norm, FontStruct_t fontstruct, UInt_t options)
                           : TGTextButton(p, menulabel, id, norm, fontstruct, options)
{
   // Create a menu button widget. The hotstring will be adopted and
   // deleted by the menu button. This constructior creates a
   // menubutton with a popup menu attached that appears when the
   // button for it is clicked. The popup menu is adopted.

   fFontStruct = fontstruct;
   fMBWidth = 16;
   fMenuLabel = new TGHotString(*menulabel);
   fPopMenu = popmenu;
   fPopMenu->fSplitButton = this;
   fSplit = split;
   fTMode = 0;
   fHKeycode = 0;
   fMBState = kButtonUp; fDefaultCursor = fClient->GetResourcePool()->GetGrabCursor();
   fKeyNavigate = kFALSE;
   fWidestLabel = "";
   fHeighestLabel = "";

   // Find and set the correct size for the menu and the button.
   TGMenuEntry *entry = 0;
   TGHotString lstring(*fMenuLabel);
   TGHotString hstring(*fMenuLabel);
   const TList *list = fPopMenu->GetListOfEntries();
   UInt_t lwidth = 0, lheight = 0;
   UInt_t twidth = 0, theight = 0;
   
   TGFont *font = fClient->GetFontPool()->FindFont(fFontStruct);
   if (!font) {
      font = fClient->GetFontPool()->GetFont(fgDefaultFont);
      fFontStruct = font->GetFontStruct();
   }

   font->ComputeTextLayout(lstring, lstring.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &lwidth, &lheight);

   TIter iter(list);
   entry = (TGMenuEntry *)iter.Next();
   while (entry != 0) {
      if (entry->GetType() == kMenuEntry) {
         const TGHotString string(*(entry->GetLabel()));
         font->ComputeTextLayout(string, string.GetLength(),
                                 fWrapLength, kTextLeft, 0,
                                 &twidth, &theight);
         if(twidth > lwidth) {
            lstring = string;
         }
         if(theight > lheight) {
            hstring = string;
         }
      }
      entry = (TGMenuEntry *)iter.Next();
   }
   fWidestLabel = lstring;
   fHeighestLabel =  hstring;

   UInt_t dummy = 0;
   font->ComputeTextLayout(lstring, lstring.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &fTWidth, &dummy);
   font->ComputeTextLayout(hstring, hstring.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &dummy, &fTHeight);

   fTBWidth = fTWidth + 8;
   fHeight = fTHeight + 7;      
   Resize(fTBWidth + fMBWidth, fHeight);

   ChangeOptions(GetOptions() | kFixedSize);

   // Save the id of the 1st item on the menu.
   TIter iter1(list);
   do {
      entry = (TGMenuEntry *)iter1.Next();
      if ((entry->GetStatus() & kMenuEnableMask) &&
          !(entry->GetStatus() & kMenuHideMask) &&
          (entry->GetType() != kMenuSeparator) &&
          (entry->GetType() != kMenuLabel)) break;
      entry = (TGMenuEntry *)iter1.Next();
   } while (entry);
   if (entry) fEntryId = entry->GetEntryId();

   // An additional connection that is needed.
   fPopMenu->Connect("Activated(Int_t)", "TGSplitButton", this, "HandleMenu(Int_t)");
   SetSplit(fSplit);

   Init();
}


//______________________________________________________________________________
void TGSplitButton::Init()
{
   // Common initialization used by the different ctors.

   Int_t hotchar;

   fTMode       = kTextCenterX | kTextCenterY;
   fHKeycode    = 0;
   fHasOwnFont  = kFALSE;
   fPrevStateOn =
   fStateOn     = kFALSE;
   fMBState     = kButtonUp;

   SetSize(TGDimension(fWidth, fHeight));

   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0) {
         const TGMainFrame *main = (TGMainFrame *) GetMainFrame();
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
   }
   SetWindowAttributes_t wattr;
   wattr.fMask = kWAWinGravity | kWABitGravity;
   wattr.fBitGravity = 5; // center
   wattr.fWinGravity = 1;
   gVirtualX->ChangeWindowAttributes(fId, &wattr);

   // Make sure motion is detected too.
   AddInput(kPointerMotionMask | kEnterWindowMask | kLeaveWindowMask);

   SetWindowName();
}

//______________________________________________________________________________
TGSplitButton::~TGSplitButton()
{
   // Delete a split button widget.

   if (fPopMenu) delete fPopMenu;
   if (fMenuLabel) delete fMenuLabel;
}

//________________________________________________________________________________
void TGSplitButton::DrawTriangle(const GContext_t gc, Int_t x, Int_t y)
{
   // Draw triangle (arrow) on which user can click to open Popup.

   Point_t points[3];

   points[0].fX = x;
   points[0].fY = y;
   points[1].fX = x + 5;
   points[1].fY = y;
   points[2].fX = x + 2;
   points[2].fY = y + 3;

   gVirtualX->FillPolygon(fId, gc, points, 3);
}

//______________________________________________________________________________
void TGSplitButton::CalcSize() 
{
   // Calculate the size of the button.

   Int_t max_ascent, max_descent;
   fTWidth = gVirtualX->TextWidth(fFontStruct, fLabel->GetString(), fLabel->GetLength());
   gVirtualX->GetFontProperties(fFontStruct, max_ascent, max_descent);
   fTHeight = max_ascent + max_descent;
   
   fTBWidth = fTWidth + 8;
   fHeight = fTHeight + 7;      
   fWidth = fTBWidth;
}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleSButton(Event_t *event)
{                    
   // Handle mouse button event in case the button is split.

   if (fState == kButtonDisabled) return kFALSE;

   Bool_t activate = kFALSE;
   Bool_t bclick = kFALSE;
   static Bool_t mbpress = kFALSE;
   static Bool_t tbpress = kFALSE;
   static Bool_t outpress = kFALSE;

   Bool_t inTB = (event->fX >= 0) && (event->fY >= 0) &&
                 (event->fX <= (Int_t)fTBWidth) && (event->fY <= (Int_t)fHeight);

   Bool_t inMB = (event->fX >= (Int_t)(fWidth -fMBWidth)) && (event->fY >= 0) &&
      (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);
   
   // We don't need to check the button number as GrabButton will
   // only allow button1 events

   if (inTB) {
      if (event->fType == kButtonPress) {
         mbpress = kFALSE;
         tbpress = kTRUE;
         fgReleaseBtn = 0;
         if (fState == kButtonEngaged) {
            return kTRUE;
         }
         SetState(kButtonDown);
         Pressed();
      } else { // ButtonRelease
         if (fMBState == kButtonDown) {
            SetMBState(kButtonUp);
         }
         if (fState == kButtonEngaged && tbpress) {
            SetState(kButtonUp);
            Released();
            bclick = kTRUE;
         } else if (fState == kButtonDown && tbpress) {
            tbpress = kFALSE;
            if (fStayDown) {
               bclick = kTRUE;
               SetState(kButtonEngaged);
               fgReleaseBtn = 0;
            } else {
               bclick = kTRUE;
               SetState(kButtonUp);
               Released();
               fgReleaseBtn = fId;               
            } 
         }else {
            SetState(kButtonUp);
         }
      }
   } else if (inMB) {
      if (event->fType == kButtonPress) {
         fgReleaseBtn = 0;
         mbpress = kTRUE;
         tbpress = kFALSE;
         if (fMBState == kButtonEngaged) {
            return kTRUE;
         }
         SetMBState(kButtonDown);
         MBPressed();
         gVirtualX->GrabPointer(fId, kButtonPressMask | kButtonReleaseMask |
                                kPointerMotionMask, kNone, fDefaultCursor);
      } else { // ButtonRelease
         if (fState == kButtonDown) {
            SetState(kButtonUp);
         }                
         if (fMBState == kButtonEngaged && mbpress) {
            mbpress = kFALSE;
            SetMBState(kButtonUp);
            SetMenuState(kFALSE);
            MBReleased();
            MBClicked();
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
         } else if (fMBState == kButtonDown && mbpress) {
            MBClicked();
            SetMBState(kButtonEngaged);
            SetMenuState(kTRUE);            
            fgReleaseBtn = 0;
         } else {
            SetMBState(kButtonUp);
         }
      }
   } else {
      if (event->fType == kButtonPress) {
         fgReleaseBtn = 0;
         outpress = kTRUE;
      } else { // ButtonRelease
         if(outpress) {
            outpress = kFALSE;
            SetMBState(kButtonUp);
            SetMenuState(kFALSE);
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
            activate = kTRUE;
         }
      }      
   }
   if (bclick) {
      Clicked();
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }
   if (activate) {
      TGMenuEntry *entry =  fPopMenu->GetCurrent();
      if (entry) {
         if ((entry->GetStatus() & kMenuEnableMask) &&
             !(entry->GetStatus() & kMenuHideMask) &&
             (entry->GetType() != kMenuSeparator) &&
             (entry->GetType() != kMenuLabel)) {
            Int_t id = entry->GetEntryId();
            fPopMenu->Activated(id);
         }
      }
   }
   //   if (mbclick) {
   //      MBClicked();
   //      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
   //             (Long_t) fUserData);
   //      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
   //                      (Long_t) fUserData);
   // }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleSCrossing(Event_t *event)
{
   // Handle mouse crossing event in case of split menu.

   if (fTip) {
      if (event->fType == kEnterNotify)
         fTip->Reset();
      else
         fTip->Hide();
   }

   if ((fgDbw != event->fWindow) || (fgReleaseBtn == event->fWindow)) return kTRUE;

   if (!(event->fState & (kButton1Mask | kButton2Mask | kButton3Mask)))
      return kTRUE;

   if (fState == kButtonEngaged || fState == kButtonDisabled) return kTRUE;

   Bool_t inTB = (event->fX <= (Int_t)fTBWidth);

   //   Bool_t inMB = (event->fX >= (Int_t)(fWidth -fMBWidth)) && (event->fY >= 0) &&
   //      (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);

   if (event->fType == kEnterNotify) {
      if (inTB) {
         SetState(kButtonDown, kFALSE);
      } else {
         if(fMBState == kButtonEngaged)  return kTRUE;
         SetMBState(kButtonDown);
      }
   } else {
      // kLeaveNotify
      if(fState == kButtonDown) {
         SetState(kButtonUp, kFALSE);
      }
      if (fMBState == kButtonEngaged) return kTRUE;
      SetMBState(kButtonUp);
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleSKey(Event_t *event)
{
   // Handle key event. This function will be called when the hotkey is hit.

   if (fState == kButtonDisabled) return kFALSE;

   Bool_t click = kFALSE;

   if (event->fType == kGKeyPress) {
      gVirtualX->SetKeyAutoRepeat(kFALSE);
   } else {
      gVirtualX->SetKeyAutoRepeat(kTRUE);
   }

   if (fTip && event->fType == kGKeyPress) fTip->Hide();

   // We don't need to check the key number as GrabKey will only
   // allow fHotchar events if Alt button is pressed (kKeyMod1Mask)
   
   if ((event->fType == kGKeyPress) && (event->fState & kKeyMod1Mask)) {
      if (fState == kButtonEngaged) return kTRUE;
      SetState(kButtonDown);
      Pressed();
   } else if ((event->fType == kKeyRelease) && (event->fState & kKeyMod1Mask)) {
      if (fState == kButtonEngaged) {
         SetState(kButtonUp);
         Released();
      }
      if (fStayDown) {
         SetState(kButtonEngaged);
      } else {
         SetState(kButtonUp);         
         Released();
      }
      click = kTRUE;
   }
   if (click) {
      Clicked();
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }

   return kTRUE;
}

//______________________________________________________________________________
void TGSplitButton::SetMenuState(Bool_t state) 
{
   // Popup the attached menu. 

   if (state) {
      Int_t    ax, ay;
      Window_t wdummy;

      if (fSplit) {
         Int_t n_entries = 0;
         TGMenuEntry *entry = 0;
         TIter next(fPopMenu->GetListOfEntries());
         
         while ((entry = (TGMenuEntry *) next())) {
            if ((entry->GetType() != kMenuSeparator) &&
                (entry->GetType() != kMenuLabel)) {
               n_entries++;
            }
         }
         if (n_entries <= 1) {
            Info("TGSplitButton", "Only one entry in the menu.");
            return;
         }
      }

      gVirtualX->TranslateCoordinates(fId, fPopMenu->GetParent()->GetId(),
                                      0, 0, ax, ay, wdummy);

      // place the menu just under the window:
      fPopMenu->PlaceMenu(ax-1, ay+fHeight, kTRUE, kFALSE); //kTRUE);
      BindKeys(kTRUE);
      BindMenuKeys(kTRUE);
   } else {
      fPopMenu->EndMenu(fUserData);
      BindKeys(kFALSE);
      BindMenuKeys(kFALSE);
      fPopMenu->EndMenu(fUserData);
   }
}

//______________________________________________________________________________
void TGSplitButton::DoRedraw()
{
   // Draw the text button.

   int x, y;
   TGFrame::DoRedraw();
   
   if (fState == kButtonDisabled) fMBState = kButtonDisabled;
   else if (fMBState == kButtonDisabled) fMBState = kButtonUp;

   if (fTMode & kTextLeft) {
      x = fMLeft + 4;
   } else if (fTMode & kTextRight) {
      x = fWidth - fTWidth -fMBWidth - fMRight - 4;
   } else {
      x = (fWidth - fTWidth -fMBWidth + fMLeft - fMRight) >> 1;
   }

   if (fTMode & kTextTop) {
      y = fMTop + 3;
   } else if (fTMode & kTextBottom) {
      y = fHeight - fTHeight - fMBottom - 3;
   } else {
      y = (fHeight - fTHeight + fMTop - fMBottom) >> 1;
   }

   if (fState == kButtonDown || fState == kButtonEngaged) { ++x; ++y; }
   if (fState == kButtonEngaged) {
      gVirtualX->FillRectangle(fId, GetHibckgndGC()(), 2, 2, fWidth-4, fHeight-4);
      gVirtualX->DrawLine(fId, GetHilightGC()(), 2, 2, fWidth-3, 2);
   }

   Int_t hotpos = fLabel->GetHotPos();

   if (fState == kButtonDisabled) {
      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);
      Pixel_t fore = gc->GetForeground();
      Pixel_t hi = GetHilightGC().GetForeground();
      Pixel_t sh = GetShadowGC().GetForeground();

      gc->SetForeground(hi);
      fTLayout->DrawText(fId, gc->GetGC(), x + 1, y + 1, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x + 1, y + 1, hotpos - 1);

      gc->SetForeground(sh);
      fTLayout->DrawText(fId, gc->GetGC(), x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, gc->GetGC(), x, y, hotpos - 1);
      gc->SetForeground(fore);
   } else {
      fTLayout->DrawText(fId, fNormGC, x, y, 0, -1);
      if (hotpos) fTLayout->UnderlineChar(fId, fNormGC, x, y, hotpos - 1);
   }

   // Draw the parts of the button needed when a menu is attached.
   
   // triangle position
   x = fWidth - 11;
   y = fHeight - 10;
   
   if (fSplit) {
      // separator position
      Int_t lx = fWidth - fMBWidth;
      Int_t ly = 2;
      Int_t lh = fHeight - 2;
      
      if(fMBState == kButtonDown || fMBState == kButtonEngaged) {
         x++;
         y++;
      }

      gVirtualX->DrawLine(fId, GetShadowGC()(),  lx, ly + 2, lx, lh - 4);
      gVirtualX->DrawLine(fId, GetHilightGC()(), lx + 1, ly + 2, lx + 1, lh - 3);
      gVirtualX->DrawLine(fId, GetHilightGC()(), lx, lh - 3, lx + 1, lh - 3);

      if (fMBState == kButtonEngaged) {
         gVirtualX->FillRectangle(fId, GetHibckgndGC()(), fTBWidth + 1, 1, fMBWidth - 3, fHeight - 3);
      }

      if (fMBState == kButtonDisabled) {
         DrawTriangle(GetHilightGC()(), x + 1, y + 1);
         DrawTriangle(GetShadowGC()(), x, y);
      } else {
         DrawTriangle(fNormGC, x, y);
      }

   } else {
      x -= 2;
      if(fState == kButtonDown || fState == kButtonEngaged) {
         x++;
         y++;
      }
      if (fState == kButtonDisabled) {
         DrawTriangle(GetHilightGC()(), x + 1, y + 1);
         DrawTriangle(GetShadowGC()(), x, y);
      } else {
         DrawTriangle(fNormGC, x, y);
      }
   }
   
}

//______________________________________________________________________________
void TGSplitButton::BindKeys(Bool_t on)
{
   // If on kTRUE bind arrow, popup menu hot keys, otherwise
   // remove key bindings.

   gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(kKey_Up), kAnyModifier, on);
   gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(kKey_Down), kAnyModifier, on);
   gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(kKey_Enter), kAnyModifier, on);
   gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(kKey_Return), kAnyModifier, on);
   gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(kKey_Escape), kAnyModifier, on);
}

//______________________________________________________________________________
void TGSplitButton::BindMenuKeys(Bool_t on)
{
   // If on kTRUE bind Menu hot keys, otherwise remove key bindings.

   TGMenuEntry *e = 0;
   TIter next(fPopMenu->GetListOfEntries());
   
   while ((e = (TGMenuEntry*)next())) {
      Int_t hot = 0;
      if (e->GetLabel()) {
         hot = e->GetLabel()->GetHotChar();
      }
      if (!hot) continue;
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), 0, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyShiftMask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyLockMask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyMod2Mask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyShiftMask | kKeyLockMask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyShiftMask | kKeyMod2Mask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyLockMask  | kKeyMod2Mask, on);
      gVirtualX->GrabKey(fId, gVirtualX->KeysymToKeycode(hot), kKeyShiftMask | kKeyLockMask | kKeyMod2Mask, on);
   }
}

//______________________________________________________________________________
TGDimension TGSplitButton::GetDefaultSize() const
{
   // returns default size

   UInt_t w = GetOptions() & kFixedWidth ? fWidth + fMBWidth : fTWidth + fMLeft + fMRight + fMBWidth + 8;
   UInt_t h = GetOptions() & kFixedHeight ? fHeight : fTHeight + fMTop + fMBottom + 7;
   return TGDimension(w, h);
}

//______________________________________________________________________________
void TGSplitButton::SetText(TGHotString *new_label)
{
   // Set new button text.

   Int_t hotchar;
   static Bool_t longlabeltip = kFALSE;
   const TGMainFrame *main = (TGMainFrame *) GetMainFrame();   

   TGFont *font = fClient->GetFontPool()->FindFont(fFontStruct);
   if (!font) {
      font = fClient->GetFontPool()->GetFont(fgDefaultFont);
      fFontStruct = font->GetFontStruct();
   }

   UInt_t width = 0, bwidth = 0, dummy;
   font->ComputeTextLayout(new_label->GetString(), new_label->GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &width, &dummy);
   font->ComputeTextLayout(fWidestLabel.GetString(), fWidestLabel.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &bwidth, &dummy);

   if (width > bwidth) {
      if (!fTip) {
         SetToolTipText(new_label->GetString());
         longlabeltip = kTRUE;
      }
      Info("TGSplitbutton", "Length of new label to long, label truncated.");
      new_label->Resize(fWidestLabel.GetLength());
   } else if (new_label->GetLength() <= fWidestLabel.GetLength() && longlabeltip) {
      if (fTip) delete fTip;
      fTip = 0;
      longlabeltip = kFALSE;
   }
         
   if (fLabel) {
      if (fHKeycode) {
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->RemoveBind(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
      }
      delete fLabel;
   }

   fLabel = new_label;
   if ((hotchar = fLabel->GetHotChar()) != 0) {
      if ((fHKeycode = gVirtualX->KeysymToKeycode(hotchar)) != 0)
         main->BindKey(this, fHKeycode, kKeyMod1Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyLockMask);

         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyMod2Mask | kKeyLockMask);
         main->BindKey(this, fHKeycode, kKeyMod1Mask | kKeyShiftMask | kKeyMod2Mask | kKeyLockMask);
   }

   Layout();
}

//______________________________________________________________________________
void TGSplitButton::SetText(const TString &new_label)
{
   // Set new button text.

   SetText(new TGHotString(new_label));
}

//______________________________________________________________________________
void TGSplitButton::SetFont(FontStruct_t font, Bool_t global)
{
   // Changes text font.
   // If global is kTRUE font is changed globally, otherwise - locally.

   if (font != fFontStruct) {
      FontH_t v = gVirtualX->GetFontHandle(font);
      if (!v) return;

      fFontStruct = font;
      TGGCPool *pool =  fClient->GetResourcePool()->GetGCPool();
      TGGC *gc = pool->FindGC(fNormGC);

      if (!global) {
         gc = pool->GetGC((GCValues_t*)gc->GetAttributes(), kTRUE); // copy
         fHasOwnFont = kTRUE;
      }

      gc->SetFont(v);

      fNormGC = gc->GetGC();

      fClient->NeedRedraw(this);
   }
}

//______________________________________________________________________________
void TGSplitButton::SetFont(const char *fontName, Bool_t global)
{
   // Changes text font specified by name.
   // If global is true color is changed globally, otherwise - locally.

   TGFont *font = fClient->GetFont(fontName);
   if (font) {
      SetFont(font->GetFontStruct(), global);
   }
}

//______________________________________________________________________________
void TGSplitButton::SetMBState(EButtonState state)
{
   // Set the state of the Menu Button part
   
   if (state != fMBState) {
      fMBState = state;
      DoRedraw();
   }
}

//______________________________________________________________________________
void TGSplitButton::SetSplit(Bool_t split)
{
   // Set the split status of a button.
   
   if(split) {
      fStayDown = kFALSE;
      Disconnect(fPopMenu, "PoppedDown()");
      fPopMenu->Connect("PoppedDown()", "TGSplitButton", this, "SetMBState(=kButtonUp)");
      fPopMenu->Connect("PoppedDown()", "TGSplitButton", this, "MBReleased()");

      TGMenuEntry *entry = fPopMenu->GetEntry(fEntryId);

      TGHotString *tmp = new TGHotString(*(entry->GetLabel()));
      SetText(tmp);

      TString str("ItemClicked(=");
      str += entry->GetEntryId();
      str += ")";
      Connect("Clicked()", "TGSplitButton", this, str);
      fEntryId = entry->GetEntryId();
      fPopMenu->HideEntry(fEntryId);
   } else {
      fStayDown = kTRUE;
      Disconnect(fPopMenu, "PoppedDown()");
      Disconnect(this, "Clicked()", this);
      fPopMenu->Connect("PoppedDown()", "TGSplitButton", this, "SetState(=kButtonUp)");
      fPopMenu->Connect("PoppedDown()", "TGSplitButton", this, "Released()");
      fPopMenu->EnableEntry(fEntryId);
      TGHotString *tmp = new TGHotString(*fMenuLabel);
      SetText(tmp);
   }

   fSplit = split;
   DoRedraw();
}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleButton(Event_t *event)
{                    
   // Handle button events.

   if (fState == kButtonDisabled) return kFALSE;

   if (fSplit) return HandleSButton(event);

   Bool_t in = (event->fX >= 0) && (event->fY >= 0) &&
               (event->fX <= (Int_t)fWidth) && (event->fY <= (Int_t)fHeight);

   Bool_t activate = kFALSE;
   Bool_t click = kFALSE;

   if (in) {
      if (event->fType == kButtonPress) {
         fgReleaseBtn = 0;
         if (fState == kButtonEngaged) {
            return kTRUE;
         }
         SetState(kButtonDown);
         Pressed();
         gVirtualX->GrabPointer(fId, kButtonPressMask | kButtonReleaseMask |
                                kPointerMotionMask, kNone, fDefaultCursor);
      } else { // ButtonRelease
         if (fState == kButtonEngaged) {
            SetState(kButtonUp);
            SetMenuState(kFALSE);
            Released();
            click = kTRUE;
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
         } else {
            click = (fState == kButtonDown);
            if (click && fStayDown) {
               SetState(kButtonEngaged);
               SetMenuState(kTRUE);
               fgReleaseBtn = 0;
            } else {
               SetState(kButtonUp);
               Released();
               fgReleaseBtn = fId;
            }
         }
         fKeyNavigate = kFALSE;
      }
   } else {
      if (event->fType == kButtonPress) {
         fgReleaseBtn = 0;
      } else { // ButtonRelease
         SetState(kButtonUp);
         SetMenuState(kFALSE);
         gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);  // ungrab pointer
         activate = kTRUE;
      }      
   }
   if (click) {
      Clicked();
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }
   if (activate && !fKeyNavigate) {
      TGMenuEntry *entry =  fPopMenu->GetCurrent();
      if (entry) {
         if ((entry->GetStatus() & kMenuEnableMask) &&
             !(entry->GetStatus() & kMenuHideMask) &&
             (entry->GetType() != kMenuSeparator) &&
             (entry->GetType() != kMenuLabel)) {
            Int_t id = entry->GetEntryId();
            fPopMenu->Activated(id);
         }
      }
   }
     
   return kTRUE;

}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleCrossing(Event_t *event)
{
   // Handle mouse crossing event.

   if (fSplit) { 
      return HandleSCrossing(event);
   } else {
      return TGButton::HandleCrossing(event);
   }
   fKeyNavigate = kFALSE;

}
   
//______________________________________________________________________________
Bool_t TGSplitButton::HandleKey(Event_t *event)
{
   // Handle key event. This function will be called when the hotkey is hit.

   Bool_t click = kFALSE;

   if (fState == kButtonDisabled) return kTRUE;

   if(fSplit) return HandleSKey(event);

   if (event->fType == kGKeyPress) {
      gVirtualX->SetKeyAutoRepeat(kFALSE);
   } else {
      gVirtualX->SetKeyAutoRepeat(kTRUE);
   }

   if (fTip && event->fType == kGKeyPress) fTip->Hide();

   // We don't need to check the key number as GrabKey will only
   // allow fHotchar events if Alt button is pressed (kKeyMod1Mask)
   if (event->fState & kKeyMod1Mask) {
      RequestFocus();
      fKeyNavigate = kTRUE;
      if (event->fType == kGKeyPress) {
         if (fState == kButtonEngaged) return kTRUE;
         SetState(kButtonDown);
         Pressed();
      } else if (event->fType == kKeyRelease) {
         click = kTRUE;
         if (fState == kButtonEngaged) {
            SetState(kButtonUp);
            SetMenuState(kFALSE);
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);
         } else if (fState == kButtonDown && fStayDown) {
            SetState(kButtonEngaged);
            SetMenuState(kTRUE);
            gVirtualX->GrabPointer(fId, kButtonPressMask | kButtonReleaseMask |
                                   kPointerMotionMask, kNone, fDefaultCursor);
            TGMenuEntry *entry = 0;
            TIter next(fPopMenu->GetListOfEntries());

            while ((entry = (TGMenuEntry *) next())) {
               if ((entry->GetStatus() & kMenuEnableMask) &&
                   !(entry->GetStatus() & kMenuHideMask) &&
                   (entry->GetType() != kMenuSeparator) &&
                   (entry->GetType() != kMenuLabel)) break;
            }
            if (entry) {
               fPopMenu->Activate(entry);
            }
         } else {
            Released();
            SetState(kButtonUp);
         }
      }
   } else {
      fKeyNavigate = kTRUE;
      if (event->fType == kGKeyPress) {
         Event_t ev;
         UInt_t keysym;
         char tmp[2];
         
         gVirtualX->LookupString(event, tmp, sizeof(tmp), keysym);

         TGMenuEntry *ce = 0;
         TIter next(fPopMenu->GetListOfEntries());
         
         while ((ce = (TGMenuEntry*)next())) {
            UInt_t hot = 0;
            if (ce->GetLabel()) hot = ce->GetLabel()->GetHotChar();
            if (!hot || (hot != keysym)) continue;
         
            fPopMenu->Activate(ce);
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);
            SetMenuState(kFALSE);
            ev.fType = kButtonRelease;
            ev.fWindow = fPopMenu->GetId();
            fKeyNavigate = kFALSE;
            return HandleButton(&ev);            
         }

         ce = fPopMenu->GetCurrent();
         
         switch ((EKeySym)keysym) {
         case kKey_Up:
            if (ce) ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->Before(ce);
            while (ce && ((ce->GetType() == kMenuSeparator) ||
                          (ce->GetType() == kMenuLabel) ||
                          !(ce->GetStatus() & kMenuEnableMask))) {
               ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->Before(ce);
            }
            if (!ce) ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->Last();
            break;
         case kKey_Down:
            if (ce) ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->After(ce);
            while (ce && ((ce->GetType() == kMenuSeparator) ||
                          (ce->GetType() == kMenuLabel) ||
                          !(ce->GetStatus() & kMenuEnableMask))) {
               ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->After(ce);
            }
            if (!ce) ce = (TGMenuEntry*)fPopMenu->GetListOfEntries()->First();
            break;
         case kKey_Enter:
         case kKey_Return:
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);
            SetMenuState(kFALSE);
            ev.fType = kButtonRelease;
            ev.fWindow = fPopMenu->GetId();
            fKeyNavigate = kFALSE;
            HandleButton(&ev);
            break;
         case kKey_Escape:
            gVirtualX->GrabPointer(0, 0, 0, 0, kFALSE);
            SetMenuState(kFALSE);
            break;
         default:
            break;
         }
         if (ce) fPopMenu->Activate(ce);
      }
   }
   if (click) {
      Clicked();
      SendMessage(fMsgWindow, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                  (Long_t) fUserData);
      fClient->ProcessLine(fCommand, MK_MSG(kC_COMMAND, kCM_BUTTON), fWidgetId,
                           (Long_t) fUserData);
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGSplitButton::HandleMotion(Event_t *event)
{
   // Handle a motion event in a TGSplitButton.

   if (fKeyNavigate) return kTRUE;

   if (fSplit) {
      if (fMBState == kButtonDown) {
         if (event->fX < (Int_t)fTBWidth) {
            SetMBState(kButtonUp);
            SetState(kButtonDown);
         }
      } else if (fState == kButtonDown) {
         if (event->fX > (Int_t)fTBWidth) {
            SetState(kButtonUp);
            SetMBState(kButtonDown);
         }
            
      }
   }
   return kTRUE;
}

//______________________________________________________________________________
void TGSplitButton::Layout()
{
   // layout text button

   UInt_t dummya = 0, dummyb = 0;
   delete fTLayout;

   TGFont *font = fClient->GetFontPool()->FindFont(fFontStruct);
   if (!font) {
      font = fClient->GetFontPool()->GetFont(fgDefaultFont);
      fFontStruct = font->GetFontStruct();
   }

   fTLayout = font->ComputeTextLayout(fLabel->GetString(), 
                                      fLabel->GetLength(),
                                      fWrapLength, kTextLeft, 0,
                                      &dummya, &dummyb);

   UInt_t dummy = 0;
   font->ComputeTextLayout(fWidestLabel.GetString(), fWidestLabel.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &fTWidth, &dummy);
   font->ComputeTextLayout(fHeighestLabel.GetString(), fHeighestLabel.GetLength(),
                           fWrapLength, kTextLeft, 0,
                           &dummy, &fTHeight);

   fTBWidth = fTWidth + 8;
   fWidth = fTBWidth + fMBWidth;
   fHeight = fTHeight + 7;
   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
void TGSplitButton::HandleMenu(Int_t id) 
{
   // Handle a menu item activation.
   
   SetMenuState(kFALSE);

   if (fSplit) {
      SetMBState(kButtonUp);
      Disconnect(this, "Clicked()", this);
      // connect clicked to the ItemClicked signal with the correct id
      char str[20];
      sprintf(str, "ItemClicked(=%d)", id);
      Connect("Clicked()", "TGSplitButton", this, str);

      // reenable hidden entries
      const TList *list = fPopMenu->GetListOfEntries();
      TIter iter(list);
      fPopMenu->EnableEntry(fEntryId);
      TGHotString *label = fPopMenu->GetEntry(id)->GetLabel();
      TGHotString *tmp = new TGHotString(*label);
      SetText(tmp);
      fPopMenu->HideEntry(id);
      fEntryId = fPopMenu->GetEntry(id)->GetEntryId();
   } else {
      SetState(kButtonUp);
      ItemClicked(id);
   }
   DoRedraw();
}
