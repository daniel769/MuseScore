//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: tempotext.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "tempotext.h"
#include "tempo.h"
#include "system.h"
#include "measure.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      _tempo      = 2.0;      // propertyDefault(P_TEMPO).toDouble();
      _followText = false;
      setPlacement(ABOVE);
      setTextStyle(s->textStyle(TEXT_STYLE_TEMPO));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(Xml& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      if (_followText)
            xml.tag("followText", _followText);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "tempo")
                  _tempo = e.text().toDouble();
            else if (tag == "followText")
                  _followText = e.text().toInt();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      if (score()->mscVersion() < 119) {
            //
            // Reset text in old version to
            // style.
            //
//TODO            if (textStyle() != TEXT_STYLE_INVALID) {
//                  setStyled(true);
//                  styleChanged();
//                  }
            }
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      const char* pattern;
      qreal f;
      TempoPattern(const char* s, qreal v) : pattern(s), f(v) {}
      };

//---------------------------------------------------------
//   textChanged
//    text may have changed
//---------------------------------------------------------

void TempoText::textChanged()
      {
      if (!_followText)
            return;
      QString s = getText();

      static const TempoPattern tp[] = {
            TempoPattern("\\xd834\\xdd5f = (\\d+)", 1.0/60.0),      // 1/4
            TempoPattern("\\xd834\\xdd5e = (\\d+)", 1.0/30.0),      // 1/2
            TempoPattern("\\xd834\\xdd60 = (\\d+)", 1.0/120.0),     // 1/8
            TempoPattern("\\xd834\\xdd5f\\xd834\\xdd6d = (\\d+)", 1.5/60.0),   // dotted 1/4
            TempoPattern("\\xd834\\xdd5e\\xd834\\xdd6d = (\\d+)", 1.5/30.0),   // dotted 1/2
            TempoPattern("\\xd834\\xdd60\\xd834\\xdd6d = (\\d+)", 1.5/120.0),  // dotted 1/8
            };

      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            QRegExp re(tp[i].pattern);      // 1/4
            if (re.indexIn(s) != -1) {
                  QStringList sl = re.capturedTexts();
                  if (sl.size() == 2) {
                        qreal nt = qreal(sl[1].toInt()) * tp[i].f;
                        if (nt != _tempo) {
                              _tempo = qreal(sl[1].toInt()) * tp[i].f;
                              score()->setPlaylistDirty(true);
                              }
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   undoSetTempo
//---------------------------------------------------------

void TempoText::undoSetTempo(qreal v)
      {
      score()->undoChangeProperty(this, P_TEMPO, v);
      }

//---------------------------------------------------------
//   undoSetFollowText
//---------------------------------------------------------

void TempoText::undoSetFollowText(bool v)
      {
      score()->undoChangeProperty(this, P_TEMPO_FOLLOW_TEXT, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TempoText::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_TEMPO:             return _tempo;
            case P_TEMPO_FOLLOW_TEXT: return _followText;
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TempoText::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_TEMPO:
                  _tempo = v.toDouble();
                  break;
            case P_TEMPO_FOLLOW_TEXT:
                  _followText = v.toBool();
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TempoText::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_TEMPO:             return 2.0;
            case P_TEMPO_FOLLOW_TEXT: return false;
            case P_PLACEMENT:         return ABOVE;
            default:                  return Text::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TempoText::layout()
      {
      Text::layout();
      if (placement() == BELOW) {
            rypos() = -rypos() + 4 * spatium();
            // rUserYoffset() *= -1;
            // text height ?
            }
      }

