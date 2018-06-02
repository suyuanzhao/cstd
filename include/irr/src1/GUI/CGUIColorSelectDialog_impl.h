// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//#include "CGUIColorSelectDialog.h"
#ifdef _IRR_COMPILE_WITH_GUI_
//#include "IGUISkin.h"
//#include "IGUIEnvironment.h"
//#include "IVideoDriver.h"
//#include "IGUIButton.h"
//#include "IGUIStaticText.h"
//#include "IGUIFont.h"
//#include "IGUISpriteBank.h"
//#include "IFileList.h"
//#include "os.h"
const s32 CSD_WIDTH = 350;
const s32 CSD_HEIGHT = 300;
struct subElementPredefines {
  const char* pre;
  const char* init;
  const char* post;
  int x, y;
  int range_down , range_up;
};
static const subElementPredefines Template [] = {
  { "A:", "0", 0, 50, 165, 0, 255 },
  { "R:", "0", 0, 20, 205, 0, 255 },
  { "G:", "0", 0, 20, 230, 0, 255 },
  { "B:", "0", 0, 20, 255, 0, 255 },
  { "H:", "0", "°", 80, 205, 0, 360 },
  { "S:", "0", "%", 80, 230, 0, 100 },
  { "L:", "0", "%", 80, 255, 0, 100 },
};
}
//! constructor
CGUIColorSelectDialog::CGUIColorSelectDialog(const char* title, IGUIEnvironment* environment, IGUIElement* parent, s32 id)
  : IGUIColorSelectDialog(environment, parent, id,
      IRECT((parent->getAbsolutePosition().getWidth() - CSD_WIDTH) / 2,
          (parent->getAbsolutePosition().getHeight() - CSD_HEIGHT) / 2,
          (parent->getAbsolutePosition().getWidth() - CSD_WIDTH) / 2 + CSD_WIDTH,
          (parent->getAbsolutePosition().getHeight() - CSD_HEIGHT) / 2 + CSD_HEIGHT)),
  Dragging(false)
{
#ifdef _DEBUG
  IGUIElement::setDebugName("CGUIColorSelectDialog");
#endif
  Text = title;
  IGUISkin* skin = Environment->getSkin();
  const s32 buttonw = environment->getSkin()->getSize(EGDS_WINDOW_BUTTON_WIDTH);
  const s32 posx = RelativeRect.getWidth() - buttonw - 4;
  CloseButton = Environment->addButton(IRECT(posx, 3, posx + buttonw, 3 + buttonw),
      this, -1, "", skin ? skin->getDefaultText(EGDT_WINDOW_CLOSE) : "Close");
  if (skin && skin->getSpriteBank()) {
    CloseButton->setSpriteBank(skin->getSpriteBank());
    CloseButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_WINDOW_CLOSE), skin->getColor(EGDC_WINDOW_SYMBOL));
    CloseButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_WINDOW_CLOSE), skin->getColor(EGDC_WINDOW_SYMBOL));
  }
  CloseButton->setSubElement(true);
  CloseButton->setTabStop(false);
  CloseButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
  CloseButton->grab();
  OKButton = Environment->addButton(
      IRECT(RelativeRect.getWidth() - 80, 30, RelativeRect.getWidth() - 10, 50),
      this, -1, skin ? skin->getDefaultText(EGDT_MSG_BOX_OK) : "OK");
  OKButton->setSubElement(true);
  OKButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
  OKButton->grab();
  CancelButton = Environment->addButton(
      IRECT(RelativeRect.getWidth() - 80, 55, RelativeRect.getWidth() - 10, 75),
      this, -1, skin ? skin->getDefaultText(EGDT_MSG_BOX_CANCEL) : "Cancel");
  CancelButton->setSubElement(true);
  CancelButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
  CancelButton->grab();
  IVideoDriver* driver = Environment->getVideoDriver();
  ColorRing.Texture = driver->getTexture("#colorring");
  if (0 == ColorRing.Texture) {
    buildColorRing(dimension2du(128, 128), 1,
        Environment->getSkin()->getColor(EGDC_3D_SHADOW));
  }
  IRECT r(20, 20, 0, 0);
  ColorRing.Control = Environment->addImage(ColorRing.Texture, r.UpperLeftCorner, true, this);
  ColorRing.Control->setSubElement(true);
  ColorRing.Control->grab();
  int i;
  for (i = 0; i != sizeof(Template) / sizeof(subElementPredefines); ++i) {
    if (Template[i].pre) {
      r.UpperLeftCorner.X = Template[i].x;
      r.UpperLeftCorner.Y = Template[i].y;
      r.LowerRightCorner.X = r.UpperLeftCorner.X + 15;
      r.LowerRightCorner.Y = r.UpperLeftCorner.Y + 20;
      IGUIElement* t = Environment->addStaticText(Template[i].pre, r, false, false, this);
      t->setSubElement(true);
    }
    if (Template[i].post) {
      r.UpperLeftCorner.X = Template[i].x + 56;
      r.UpperLeftCorner.Y = Template[i].y;
      r.LowerRightCorner.X = r.UpperLeftCorner.X + 15;
      r.LowerRightCorner.Y = r.UpperLeftCorner.Y + 20;
      IGUIElement* t = Environment->addStaticText(Template[i].post, r, false, false, this);
      t->setSubElement(true);
    }
    r.UpperLeftCorner.X = Template[i].x + 15;
    r.UpperLeftCorner.Y = Template[i].y - 2;
    r.LowerRightCorner.X = r.UpperLeftCorner.X + 40;
    r.LowerRightCorner.Y = r.UpperLeftCorner.Y + 20;
    IGUISpinBox* spin = Environment->addSpinBox(Template[i].init, r, true, this);
    spin->setSubElement(true);
    spin->setDecimalPlaces(0);
    spin->setRange((f32)Template[i].range_down, (f32)Template[i].range_up);
    spin->grab();
    Battery.push_back(spin);
  }
  bringToFront(CancelButton);
  bringToFront(OKButton);
}
//! destructor
CGUIColorSelectDialog::~CGUIColorSelectDialog()
{
  if (CloseButton) {
    CloseButton->drop();
  }
  if (OKButton) {
    OKButton->drop();
  }
  if (CancelButton) {
    CancelButton->drop();
  }
  int i;
  for (i = 0; i != Battery_size; ++i) {
    Battery[i]->drop();
  }
  if (ColorRing.Control) {
    ColorRing.Control->drop();
  }
}
//! renders a antialiased, colored ring
void CGUIColorSelectDialog::buildColorRing(const dimension2du& dim, s32 supersample, const SColor& borderColor)
{
  const dimension2du d(dim.Width * supersample, dim.Height * supersample);
  IVideoDriver* driver = Environment->getVideoDriver();
  IImage* RawTexture = driver->createImage(ECF_A8R8G8B8, d);
  RawTexture->fill(0x00808080);
  const s32 radiusOut = (d.Width / 2) - 4;
  const s32 fullR2 = radiusOut * radiusOut;
  SColorf rgb1(0, 0, 0);
  SColorHSL hsl;
  hsl.Luminance = 50;
  hsl.Saturation = 100;
  position2di p;
  for (p.Y = -radiusOut;  p.Y <= radiusOut;  p.Y += 1) {
    s32 y2 = p.Y * p.Y;
    for (p.X = -radiusOut; p.X <= radiusOut; p.X += 1) {
      s32 r2 = y2 + (p.X * p.X);
      // test point in circle
      s32 testa = r2 - fullR2;
      if (testa < 0) {
        // dotproduct u ( x,y ) * v ( 1, 0 ) = cosinus(a)
        const f32 r = sqrtf((f32) r2);
        // normalize, dotproduct = xnorm
        const f32 xn = r == 0.f ? 0.f : -p.X * reciprocal(r);
        hsl.Hue = acosf(xn) * RADTODEG;
        if (p.Y > 0) {
          hsl.Hue = 360 - hsl.Hue;
        }
        hsl.Hue -= 90;
        const f32 rTest = r / radiusOut;
#if 0
        if (rTest < 0.33f) {
          // luminance from 0 to 50
          hsl.Luminance = 50 * (rTest / 0.33);
          hsl.Saturation = 0.f;
          hsl.toRGB(rgb1);
        }
        else if (rTest < 0.66f) {
          // saturation from 0 to 100
          hsl.Saturation = 100 * ((rTest - 0.33f) / 0.33f);
          hsl.Luminance = 50;
          hsl.toRGB(rgb1);
        }
        else {
          // luminance from 50 to 100
          hsl.Luminance = 100 * (0.5f + ((rTest - 0.66f) / .66f));
          hsl.Saturation = 100;
          hsl.toRGB(rgb1);
        }
        // borders should be slightly transparent
        if (rTest >= 0.95f) {
          rgb1.a = (1.f - rTest) * 20;
        }
        else {
          rgb1.a = 1.f;
        }
#else
        if (rTest > 0.5f) {
          hsl.Saturation = 100;
          hsl.Luminance = 50;
          hsl.toRGB(rgb1);
        }
        // borders should be slightly transparent
        if (rTest < 0.5f) {
          rgb1.a = 0;
        }
        else if (rTest >= 0.95f) {
          rgb1.a = (1.f - rTest) * 20;
        }
        else if (rTest <= 0.55f) {
          rgb1.a = (rTest - 0.5f) * 20;
        }
        else {
          rgb1.a = 1.f;
        }
#endif
        RawTexture->setPixel(4 + p.X + radiusOut, 4 + p.Y + radiusOut, rgb1.toSColor());
      }
    }
  }
  RawTexture->unlock();
  if (supersample > 1) {
    IImage* filter = driver->createImage(ECF_A8R8G8B8, dim);
    RawTexture->copyToScalingBoxFilter(filter);
    RawTexture->drop();
    RawTexture = filter;
  }
  bool generateMipLevels = driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
  driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);
  ColorRing.Texture = driver->addTexture("#colorring", RawTexture);
  RawTexture->drop();
  driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, generateMipLevels);
}
//! called if an event happened.
bool CGUIColorSelectDialog::OnEvent(const SEvent& event)
{
  if (isEnabled()) {
    switch (event.EventType) {
    case EET_GUI_EVENT:
      switch (event.GUIEvent.EventType) {
      case EGET_SPINBOX_CHANGED: {
        int i;
        for (i = 0; i != Battery_size; ++i) {
          if (event.GUIEvent.Caller == Battery[i]) {
            if (i < 4) {
              SColor rgb1((u32)Battery[0]->getValue(), (u32)Battery[1]->getValue(),
                  (u32)Battery[2]->getValue(), (u32)Battery[3]->getValue());
              SColorHSL hsl;
              SColorf rgb2(rgb1);
              hsl.fromRGB(rgb2);
              Battery[4]->setValue(hsl.Hue);
              Battery[5]->setValue(hsl.Saturation);
              Battery[6]->setValue(hsl.Luminance);
            }
            else {
              SColorHSL hsl(Battery[4]->getValue(), Battery[5]->getValue(),
                  Battery[6]->getValue());
              SColorf rgb2;
              hsl.toRGB(rgb2);
              SColor rgb1 = rgb2.toSColor();
              Battery[1]->setValue((f32)rgb1.getRed());
              Battery[2]->setValue((f32)rgb1.getGreen());
              Battery[3]->setValue((f32)rgb1.getBlue());
            }
          }
        }
        return true;
      }
      case EGET_ELEMENT_FOCUS_LOST:
        Dragging = false;
        break;
      case EGET_BUTTON_CLICKED:
        if (event.GUIEvent.Caller == CloseButton ||
            event.GUIEvent.Caller == CancelButton) {
          sendCancelEvent();
          remove();
          return true;
        }
        else if (event.GUIEvent.Caller == OKButton) {
          sendSelectedEvent();
          remove();
          return true;
        }
        break;
      case EGET_LISTBOX_CHANGED:
      case EGET_LISTBOX_SELECTED_AGAIN:
      default:
        break;
      }
      break;
    case EET_MOUSE_INPUT_EVENT:
      switch (event.MouseInput.Event) {
      case EMIE_LMOUSE_PRESSED_DOWN:
        DragStart.X = event.MouseInput.X;
        DragStart.Y = event.MouseInput.Y;
        Dragging = true;
        Environment->setFocus(this);
        return true;
      case EMIE_LMOUSE_LEFT_UP:
        Dragging = false;
        Environment->removeFocus(this);
        return true;
      case EMIE_MOUSE_MOVED:
        if (Dragging) {
          // gui window should not be dragged outside its parent
          if (Parent)
            if (event.MouseInput.X < Parent->getAbsolutePosition().UpperLeftCorner.X + 1 ||
                event.MouseInput.Y < Parent->getAbsolutePosition().UpperLeftCorner.Y + 1 ||
                event.MouseInput.X > Parent->getAbsolutePosition().LowerRightCorner.X - 1 ||
                event.MouseInput.Y > Parent->getAbsolutePosition().LowerRightCorner.Y - 1) {
              return true;
            }
          move(position2di(event.MouseInput.X - DragStart.X, event.MouseInput.Y - DragStart.Y));
          DragStart.X = event.MouseInput.X;
          DragStart.Y = event.MouseInput.Y;
          return true;
        }
      default:
        break;
      }
    default:
      break;
    }
  }
  return IGUIElement::OnEvent(event);
}
//! draws the element and its children
void CGUIColorSelectDialog::draw()
{
  if (!IsVisible) {
    return;
  }
  IGUISkin* skin = Environment->getSkin();
  IRECT rect = skin->draw3DWindowBackground(this, true, skin->getColor(EGDC_ACTIVE_BORDER),
      AbsoluteRect, &AbsoluteClippingRect);
  if (Text_size) {
    rect.UpperLeftCorner.X += 2;
    rect.LowerRightCorner.X -= skin->getSize(EGDS_WINDOW_BUTTON_WIDTH) + 5;
    IGUIFont* font = skin->getFont(EGDF_WINDOW);
    if (font)
      font->draw(Text, rect, skin->getColor(EGDC_ACTIVE_CAPTION), false, true,
          &AbsoluteClippingRect);
  }
  IGUIElement::draw();
  // draw color selector after the window elements
  vector2di pos(ColorRing.Control->getAbsolutePosition().UpperLeftCorner);
  pos.X += ColorRing.Texture->getOriginalSize().Width / 2;
  pos.Y += ColorRing.Texture->getOriginalSize().Height / 2;
#if 0
  const f32 h = Battery[4]->getValue();
  const f32 s = Battery[5]->getValue();
  const f32 l = Battery[6]->getValue();
  const f32 factor = 58.f * (((s == 0) && (l < 50)) ? (l * 0.33f / 50) : (
      (s < 100) ? ((.33f + (s * 0.33f / 100))) : ((0.66f + (l - 50) * 0.33f / 50))));
#else
  const f32 factor = 44;
#endif
  pos.X += round32(sinf(Battery[4]->getValue() * DEGTORAD) * factor);
  pos.Y -= round32(cosf(Battery[4]->getValue() * DEGTORAD) * factor);
  Environment->getVideoDriver()->draw2DPolygon(pos, 4, 0xffffffff, 4);
}
SColor CGUIColorSelectDialog::getColor()
{
  return SColor((u32)Battery[0]->getValue(), (u32)Battery[1]->getValue(),
      (u32)Battery[2]->getValue(), (u32)Battery[3]->getValue());
}
SColorHSL CGUIColorSelectDialog::getColorHSL()
{
  return SColorHSL(Battery[4]->getValue(), Battery[5]->getValue(),
      Battery[6]->getValue());
}
//! sends the event that the file has been selected.
void CGUIColorSelectDialog::sendSelectedEvent()
{
  SEvent event;
  event.EventType = EET_GUI_EVENT;
  event.GUIEvent.Caller = this;
  event.GUIEvent.Element = 0;
  event.GUIEvent.EventType = EGET_FILE_SELECTED;
  Parent->OnEvent(event);
}
//! sends the event that the file choose process has been canceld
void CGUIColorSelectDialog::sendCancelEvent()
{
  SEvent event;
  event.EventType = EET_GUI_EVENT;
  event.GUIEvent.Caller = this;
  event.GUIEvent.Element = 0;
  event.GUIEvent.EventType = EGET_FILE_CHOOSE_DIALOG_CANCELLED;
  Parent->OnEvent(event);
}
#endif // _IRR_COMPILE_WITH_GUI_
