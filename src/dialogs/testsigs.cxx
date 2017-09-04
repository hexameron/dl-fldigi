// generated by Fast Light User Interface Designer (fluid) version 1.0304

#include "gettext.h"
#include "testsigs.h"
#include "configuration.h"

Fl_Counter2 *noiseDB=(Fl_Counter2 *)0;

static void cb_noiseDB(Fl_Counter2* o, void*) {
  progdefaults.s2n = o->value();
}

Fl_Check_Button *btnNoiseOn=(Fl_Check_Button *)0;

static void cb_btnNoiseOn(Fl_Check_Button* o, void*) {
  progdefaults.noise = o->value();
}

Fl_Counter *ctrl_freq_offset=(Fl_Counter *)0;

Fl_Check_Button *btnOffsetOn=(Fl_Check_Button *)0;

Fl_Counter2 *xmtimd=(Fl_Counter2 *)0;

Fl_Check_Button *btn_imd_on=(Fl_Check_Button *)0;

Fl_Double_Window* make_testdialog() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(480, 100, _("Test Signals"));
    w = o; if (w) {/* empty */}
    { Fl_Counter2* o = noiseDB = new Fl_Counter2(10, 41, 127, 21, _("Noise level (db)"));
      noiseDB->box(FL_UP_BOX);
      noiseDB->color(FL_BACKGROUND_COLOR);
      noiseDB->selection_color(FL_INACTIVE_COLOR);
      noiseDB->labeltype(FL_NORMAL_LABEL);
      noiseDB->labelfont(0);
      noiseDB->labelsize(14);
      noiseDB->labelcolor(FL_FOREGROUND_COLOR);
      noiseDB->minimum(-18);
      noiseDB->maximum(60);
      noiseDB->value(20);
      noiseDB->callback((Fl_Callback*)cb_noiseDB);
      noiseDB->align(Fl_Align(FL_ALIGN_TOP));
      noiseDB->when(FL_WHEN_CHANGED);
      o->value(progdefaults.s2n);
      o->lstep(1);
    } // Fl_Counter2* noiseDB
    { Fl_Check_Button* o = btnNoiseOn = new Fl_Check_Button(39, 73, 68, 12, _("Noise on"));
      btnNoiseOn->down_box(FL_DOWN_BOX);
      btnNoiseOn->callback((Fl_Callback*)cb_btnNoiseOn);
      o->value(progdefaults.noise);
    } // Fl_Check_Button* btnNoiseOn
    { Fl_Counter* o = ctrl_freq_offset = new Fl_Counter(174, 41, 127, 21, _("freq-offset"));
      ctrl_freq_offset->tooltip(_("ONLY FOR TESTING !"));
      ctrl_freq_offset->minimum(-250);
      ctrl_freq_offset->maximum(250);
      ctrl_freq_offset->align(Fl_Align(FL_ALIGN_TOP));
      o->lstep(10);
    } // Fl_Counter* ctrl_freq_offset
    { btnOffsetOn = new Fl_Check_Button(203, 73, 68, 12, _("Offset on"));
      btnOffsetOn->down_box(FL_DOWN_BOX);
    } // Fl_Check_Button* btnOffsetOn
    { Fl_Counter2* o = xmtimd = new Fl_Counter2(339, 41, 127, 21, _("PSK IMD\nlevel (db)"));
      xmtimd->box(FL_UP_BOX);
      xmtimd->color(FL_BACKGROUND_COLOR);
      xmtimd->selection_color(FL_INACTIVE_COLOR);
      xmtimd->labeltype(FL_NORMAL_LABEL);
      xmtimd->labelfont(0);
      xmtimd->labelsize(14);
      xmtimd->labelcolor(FL_FOREGROUND_COLOR);
      xmtimd->minimum(-40);
      xmtimd->maximum(-15);
      xmtimd->value(-30);
      xmtimd->align(Fl_Align(FL_ALIGN_TOP));
      xmtimd->when(FL_WHEN_CHANGED);
      o->lstep(1.0);
    } // Fl_Counter2* xmtimd
    { btn_imd_on = new Fl_Check_Button(368, 73, 68, 12, _("IMD on"));
      btn_imd_on->down_box(FL_DOWN_BOX);
    } // Fl_Check_Button* btn_imd_on
    { Fl_Box* o = new Fl_Box(2, 2, 368, 20, _("!! DO NOT USE ON LIVE TRANSMITER !!"));
      o->labelcolor((Fl_Color)80);
    } // Fl_Box* o
    o->end();
  } // Fl_Double_Window* o
  return w;
}
