// ----------------------------------------------------------------------------
//
//	fl_digi.cxx
//
// Copyright (C) 2006
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.
//
// fldigi is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ----------------------------------------------------------------------------

#include <config.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <string>

#include "fl_digi.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Tile.H>
#include <FL/x.H>

#include "waterfall.h"
#include "raster.h"
#include "main.h"
#include "threads.h"
#include "trx.h"
#if USE_HAMLIB
	#include "hamlib.h"
#endif
#include "rigio.h"
#include "rigMEM.h"
#include "psk.h"
#include "cw.h"
#include "mfsk.h"
#include "rtty.h"
#include "olivia.h"
#include "dominoex.h"
#include "feld.h"
#include "throb.h"
#include "wwv.h"
#include "analysis.h"

#include "ascii.h"
#include "globals.h"
#include "misc.h"
//#include "help.h"

#include "Config.h"
#include "configuration.h"
#include "macros.h"
#include "macroedit.h"
#include "logger.h"
#include "qrzcall.h"

#include "combo.h"
#include "font_browser.h"
#include "fldigi-icon-48.xpm"

#include "status.h"

#include "rigsupport.h"

#include "qrunner.h"

Fl_Double_Window	*fl_digi_main=(Fl_Double_Window *)0;

cMixer mixer;

bool useCheckButtons = false;

Fl_Button			*btnTune = (Fl_Button *)0;
Fl_Tile_check				*TiledGroup = 0;
ReceiveWidget			*ReceiveText = 0;
TransmitWidget			*TransmitText = 0;
Fl_Text_Buffer		*rcvBuffer = (Fl_Text_Buffer *)0;
Fl_Text_Buffer		*xmtBuffer = (Fl_Text_Buffer *)0;
Raster				*FHdisp;
Fl_Box				*StatusBar = (Fl_Box *)0;
Fl_Box				*Status2 = (Fl_Box *)0;
Fl_Box				*Status1 = (Fl_Box *)0;
Fl_Box				*WARNstatus = (Fl_Box *)0;
Fl_Button			*MODEstatus = (Fl_Button *)0;
Fl_Button 			*btnMacro[10];
Fl_Button			*btnAltMacros;
Fl_Light_Button		*afconoff;
Fl_Light_Button		*sqlonoff;
Fl_Check_Button		*chk_afconoff;
Fl_Check_Button		*chk_sqlonoff;
Fl_Input			*inpFreq;
Fl_ComboBox			*cboBand;
Fl_Button			*btnSideband;
Fl_Input			*inpTime;
Fl_Input			*inpCall;
Fl_Input			*inpName;
Fl_Input			*inpRstIn;
Fl_Input			*inpRstOut;
Fl_Input			*inpQth;
Fl_Input			*inpLoc;
Fl_Input			*inpNotes;
Fl_Input			*inpAZ;	// WA5ZNU
Fl_Button			*qsoTime;
Fl_Button			*qsoClear;
Fl_Button			*qsoSave;
Fl_Button			*btnMacroTimer;
Fl_Button			*btnQRZ;
Fl_Slider			*valRcvMixer;
Fl_Slider			*valXmtMixer;

bool				altMacros = false;
bool				bSaveFreqList = false;
string				strMacroName[10];


waterfall			*wf = (waterfall *)0;
Digiscope			*digiscope = (Digiscope *)0;
Fl_Slider			*sldrSquelch = (Fl_Slider *)0;
Fl_Progress			*pgrsSquelch = (Fl_Progress *)0;

Fl_RGB_Image		*feld_image = 0;

Pixmap				fldigi_icon_pixmap;


int IMAGE_WIDTH = DEFAULT_IMAGE_WIDTH;
int Hwfall = DEFAULT_HWFALL;
int HNOM = DEFAULT_HNOM;
int WNOM = DEFAULT_WNOM;


void cb_init_mode(Fl_Widget *, void *arg);
Fl_Widget *modem_config_tab;
Fl_Menu_Item *quick_change;
Fl_Menu_Item quick_change_psk[] = {
	{ mode_info[MODE_BPSK31].name, 0, cb_init_mode, (void *)MODE_BPSK31 },
	{ mode_info[MODE_PSK63].name, 0, cb_init_mode, (void *)MODE_PSK63 },
	{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125 },
	{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250 },
	{ 0 }
};

Fl_Menu_Item quick_change_qpsk[] = {
	{ mode_info[MODE_QPSK31].name, 0, cb_init_mode, (void *)MODE_QPSK31 },
	{ mode_info[MODE_QPSK63].name, 0, cb_init_mode, (void *)MODE_QPSK63 },
	{ mode_info[MODE_QPSK125].name, 0, cb_init_mode, (void *)MODE_QPSK125 },
	{ mode_info[MODE_QPSK250].name, 0, cb_init_mode, (void *)MODE_QPSK250 },
	{ 0 }
};

Fl_Menu_Item quick_change_mfsk[] = {
	{ mode_info[MODE_MFSK8].name, 0, cb_init_mode, (void *)MODE_MFSK8 },
	{ mode_info[MODE_MFSK16].name, 0, cb_init_mode, (void *)MODE_MFSK16 },
	{ 0 }
};

Fl_Menu_Item quick_change_domino[] = {
	{ mode_info[MODE_DOMINOEX4].name, 0, cb_init_mode, (void *)MODE_DOMINOEX4 },
	{ mode_info[MODE_DOMINOEX5].name, 0, cb_init_mode, (void *)MODE_DOMINOEX5 },
	{ mode_info[MODE_DOMINOEX8].name, 0, cb_init_mode, (void *)MODE_DOMINOEX8 },
	{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11 },
	{ mode_info[MODE_DOMINOEX16].name, 0, cb_init_mode, (void *)MODE_DOMINOEX16 },
	{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22 },
	{ 0 }
};

Fl_Menu_Item quick_change_feld[] = {
	{ mode_info[MODE_FELDHELL].name, 0, cb_init_mode, (void *)MODE_FELDHELL },
	{ mode_info[MODE_FSKHELL].name, 0, cb_init_mode, (void *)MODE_FSKHELL },
	{ mode_info[MODE_FSKH105].name, 0, cb_init_mode, (void *)MODE_FSKH105 },
	{ 0 }
};

Fl_Menu_Item quick_change_throb[] = {
	{ mode_info[MODE_THROB1].name, 0, cb_init_mode, (void *)MODE_THROB1 },
	{ mode_info[MODE_THROB2].name, 0, cb_init_mode, (void *)MODE_THROB2 },
	{ mode_info[MODE_THROB4].name, 0, cb_init_mode, (void *)MODE_THROB4 },
	{ mode_info[MODE_THROBX1].name, 0, cb_init_mode, (void *)MODE_THROBX1 },
	{ mode_info[MODE_THROBX2].name, 0, cb_init_mode, (void *)MODE_THROBX2 },
	{ mode_info[MODE_THROBX4].name, 0, cb_init_mode, (void *)MODE_THROBX4 },
	{ 0 }
};


void startup_modem(modem *m)
{
	trx_start_modem(m);

	restoreFocus();

	FL_LOCK_D();
	if (m == feld_modem ||
		m == feld_FMmodem ||
		m == feld_FM105modem ) {
		ReceiveText->Hide();
		FHdisp->show();
	} else {
		ReceiveText->Show();
		FHdisp->hide();
	}
	FL_UNLOCK_D();
	FL_AWAKE_D();

}

void cb_mnuOpenMacro(Fl_Menu_*, void*) {
	macros.openMacroFile();
	restoreFocus();
}

void cb_mnuSaveMacro(Fl_Menu_*, void*) {
	macros.saveMacroFile();
	restoreFocus();
}

bool logging = false;
void cb_mnuLogFile(Fl_Menu_ *, void *) {
	logging = !logging;
	restoreFocus();
}

void clean_exit() {
	if (progdefaults.changed == true) {
		if (fl_choice("Configuration changed, Save", "No", "Yes", 0) == 1)
			progdefaults.saveDefaults();
	}
	if (macros.changed == true) {
		if (fl_choice("Macros changed, Save", "No", "Yes", 0) == 1)
			macros.saveMacroFile();
	}
	if (Maillogfile)
		Maillogfile->log_to_file_stop();
	if (logfile)
		logfile->log_to_file_stop();
	
	if (bSaveFreqList)
		saveFreqList();
		
	progStatus.saveLastState();

#if USE_HAMLIB
	hamlib_close();
#endif
	rigCAT_close();
	rigMEM_close();

	mixer.closeMixer();
	active_modem->set_stopflag(true);
	while (trx_state != STATE_RX)
		MilliSleep(100);
		
//	fl_lock (&trx_mutex);
//	if (active_modem) {
//		active_modem->shutdown();
//		MilliSleep(100);
//		delete active_modem;
//	}
//	active_modem = (modem *) 0;
//	fl_unlock (&trx_mutex);

//#if USE_HAMLIB	
//	delete xcvr;
//#endif
//	delete push2talk;
	
	exit(0);
}

void cb_E(Fl_Menu_*, void*) {
	clean_exit();
}

void cb_wMain( Fl_Widget *, void * ) 
{
	if (Fl::event_key(FL_Escape)) {
		TransmitText->clear();
		active_modem->set_stopflag(true);
	} else
		clean_exit();
}

void init_modem(trx_mode mode)
{
	quick_change = 0;
	modem_config_tab = tabsModems->child(0);

	switch (mode) {
	case MODE_NEXT:
		if ((mode = active_modem->get_mode() + 1) == NUM_MODES)
			mode = 0;
		return init_modem(mode);
	case MODE_PREV:
		if ((mode = active_modem->get_mode() - 1) < 0)
			mode = NUM_MODES - 1;
		return init_modem(mode);

	case MODE_CW:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new cw);
		modem_config_tab = tabCW;
		break;

	case MODE_DOMINOEX4: case MODE_DOMINOEX5: case MODE_DOMINOEX8:
	case MODE_DOMINOEX11: case MODE_DOMINOEX16: case MODE_DOMINOEX22:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new dominoex(mode));
		quick_change = quick_change_domino;
		modem_config_tab = tabDomEX;
		break;

	case MODE_FELDHELL: case MODE_FSKHELL: case MODE_FSKH105:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new feld(mode));
		quick_change = quick_change_feld;
		modem_config_tab = tabFeld;
		break;

	case MODE_MFSK8: case MODE_MFSK16:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new mfsk(mode));
		quick_change = quick_change_mfsk;
		break;

	case MODE_BPSK31: case MODE_PSK63: case MODE_PSK125: case MODE_PSK250:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new psk(mode));
		quick_change = quick_change_psk;
		modem_config_tab = tabPSK;
		break;
	case MODE_QPSK31: case MODE_QPSK63: case MODE_QPSK125: case MODE_QPSK250:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new psk(mode));
		quick_change = quick_change_qpsk;
		modem_config_tab = tabPSK;
		break;

	case MODE_OLIVIA:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new olivia);
		modem_config_tab = tabOlivia;
		break;

	case MODE_RTTY:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new rtty(mode));
		modem_config_tab = tabRTTY;
		break;

	case MODE_THROB1: case MODE_THROB2: case MODE_THROB4:
	case MODE_THROBX1: case MODE_THROBX2: case MODE_THROBX4:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new throb(mode));
		quick_change = quick_change_throb;
		break;

	case MODE_WWV:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new wwv);
		break;

	case MODE_ANALYSIS:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new anal);
		break;

	default:
		cerr << "Unknown mode: " << mode << '\n';
		return init_modem(MODE_BPSK31);
	}

	clear_StatusMessages();
	progStatus.saveModeState(mode);
}

void init_modem_sync(trx_mode m)
{
#ifndef NDEBUG
        if (GET_THREAD_ID() == TRX_TID)
                cerr << "trx thread called init_modem_sync!\n";
#endif

        wait_modem_ready_prep();
        init_modem(m);
        wait_modem_ready_cmpl();
}

void cb_init_mode(Fl_Widget *, void *mode)
{
	init_modem(reinterpret_cast<trx_mode>(mode));
}


void restoreFocus()
{
	FL_LOCK_D();
	TransmitText->take_focus();
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void macro_cb(Fl_Widget *w, void *v)
{
	int b = (int)(reinterpret_cast<long> (v));
	b += (altMacros ? 10 : 0);
	int mouse = Fl::event_button();
	if (mouse == 1 && !macros.text[b].empty())
		macros.execute(b);
	else if (mouse == 3)
		editMacro(b);
	restoreFocus();
}

void altmacro_cb(Fl_Widget *w, void *v)
{
	altMacros = !altMacros;
	FL_LOCK_D();
	for (int i = 0; i < 10; i++)
		btnMacro[i]->label(macros.name[i + (altMacros ? 10: 0)].c_str());
	FL_UNLOCK_D();
	restoreFocus();
}

void cb_mnuConfigOperator(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabOperator);
	dlgConfig->show();
}

void cb_mnuConfigWaterfall(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabWaterfall);
	dlgConfig->show();
}

void cb_mnuConfigVideo(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabVideo);
	dlgConfig->show();
}

void cb_mnuConfigQRZ(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabQRZ);
	dlgConfig->show();
}

void cb_mnuConfigMisc(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabMisc);
	dlgConfig->show();
}

void cb_mnuConfigRigCtrl(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabRig);
	dlgConfig->show();
}

void cb_mnuConfigSoundCard(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabSoundCard);
	dlgConfig->show();
}

void cb_mnuConfigModems(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabModems);
	dlgConfig->show();
}

#if USE_SNDFILE
bool capval = false;
bool genval = false;
bool playval = false;
void cb_mnuCapture(Fl_Widget *w, void *d)
{
	Fl_Menu_Item *m = (Fl_Menu_Item *)(((Fl_Menu_*)w)->mvalue());
	if (!scard) return;
	if (playval || genval) {
		m->flags &= ~FL_MENU_VALUE;
		return;
	}
	capval = m->value();
	if(!scard->Capture(capval)) {
		m->flags &= ~FL_MENU_VALUE;
		capval = false;
	}
}

void cb_mnuGenerate(Fl_Widget *w, void *d)
{
	Fl_Menu_Item *m = (Fl_Menu_Item *)(((Fl_Menu_*)w)->mvalue());
	if (!scard) return;
	if (capval || playval) {
		m->flags &= ~FL_MENU_VALUE;
		return;
	}
	genval = m->value();
	if (!scard->Generate(genval)) {
		m->flags &= ~FL_MENU_VALUE;
		genval = false;
	}
}

void cb_mnuPlayback(Fl_Widget *w, void *d)
{
	Fl_Menu_Item *m = (Fl_Menu_Item *)(((Fl_Menu_*)w)->mvalue());
	if (!scard) return;
	if (capval || genval) {
		m->flags &= ~FL_MENU_VALUE;
		return;
	}
	playval = m->value();
	if(!scard->Playback(playval)) {
		m->flags &= ~FL_MENU_VALUE;
		playval = false;
	}
}
#endif // USE_SNDFILE

void cb_FontBrowser(Font_Browser*, void* v)
{
	Font_Browser *ft= (Font_Browser*)v;

	Fl_Font fnt = ft->fontNumber();
	int size = ft->fontSize();

	ReceiveText->setFont(fnt);
	ReceiveText->setFontSize(size);
	
	TransmitText->setFont(fnt);
	TransmitText->setFontSize(size);
	
	progdefaults.Fontnbr = (int)(fnt);
	progdefaults.FontSize = size;
	
	ft->hide();
}

void cb_mnuConfigFonts(Fl_Menu_*, void *) {
	static Font_Browser *b = (Font_Browser *)0;
	if (!b) {
		b = new Font_Browser;
		b->fontNumber((Fl_Font)progdefaults.Fontnbr);
		b->fontSize(progdefaults.FontSize);
//		b->fontColor(progdefaults.FontColor);
	}
	b->callback((Fl_Callback*)cb_FontBrowser, (void*)(b));
	b->show();
}

void cb_mnuSaveConfig(Fl_Menu_ *, void *) {
	progdefaults.saveDefaults();
	restoreFocus();
}

//void cb_mnuHelp(Fl_Menu_*,void*) {
//	show_help();
//	restoreFocus();
//}

void cb_mnuAbout(Fl_Menu_*,void*) {
	fl_message ("fldigi @@W1HKJ\n\nw1hkj@@w1hkj.com\n\nVersion - %s", PACKAGE_VERSION);
	restoreFocus();
}

void cbTune(Fl_Widget *w, void *) {
	Fl_Button *b = (Fl_Button *)w;
	if (active_modem == wwv_modem || active_modem == anal_modem) {
		b->value(0);
		return;
	}
	if (b->value() == 1) {
		b->labelcolor(FL_RED);
		fl_lock(&trx_mutex);
			trx_state = STATE_TUNE;
		fl_unlock(&trx_mutex);
	} else {
		b->labelcolor(FL_BLACK);
		fl_lock(&trx_mutex);
			trx_state = STATE_RX;
		fl_unlock(&trx_mutex);
	}
	restoreFocus();
}

void cb_mnuRig(Fl_Menu_ *, void *) {
	if (!rigcontrol)
		createRigDialog();
	rigcontrol->show();
}

void closeRigDialog() {
	rigcontrol->hide();
}

void cb_sldrSquelch(Fl_Slider* o, void*) {
	active_modem->set_squelch(o->value());
	progdefaults.sldrSquelchValue = o->value();
	restoreFocus();
}

char *zuluTime()
{
	struct tm *tm;
	time_t t;
	static char logtime[10];
	time(&t);
    tm = gmtime(&t);
	strftime(logtime, sizeof(logtime), "%H%M", tm);
	return logtime;
}

void qsoTime_cb(Fl_Widget *b, void *)
{
	FL_LOCK_D();
	inpTime->value(zuluTime());
	FL_UNLOCK_D();
	FL_AWAKE_D();
	restoreFocus();
}

void clearQSO()
{
	FL_LOCK_D();
		inpTime->value(zuluTime());
		inpCall->value("");
		inpName->value("");
		inpRstIn->value("");
		inpRstOut->value("");
		inpQth->value("");
		inpLoc->value("");
		inpAZ->value(""); // WA5ZNU
		inpNotes->value("");
	FL_UNLOCK_D();
}

void qsoClear_cb(Fl_Widget *b, void *)
{
	if (fl_choice ("Confirm Clear", "Cancel", "OK", NULL) == 1) {
		clearQSO();
		FL_AWAKE();
	}
	restoreFocus();
}

void qsoSave_cb(Fl_Widget *b, void *)
{
	submit_log();
	restoreFocus();
}

void cb_QRZ(Fl_Widget *b, void *)
{
	QRZquery();
}

void status_cb(Fl_Widget *b, void *arg)
{
        if (Fl::event_button() == FL_RIGHT_MOUSE) {
				progdefaults.loadDefaults();
                tabsConfigure->value(tabModems);
                tabsModems->value(modem_config_tab);
                dlgConfig->show();
        }
        else {
                if (!quick_change)
                        return;
                const Fl_Menu_Item *m;
                m = quick_change->popup(Fl::event_x(),
                                        Fl::event_y(), 0, 0, 0);
                if (m && m->callback_)
                        m->do_callback(0);
        }
}

void cb_cboBand(Fl_Widget *w, void *d) 
{
	Fl_ComboBox *cbBox = (Fl_ComboBox *) w;
	wf->rfcarrier(atoi(cbBox->value())*1000L);
}

void afconoff_cb(Fl_Widget *w, void *vi)
{
	FL_LOCK_D();
	Fl_Button *b = (Fl_Button *)w;
//	Fl_Light_Button *b = (Fl_Light_Button *)w;
	int v = b->value();
	FL_UNLOCK_D();
	active_modem->set_afcOnOff(v);
	progdefaults.afconoff = v;
}

void sqlonoff_cb(Fl_Widget *w, void *vi)
{
	FL_LOCK_D();
	Fl_Button *b = (Fl_Button *)w;
//	Fl_Light_Button *b = (Fl_Light_Button *)w;
	int v = b->value();
	FL_UNLOCK_D();
	active_modem->set_sqlchOnOff( v ? true : false );
	progdefaults.sqlonoff = v ? true : false;
}


void cb_btnSideband(Fl_Widget *w, void *d)
{
	Fl_Button *b = (Fl_Button *)w;
	FL_LOCK_D();
	progdefaults.btnusb = !progdefaults.btnusb;
	if (progdefaults.btnusb) { 
		b->label("U");
		wf->USB(true);
	} else {
		b->label("L");
		wf->USB(false);
	}
	b->redraw();
	FL_UNLOCK_D();
}

void cbMacroTimerButton(Fl_Widget *w, void *d)
{
	Fl_Button *b = (Fl_Button *)w;
	progdefaults.useTimer = false;
	FL_LOCK_D();
	b->hide();
	FL_UNLOCK_D();
	restoreFocus();
}

void cb_RcvMixer(Fl_Widget *w, void *d)
{
	progdefaults.RcvMixer = valRcvMixer->value();
	mixer.setRcvGain(progdefaults.RcvMixer);
}

void cb_XmtMixer(Fl_Widget *w, void *d)
{
	progdefaults.XmtMixer = valXmtMixer->value();
	mixer.setXmtLevel(progdefaults.XmtMixer);
}


// XPM Calendar Label
static const char *cal_16[] = {
// width height num_colors chars_per_pixel
"    15    14        3            1",
// colors
". c #000000",
"d c none",
"e c #ffffff",
// pixels
"ddddddddddddddd",
"d.............d",
"d.eeeeeeeeeee.d",
"d.............d",
"d.e.e.e.e.e.e.d",
"d.............d",
"d.e.e.e.e.e.e.d",
"d.............d",
"d.e.e.e.e.e.e.d",
"d.............d",
"d.e.e.e.e.e.e.d",
"d.............d",
"d.e.e.e.e.e.e.d",
"ddddddddddddddd",
};


Fl_Menu_Item menu_[] = {
{"&Files", 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{"Open Macros", 0,  (Fl_Callback*)cb_mnuOpenMacro, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Save Macros", 0,  (Fl_Callback*)cb_mnuSaveMacro, 0, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{"Log File", 0, (Fl_Callback*)cb_mnuLogFile, 0, FL_MENU_DIVIDER | FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
#if USE_SNDFILE
{"Audio", 0, 0, 0, FL_MENU_DIVIDER | FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{"Rx capture",  0, (Fl_Callback*)cb_mnuCapture,  0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},//70
{"Tx generate", 0, (Fl_Callback*)cb_mnuGenerate, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},//71
{"Playback",    0, (Fl_Callback*)cb_mnuPlayback, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},//72
{0,0,0,0,0,0,0,0,0},
#endif
{"E&xit", 0,  (Fl_Callback*)cb_E, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
{"Op &Mode", 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_CW].name, 0, cb_init_mode, (void *)MODE_CW, 0, FL_NORMAL_LABEL, 0, 14, 0},

{"DominoEX", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX4].name, 0, cb_init_mode, (void *)MODE_DOMINOEX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX5].name, 0, cb_init_mode, (void *)MODE_DOMINOEX5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX8].name, 0, cb_init_mode, (void *)MODE_DOMINOEX8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX16].name, 0, cb_init_mode, (void *)MODE_DOMINOEX16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Hell", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FELDHELL].name, 0, cb_init_mode, (void *)MODE_FELDHELL, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FSKHELL].name, 0, cb_init_mode, (void *)MODE_FSKHELL, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FSKH105].name, 0, cb_init_mode, (void *)MODE_FSKH105, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"MFSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK8].name, 0,  cb_init_mode, (void *)MODE_MFSK8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK16].name, 0,  cb_init_mode, (void *)MODE_MFSK16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"PSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_BPSK31].name, 0, cb_init_mode, (void *)MODE_BPSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK31].name, 0, cb_init_mode, (void *)MODE_QPSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK63].name, 0, cb_init_mode, (void *)MODE_PSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK63].name, 0, cb_init_mode, (void *)MODE_QPSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK125].name, 0, cb_init_mode, (void *)MODE_QPSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
#ifdef USE250
{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK250].name, 0, cb_init_mode, (void *)MODE_QPSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
#endif
{0,0,0,0,0,0,0,0,0},

{ mode_info[MODE_OLIVIA].name, 0, cb_init_mode, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_RTTY].name, 0, cb_init_mode, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},

{"Throb", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB1].name, 0, cb_init_mode, (void *)MODE_THROB1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB2].name, 0, cb_init_mode, (void *)MODE_THROB2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB4].name, 0, cb_init_mode, (void *)MODE_THROB4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX1].name, 0, cb_init_mode, (void *)MODE_THROBX1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX2].name, 0, cb_init_mode, (void *)MODE_THROBX2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX4].name, 0, cb_init_mode, (void *)MODE_THROBX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ mode_info[MODE_WWV].name, 0, cb_init_mode, (void *)MODE_WWV, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_ANALYSIS].name, 0, cb_init_mode, (void *)MODE_ANALYSIS, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Configure", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{"Defaults",  0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{"Fonts", 0, (Fl_Callback*)cb_mnuConfigFonts, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Operator", 0, (Fl_Callback*)cb_mnuConfigOperator, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Waterfall", 0,  (Fl_Callback*)cb_mnuConfigWaterfall, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Video", 0,  (Fl_Callback*)cb_mnuConfigVideo, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Rig Ctrl", 0, (Fl_Callback*)cb_mnuConfigRigCtrl, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"QRZ", 0,  (Fl_Callback*)cb_mnuConfigQRZ, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Sound Card", 0, (Fl_Callback*)cb_mnuConfigSoundCard, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Misc", 0,  (Fl_Callback*)cb_mnuConfigMisc, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
{"Modems", 0, (Fl_Callback*)cb_mnuConfigModems, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"Save Config", 0, (Fl_Callback*)cb_mnuSaveConfig, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
{"     ", 0, 0, 0, FL_MENU_INACTIVE, FL_NORMAL_LABEL, 0, 14, 0},
{"Rig", 0, (Fl_Callback*)cb_mnuRig, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{"     ", 0, 0, 0, FL_MENU_INACTIVE, FL_NORMAL_LABEL, 0, 14, 0},
{"Help", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{"About", 0, (Fl_Callback*)cb_mnuAbout, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
{"  ", 0, 0, 0, FL_MENU_INACTIVE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
};

Fl_Menu_Bar *mnu;

Fl_Menu_Item sample_rate_menu[] = {
	{ "Auto" }, { "Native", 0, 0, 0, FL_MENU_DIVIDER },
	{ "8000" }, { "9600" }, { "11025" }, { "12000" }, { "16000" },
	{ "22050" }, { "24000" }, { "32000" }, { "44100" }, { "48000" },
	{ "88200" }, { "96000" }, { "192000" }, { 0 }
};

void activate_rig_menu_item(bool b)
{
	Fl_Menu_Item *rig = 0;
	for (size_t i = 0; i < sizeof(menu_)/sizeof(menu_[0]); i++) {
		if (menu_[i].text && !strcmp(menu_[i].text, "Rig")) {
			rig = menu_ + i;
			break;
		}
	}
	if (!rig) {
		cerr << "FIXME: could not find Rig menu\n";
		return;
	}

	if (b) {
		bSaveFreqList = true;
		rig->activate();
		
	} else {
		rig->deactivate();
		if (rigcontrol)
			rigcontrol->hide();
	}
	mnu->redraw();
}

void make_pixmap(Pixmap *xpm, const char **data)
{
	// We need a displayed window to provide a GC for X_CreatePixmap
	Fl_Window w(0, 0, PACKAGE_NAME);
	w.xclass(PACKAGE_NAME);
	w.show();
	w.make_current();

	Fl_Pixmap icon(data);
	int maxd = max(icon.w(), icon.h());
	*xpm = fl_create_offscreen(maxd, maxd);

	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

int rightof(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->x() + w->w();

	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int lw = static_cast<int>(ceil(fl_width(w->label())));

	if (a & (FL_ALIGN_TOP | FL_ALIGN_BOTTOM)) {
		if (a & FL_ALIGN_LEFT)
			return w->x() + MAX(w->w(), lw);
		else if (a & FL_ALIGN_RIGHT)
			return w->x() + w->w();
		else
			return w->x() + ((lw > w->w()) ? (lw - w->w())/2 : w->w());
	}
	else
		return w->x() + w->w() + lw;
}

int leftof(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->x();

	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int lw = static_cast<int>(ceil(fl_width(w->label())));

	if (a & (FL_ALIGN_TOP | FL_ALIGN_BOTTOM)) {
		if (a & FL_ALIGN_LEFT)
			return w->x();
		else if (a & FL_ALIGN_RIGHT)
			return w->x() - (lw > w->w() ? lw - w->w() : 0);
		else
			return w->x() - (lw > w->w() ? (lw - w->w())/2 : 0);
	}
	else {
		if (a & FL_ALIGN_LEFT)
			return w->x() - lw;
		else
			return w->x();
	}
}

int above(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->y();

	return (a & FL_ALIGN_TOP) ? w->y() + FL_NORMAL_SIZE : w->y();
}

int below(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->y() + w->h();

	return (a & FL_ALIGN_BOTTOM) ? w->y() + w->h() + FL_NORMAL_SIZE : w->y() + w->h();
}

void create_fl_digi_main() {
	int pad = wSpace, Y = 0;
	fl_digi_main = new Fl_Double_Window(WNOM, HNOM, "fldigi");
			mnu = new Fl_Menu_Bar(0, 0, WNOM - 142, Hmenu);
			// FL_NORMAL_SIZE may have changed; update the menu items
			for (size_t i = 0; i < sizeof(menu_)/sizeof(menu_[0]); i++)
				if (menu_[i].text)
					menu_[i].labelsize_ = FL_NORMAL_SIZE;
			mnu->menu(menu_);

			btnTune = new Fl_Button(WNOM - 142, 0, 60, Hmenu, "TUNE");
			btnTune->type(FL_TOGGLE_BUTTON);
			btnTune->callback(cbTune, 0);
			
			btnMacroTimer = new Fl_Button(WNOM - 80 - pad, 0, 80, Hmenu);
			btnMacroTimer->color(fl_rgb_color(255, 255, 100));
			btnMacroTimer->labelcolor(FL_RED);
			btnMacroTimer->callback(cbMacroTimerButton, 0);
			btnMacroTimer->hide();
			Fl_Box *bx = new Fl_Box(WNOM - 80 - pad, 0, 80, Hmenu,"");
			bx->box(FL_UP_BOX);
			
		
		Y += Hmenu;

		Fl_Group *qsoFrame = new Fl_Group(0, Y, WNOM, Hqsoframe);
			inpFreq = new Fl_Input(pad, Y + Hqsoframe/2 - pad, 85, Hqsoframe/2, "Frequency");
			inpFreq->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

			inpTime = new Fl_Input(rightof(inpFreq) + pad, Y + Hqsoframe/2 - pad, 45, Hqsoframe/2, "Time");
			inpTime->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

			qsoTime = new Fl_Button(rightof(inpTime) + pad, Y + Hqsoframe/2 - pad, 24, Hqsoframe/2);
			Fl_Image *pixmap = new Fl_Pixmap(cal_16);
			qsoTime->image(pixmap);
			qsoTime->callback(qsoTime_cb, 0);

			inpCall = new Fl_Input(rightof(qsoTime) + pad, Y + Hqsoframe/2 - pad, 80, Hqsoframe/2, "Call");
			inpCall->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
			// this is likely to be more readable in a constant width font
			inpCall->textfont(FL_SCREEN);

			inpName = new Fl_Input(rightof(inpCall) + pad, Y + Hqsoframe/2 - pad, 100, Hqsoframe/2, "Name");
			inpName->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

			inpRstIn = new Fl_Input(rightof(inpName) + pad, Y + Hqsoframe/2 - pad, 40, Hqsoframe/2, "RST(r)");
			inpRstIn->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

			inpRstOut = new Fl_Input(rightof(inpRstIn) + pad, Y + Hqsoframe/2 - pad, 40, Hqsoframe/2, "RST(s)");
			inpRstOut->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

			btnQRZ = new Fl_Button(WNOM - 40 - pad, Y + 1, 40, Hqsoframe/2 - pad, "QRZ");
			btnQRZ->callback(cb_QRZ, 0);

			inpLoc = new Fl_Input(leftof(btnQRZ) - pad - 80, Y + Hqsoframe/2 - pad, 80, Hqsoframe/2, "Locator");
			inpLoc->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
			inpLoc->textfont(FL_SCREEN);

			inpQth = new Fl_Input(rightof(inpRstOut) + pad, Y + Hqsoframe/2 - pad,
					      leftof(inpLoc) - rightof(inpRstOut) - 2*pad, Hqsoframe/2, "QTH");
			inpQth->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
			qsoFrame->resizable(inpQth);

			qsoClear = new Fl_Button(WNOM - 40 - pad, Y + Hqsoframe/2 + 1, 40, Hqsoframe/2 - pad, "Clear");
			qsoClear->callback(qsoClear_cb, 0);

		qsoFrame->end();
		Y += Hqsoframe;

		Fl_Group *qsoFrame2 = new Fl_Group(0,Y, WNOM, Hnotes);
			qsoSave = new Fl_Button(WNOM - 40 - pad, Y + 1, 40, Hnotes - 2, "Save");
			qsoSave->callback(qsoSave_cb, 0);

			inpAZ = new Fl_Input(leftof(qsoSave) - 80 - pad, Y, 80, Hnotes, "AZ"); // WA5ZNU
			inpAZ->align(FL_ALIGN_LEFT);

			// align this vertically with the Call field
			inpNotes = new Fl_Input(leftof(inpCall), Y, leftof(inpAZ) - leftof(inpCall) - 2*pad, Hnotes, "Notes");
			inpNotes->align(FL_ALIGN_LEFT);
			qsoFrame2->resizable(inpNotes);

			btnSideband = new Fl_Button(leftof(inpNotes) - 2*pad - (Hnotes-2), Y+1, Hnotes-2, Hnotes-2, "U");
			btnSideband->callback(cb_btnSideband, 0);
			btnSideband->hide();
			cboBand	 = new Fl_ComboBox(pad, Y, leftof(btnSideband) - pad, Hnotes, "");
			cboBand->hide();
		qsoFrame2->end();
		Y += Hnotes;
		
		int sw = 15;
		Fl_Group *MixerFrame = new Fl_Group(0,Y,sw, Hrcvtxt + Hxmttxt);
			valRcvMixer = new Fl_Slider(0, Y, sw, (Htext)/2, "");
			valRcvMixer->type(FL_VERT_NICE_SLIDER);
			valRcvMixer->color(fl_rgb_color(0,110,30));
			valRcvMixer->labeltype(FL_ENGRAVED_LABEL);
			valRcvMixer->selection_color(fl_rgb_color(255,255,0));
			valRcvMixer->range(1.0,0.0);
			valRcvMixer->value(1.0);
			valRcvMixer->callback( (Fl_Callback *)cb_RcvMixer);
			valXmtMixer = new Fl_Slider(0, Y + (Htext)/2, sw, (Htext)/2, "");
			valXmtMixer->type(FL_VERT_NICE_SLIDER);
			valXmtMixer->color(fl_rgb_color(110,0,30));
			valXmtMixer->labeltype(FL_ENGRAVED_LABEL);
			valXmtMixer->selection_color(fl_rgb_color(255,255,0));
			valXmtMixer->range(1.0,0.0);
			valXmtMixer->value(1.0);
			valXmtMixer->callback( (Fl_Callback *)cb_XmtMixer);
			valRcvMixer->deactivate();
			valXmtMixer->deactivate();
		MixerFrame->end();

		TiledGroup = new Fl_Tile_check(sw, Y, WNOM-sw, Htext);
            int minRxHeight = Hrcvtxt;
            int minTxHeight;
            if (minRxHeight < 66) minRxHeight = 66;
            minTxHeight = Htext - minRxHeight;

			Fl_Box *minbox = new Fl_Box(sw,Y + 66, WNOM-sw, Htext - 66 - 32);
			minbox->hide();

			if (progdefaults.alt_text_widgets)
				ReceiveText = new FTextView(sw, Y, WNOM-sw, minRxHeight, "");
			else
				ReceiveText = new TextView(sw, Y, WNOM-sw, minRxHeight, "");
		
			FHdisp = new Raster(sw, Y, WNOM-sw, minRxHeight);
			FHdisp->hide();
			Y += minRxHeight;

			if (progdefaults.alt_text_widgets)
				TransmitText = new FTextEdit(sw, Y, WNOM-sw, minTxHeight);
			else
				TransmitText = new TextEdit(sw, Y, WNOM-sw, minTxHeight);
			Y += minTxHeight;

			TiledGroup->resizable(minbox);
		TiledGroup->end();
		Fl_Group::current()->resizable(TiledGroup);

		
		Fl_Box *macroFrame = new Fl_Box(0, Y, WNOM, Hmacros);
			macroFrame->box(FL_ENGRAVED_FRAME);
			int Wbtn = (WNOM - 30 - 8 - 4)/10;
			int xpos = 2;
			for (int i = 0; i < 10; i++) {
				if (i == 4 || i == 8) {
					bx = new Fl_Box(xpos, Y+2, 5, Hmacros - 4);
					bx->box(FL_FLAT_BOX);
					bx->color(FL_BLACK);
					xpos += 4;
				}
				btnMacro[i] = new Fl_Button(xpos, Y+2, Wbtn, Hmacros - 4);
				btnMacro[i]->callback(macro_cb, (void *)i);
				btnMacro[i]->label( (macros.name[i]).c_str());
				xpos += Wbtn;
			}
			bx = new Fl_Box(xpos, Y+2, WNOM - 32 - xpos, Hmacros - 4);
			bx->box(FL_FLAT_BOX);
			bx->color(FL_BLACK);
			btnAltMacros = new Fl_Button(WNOM-32, Y+2, 30, Hmacros - 4, "Alt");
			btnAltMacros->callback(altmacro_cb, 0);
				
		Y += Hmacros;

		Fl_Pack *wfpack = new Fl_Pack(0, Y, WNOM, Hwfall);
			wfpack->type(1);
			wf = new waterfall(0, Y, Wwfall, Hwfall);
			wf->end();
			Fl_Pack *ypack = new Fl_Pack(WNOM-(Hwfall-24), Y, Hwfall-24, Hwfall);
				ypack->type(0);

				digiscope = new Digiscope (WNOM-(Hwfall-24), Y, Hwfall-24, Hwfall-24);
	
				pgrsSquelch = new Fl_Progress(
					WNOM-(Hwfall-24), Y + Hwfall - 24,
					Hwfall - 24, 12, "");
				pgrsSquelch->color(FL_BACKGROUND2_COLOR, FL_DARK_GREEN);
				sldrSquelch = new Fl_Slider(
					FL_HOR_NICE_SLIDER, 
					WNOM-(Hwfall-24), Y + Hwfall - 12, 
					Hwfall - 24, 12, "");
							
				sldrSquelch->minimum(0);
				sldrSquelch->maximum(100);
				sldrSquelch->step(1);
				sldrSquelch->value(progdefaults.sldrSquelchValue);
				sldrSquelch->callback((Fl_Callback*)cb_sldrSquelch);
				sldrSquelch->color(FL_INACTIVE_COLOR);

			ypack->end();
			Fl_Group::current()->resizable(wf);
		wfpack->end();
		Y += (Hwfall + 2);

		Fl_Pack *hpack = new Fl_Pack(0, Y, WNOM, Hstatus);
			hpack->type(1);
			MODEstatus = new Fl_Button(0,Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Wmode, Hstatus, "");
			MODEstatus->box(FL_DOWN_BOX);
			MODEstatus->color(FL_BACKGROUND2_COLOR);
			MODEstatus->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			MODEstatus->callback(status_cb, (void *)0);
			MODEstatus->when(FL_WHEN_CHANGED);
			MODEstatus->tooltip("Left clk - change mode\nRight clk - Modem Tab");
			Status1 = new Fl_Box(Wmode,Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Ws2n, Hstatus, "");
			Status1->box(FL_DOWN_BOX);
			Status1->color(FL_BACKGROUND2_COLOR);
			Status1->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			Status2 = new Fl_Box(Wmode+Ws2n, Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Wimd, Hstatus, "");
			Status2->box(FL_DOWN_BOX);
			Status2->color(FL_BACKGROUND2_COLOR);
			Status2->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			StatusBar = new Fl_Box(
                Wmode+Wimd+Ws2n, Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Wstatus, Hstatus, "");
			StatusBar->box(FL_DOWN_BOX);
			StatusBar->color(FL_BACKGROUND2_COLOR);
			StatusBar->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			WARNstatus = new Fl_Box(
                Wmode+Wimd+Ws2n+Wstatus, Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
                Wwarn, Hstatus, "");
			WARNstatus->box(FL_DIAMOND_DOWN_BOX);
			WARNstatus->color(FL_BACKGROUND_COLOR);
			WARNstatus->labelcolor(FL_RED);
			WARNstatus->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

			if (useCheckButtons) {
				chk_afconoff = new Fl_Check_Button(
								WNOM - bwAfcOnOff - bwSqlOnOff, 
								Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
								bwAfcOnOff, Hstatus, "Afc");
				chk_afconoff->callback(afconoff_cb, 0);
				chk_afconoff->value(1);
				chk_afconoff->tooltip("AFC on/off");
				chk_sqlonoff = new Fl_Check_Button(
								WNOM - bwSqlOnOff, 
								Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
								bwSqlOnOff, Hstatus, "Sql");
				chk_sqlonoff->callback(sqlonoff_cb, 0);
				chk_sqlonoff->value(1);
				chk_sqlonoff->tooltip("SQL on/off");
			} else {
				afconoff = new Fl_Light_Button(
								WNOM - bwAfcOnOff - bwSqlOnOff, 
								Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
								bwAfcOnOff, Hstatus, "Afc");
				afconoff->callback(afconoff_cb, 0);
				afconoff->value(1);
				afconoff->tooltip("AFC on/off");
				sqlonoff = new Fl_Light_Button(
								WNOM - bwSqlOnOff, 
								Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
								bwSqlOnOff, Hstatus, "Sql");
				sqlonoff->callback(sqlonoff_cb, 0);
				sqlonoff->value(1);
				sqlonoff->tooltip("SQL on/off");
			}
				
			Fl_Group::current()->resizable(StatusBar);
		hpack->end();

		fl_digi_main->size_range(WNOM, HNOM);
	fl_digi_main->end();
	fl_digi_main->callback(cb_wMain);

	make_pixmap(&fldigi_icon_pixmap, fldigi_icon_48_xpm);
	fl_digi_main->icon((char *)fldigi_icon_pixmap);

	fl_digi_main->xclass(PACKAGE_NAME);
}

void put_freq(double frequency)
{
	wf->carrier((int)floor(frequency + 0.5));
}

void put_Bandwidth(int bandwidth)
{
	wf->Bandwidth ((int)bandwidth);
}

void display_metric(double metric)
{
	FL_LOCK_D();
	REQ(static_cast<void (Fl_Progress::*)(float)>(&Fl_Progress::value), pgrsSquelch, metric);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_cwRcvWPM(double wpm)
{
	int U = progdefaults.CWupperlimit;
	int L = progdefaults.CWlowerlimit;
	double dWPM = 100.0*(wpm - L)/(U - L);
	FL_LOCK_D();
	REQ(static_cast<void (Fl_Progress::*)(float)>(&Fl_Progress::value), prgsCWrcvWPM, dWPM);
	REQ(static_cast<int (Fl_Value_Output::*)(double)>(&Fl_Value_Output::value), valCWrcvWPM, (int)wpm);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void set_scope(double *data, int len, bool autoscale)
{
	if (digiscope)
		digiscope->data(data, len, autoscale);
}

void set_phase(double phase, bool highlight)
{
	if (digiscope)
		digiscope->phase(phase, highlight);
}

void set_rtty(double flo, double fhi, double amp)
{
	if (digiscope)
		digiscope->rtty(flo, fhi, amp);
}

void set_video(double *data, int len)
{
	if (digiscope)
		digiscope->video(data, len);
}

void put_rx_char(unsigned int data)
{
	static unsigned int last = 0;
	const char **asc = ascii;
	rxmsgid = msgget( (key_t) progdefaults.rx_msgid, 0666);
	if (mailclient || mailserver || rxmsgid != -1)
		asc = ascii2;
	if (active_modem->get_mode() == MODE_RTTY ||
		active_modem->get_mode() == MODE_CW)
		asc = ascii;

	int style = ReceiveWidget::RECV;
	if (asc == ascii2 && iscntrl(data))
		style = ReceiveWidget::CTRL;
	if (wf->tmp_carrier())
		style = ReceiveWidget::ALTR;

	data &= 0x7F;
	switch (data) {
		case '\n':
			if (last == '\r')
				break;
		case '\r':
			REQ(&ReceiveWidget::addchr, ReceiveText, '\n', style);
			break;
		default:
			REQ(&ReceiveWidget::addchr, ReceiveText, data, style);
	}
	last = data;

	if ( rxmsgid != -1) {
		rxmsgst.msg_type = 1;
		rxmsgst.c = data;
		msgsnd (rxmsgid, (void *)&rxmsgst, 1, IPC_NOWAIT);
	}

	if (Maillogfile)
		Maillogfile->log_to_file(cLogfile::LOG_RX, ascii2[data]);

	if (logging)
		logfile->log_to_file(cLogfile::LOG_RX, ascii2[data]);
}

string strSecText = "";

void put_sec_char( char chr )
{
	if (chr >= ' ' && chr <= 'z') {
		strSecText.append(1, chr);
		if (strSecText.length() > 60)
			strSecText.erase(0,1);
		FL_LOCK_D();
		REQ(static_cast<void (Fl_Box::*)(const char *)>(&Fl_Box::label), StatusBar, strSecText.c_str());
		FL_UNLOCK_D();
		FL_AWAKE_D();
	}
}

void clear_status_cb(void *)
{
	StatusBar->label("");
}

void put_status(const char *msg, double timeout)
{
	static char m[60];
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	FL_LOCK_D();
	REQ(static_cast<void (Fl_Box::*)(const char *)>(&Fl_Box::label), StatusBar, m);
	// While it is safe to call to use Fl::add_timeout without qrunner
	// regardless of our caller's context, queuing ensures that clear_status_cb
	// really gets called at least ``timeout'' seconds after the label is set.
	if (timeout > 0 && !Fl::has_timeout(clear_status_cb)) { // clear after timeout
		Fl::remove_timeout(clear_status_cb);
		REQ(&Fl::add_timeout, timeout, clear_status_cb, (void*)0);
	}
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_Status2(const char *msg)
{
	static char m[60];
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	FL_LOCK_D();
	REQ(static_cast<void (Fl_Box::*)(const char *)>(&Fl_Box::label), Status2, m);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_Status1(const char *msg)
{
	static char m[60];
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	FL_LOCK_D();
	REQ(static_cast<void (Fl_Box::*)(const char *)>(&Fl_Box::label), Status1, m);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}


void put_WARNstatus(double val)
{
	FL_LOCK_D();
	if (val < 0.05)
		WARNstatus->color(FL_BLACK);
    if (val > 0.05)
        WARNstatus->color(FL_DARK_GREEN);
    if (val > 0.9)
        WARNstatus->color(FL_YELLOW);
    if (val > 0.98)
        WARNstatus->color(FL_DARK_RED);
	WARNstatus->redraw();
	FL_UNLOCK_D();
}


void set_CWwpm()
{
	FL_LOCK();
	sldrCWxmtWPM->value(progdefaults.CWspeed);
	FL_UNLOCK();
}

void clear_StatusMessages()
{
	FL_LOCK_E();
	StatusBar->label("");
	Status1->label("");
	Status2->label("");
	FL_UNLOCK_E();
	FL_AWAKE_E();
}

	
void put_MODEstatus(trx_mode mode)
{
	FL_LOCK_D();
	REQ(static_cast<void (Fl_Button::*)(const char *)>(&Fl_Button::label), MODEstatus, mode_info[mode].sname);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_rx_data(int *data, int len)
{
 	FHdisp->data(data, len);
}

int get_tx_char(void)
{
	if (pskmail_text_available)
		return pskmail_get_char();

	int c;
	static int pending = -1;
	if (pending >= 0) {
		c = pending;
		pending = -1;
		return c;
	}

	enum { STATE_CHAR, STATE_CTRL };
	static int state = STATE_CHAR;

	switch (c = TransmitText->nextChar()) {
	case '\n':
		pending = '\n';
		return '\r';
	case '^':
		if (state == STATE_CTRL)
			break;
		state = STATE_CTRL;
		return -1;
	case 'r': case 'R':
		if (state != STATE_CTRL)
			break;
		REQ_SYNC(&TransmitWidget::clear_sent, TransmitText);
		state = STATE_CHAR;
		c = 3; // ETX
		break;
	case -1:
		break;
	default:
		if (state == STATE_CTRL) {
			state = STATE_CHAR;
			pending = c;
			return '^';
		}
	}

	pending = -1;
	return c;
}

void put_echo_char(unsigned int data)
{
	static unsigned int last = 0;
	const char **asc = ascii;
	
	if (mailclient || mailserver || arqmode)
		asc = ascii2;
	if (active_modem->get_mode() == MODE_RTTY ||
		active_modem->get_mode() == MODE_CW)
		asc = ascii;

	data &= 0x7F;
	if (data == '\r' && last == '\r') // reject multiple CRs
		return;

	last = data;

	int style = ReceiveWidget::XMIT;
	if (asc == ascii2 && iscntrl(data))
		style = ReceiveWidget::CTRL;
	REQ(&ReceiveWidget::addchr, ReceiveText, data, style);

	if (Maillogfile)
		Maillogfile->log_to_file(cLogfile::LOG_TX, ascii2[data & 0x7F]);
	if (logging)
		logfile->log_to_file(cLogfile::LOG_TX, ascii2[data & 0x7F]);
}

void resetRTTY() {
	if (active_modem->get_mode() != MODE_RTTY) return;
	trx_reset(scDevice.c_str());
	active_modem->restart();
}

void resetOLIVIA() {
	if (active_modem->get_mode() != MODE_OLIVIA) return;
	trx_reset(scDevice.c_str());
	active_modem->restart();
}

void resetDOMEX() {
	trx_mode md = active_modem->get_mode();
	if (md == MODE_DOMINOEX4 ||
		md == MODE_DOMINOEX5 ||
		md == MODE_DOMINOEX8 ||
		md == MODE_DOMINOEX11 ||
		md == MODE_DOMINOEX16 ||
		md == MODE_DOMINOEX22 ) {

		trx_reset(scDevice.c_str());
		active_modem->restart();
	}
}

void enableMixer(bool on)
{
	FL_LOCK_D();
	if (on) {
		progdefaults.EnableMixer = true;
		mixer.openMixer(progdefaults.MXdevice.c_str());

		mixer.PCMVolume(progdefaults.PCMvolume);
		mixer.setXmtLevel(valXmtMixer->value());
		mixer.setRcvGain(valRcvMixer->value());
		if (progdefaults.LineIn == true)
			setMixerInput(1);
		else if (progdefaults.MicIn == true)
			setMixerInput(2);
		else
			setMixerInput(0);
	}else{
		progdefaults.EnableMixer = false;
		mixer.closeMixer();
	}
        resetMixerControls();
	FL_UNLOCK_D();
}

void resetMixerControls()
{
    if (progdefaults.EnableMixer) {
	    valRcvMixer->activate();
	    valXmtMixer->activate();
	    menuMix->activate();
	    btnLineIn->activate();
	    btnMicIn->activate();
        btnMixer->value(1);
	    valPCMvolume->activate();
    }
    else {
	    valRcvMixer->deactivate();
	    valXmtMixer->deactivate();
	    menuMix->deactivate();
	    btnLineIn->deactivate();
	    btnMicIn->deactivate();
        btnMixer->value(0);
	    valPCMvolume->deactivate();
    }
}

void setPCMvolume(double vol)
{
	mixer.PCMVolume(vol);
	progdefaults.PCMvolume = vol;
}

void setMixerInput(int dev)
{
	int n= -1;
	switch (dev) {
		case 0: n = mixer.InputSourceNbr("Vol");
				break;
		case 1: n = mixer.InputSourceNbr("Line");
				break;
		case 2: n = mixer.InputSourceNbr("Mic");
				break;
		default: n = mixer.InputSourceNbr("Vol");
	}
	if (n != -1)
		mixer.SetCurrentInputSource(n);
}

void resetSoundCard()
{
    bool mixer_enabled = progdefaults.EnableMixer;
	enableMixer(false);
	trx_reset(scDevice.c_str());
	progdefaults.SCdevice = scDevice;
    if (mixer_enabled)
        enableMixer(true);
}

void setReverse(int rev) {
	active_modem->set_reverse(rev);
}

/*
void setAfcOnOff(bool b) {
	FL_LOCK();
	afconoff->value(b);
	FL_UNLOCK();
	FL_AWAKE();
}	

void setSqlOnOff(bool b) {
	FL_LOCK();
	sqlonoff->value(b);
	FL_UNLOCK();
	FL_AWAKE();
}

bool QueryAfcOnOff() {
	FL_LOCK_E();
	int v = afconoff->value();
	FL_UNLOCK_E();
	return v;
}

bool QuerySqlOnOff() {
	FL_LOCK_E();
	int v = sqlonoff->value();
	FL_UNLOCK_E();
	return v;
}
*/

void change_modem_param(int state)
{
	int d;
	if ( !((d = Fl::event_dy()) || (d = Fl::event_dx())) )
		return;

	if (state & (FL_META | FL_ALT)) {
		if (d > 0)
			active_modem->searchUp();
		else if (d < 0)
			active_modem->searchDown();
		return;
	}
	if (!(state & (FL_CTRL | FL_SHIFT)))
		return; // suggestions?

	Fl_Valuator *val = 0;
	if (state & FL_CTRL) {
		switch (active_modem->get_mode()) {
		case MODE_BPSK31: case MODE_QPSK31: case MODE_PSK63: case MODE_QPSK63:
		case MODE_PSK125: case MODE_QPSK125: case MODE_PSK250: case MODE_QPSK250:
			val = mailserver ? cntServerOffset : cntSearchRange;
			break;
		case MODE_FELDHELL:
			val = sldrHellBW;
			break;
		case MODE_CW:
			val = sldrCWbandwidth;
			break;
		default:
			return;
		}
	}
	else if (state & FL_SHIFT)
		val = sldrSquelch;

	val->value(val->clamp(val->increment(val->value(), -d)));
	bool changed_save = progdefaults.changed;
	val->do_callback();
	progdefaults.changed = changed_save;
	if (val == cntServerOffset || val == cntSearchRange)
		active_modem->set_sigsearch(SIGSEARCH);
	else if (val == sldrSquelch) // sldrSquelch gives focus to TextWidget
		wf->take_focus();

	char msg[60];
	if (val != sldrSquelch)
		snprintf(msg, sizeof(msg), "%s = %2.0f Hz", val->label(), val->value());
	else
		snprintf(msg, sizeof(msg), "Squelch = %2.0f %%", val->value());
	put_status(msg, 2);
}
