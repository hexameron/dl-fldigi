// ----------------------------------------------------------------------------
// nanoIO.cxx  --  Interface to Arduino Nano keyer
//
// Copyright (C) 2018
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------


#include <config.h>

#include "fl_digi.h"
#include "nanoIO.h"
#include "serial.h"
#include "morse.h"

#include "strutil.h"

#define LOG_TINYFSK  LOG_INFO

using namespace std;

Cserial nano_serial;

bool use_nanoIO = false;
bool nanoIO_isCW = false;

static cMorse *nano_morse = 0;

// use during debugging
void sent(std::string s)
{
//	std::cout << "sent:\"" << s << "\"" << std::endl;
}

void rcvd(std::string s)
{
//	std::cout << "rcvd:\"" << s << "\"" << std::endl;
}

void nano_display_io(string s, int style)
{
	if (s.empty()) return;
	REQ(&FTextBase::addstr, txt_nano_io, s, style);
	REQ(&FTextBase::addstr, txt_nano_CW_io, s, style);
}

int nano_serial_write(char c) {
	if (!use_nanoIO) return 1;
	unsigned char buffer[2];
	buffer[0] = c;
	buffer[1] = '\0';
	return nano_serial.WriteBuffer(buffer, 1);
}

char nano_read_byte(int msec)
{
	std::string resp;
	resp.clear();

	resp = nano_serial_read();
	int numtries = msec/100;
	while (resp.empty() && numtries) {
		MilliSleep(100);
		resp = nano_serial_read();
		numtries--;
	}
	if (resp.empty())
		return 0;
	return resp[0];
}

string nano_read_string(int msec_wait, string find)
{
	std::string resp;

	int timer = msec_wait;

	if (!find.empty()) {
		while (timer && (resp.find(find) == std::string::npos)) {
			resp.append(nano_serial_read());
			--timer;
		}
	} else {
		while (timer) {
			resp.append(nano_serial_read());
			--timer;
		}
	}
	return resp;
}

bool nanoIO_busy = false;

void nano_send_char(int c)
{
	if (nanoIO_isCW) {
		if (c == GET_TX_CHAR_NODATA) {
			MilliSleep(50);
			return;
		}
		if (c == 0x0d) return;
		if (c == 0x0a || c == ' ') {
			MilliSleep(4*1200/progdefaults.CWspeed);
		} else {
			if (nano_morse == 0) return;
			if (c == '^' || c == '|') {
				nano_serial_write(c);
				return;
			}
			int len = nano_morse->tx_length(c);
			if (len) {
				nanoIO_busy = true;
				nano_serial_write(c);
				MilliSleep(1200 * len / progdefaults.CWspeed);
				nanoIO_busy = false;
			} else
				return;
		}
		return;
	}

	int charlen = 165; // msec for start + 5 data + 1.5 stop bits @ 45.45

	if (c == GET_TX_CHAR_NODATA) {
		MilliSleep(charlen); // start + 5 data + 1.5 stop bits
		return;
	}

	nano_serial_write(c);

	if (progdefaults.nanoIO_baud == 50.0) charlen = 150;
	if (progdefaults.nanoIO_baud == 75.0) charlen = 100;
	if (progdefaults.nanoIO_baud == 100.0) charlen = 75;

	MilliSleep(charlen); // start + 5 data + 1.5 stop bits

}

void nano_sendString (const std::string &s)
{
	sent(s);

	for (size_t n = 0; n < s.length(); n++)
		nano_serial_write(s[n]);

	return;
}


void nano_PTT(int val)
{
	if (!use_nanoIO) return;

	nano_serial_write(val ? '[' : ']');
}

void nano_cancel_transmit()
{
	nano_serial_write('\\');
}

void nano_mark_polarity(int v)
{
	std::string resp;
	nano_serial_write('~');
	nano_serial_write(
		v == 0 ? '1' :	// MARK high
		'0');			// MARK low
	resp = nano_serial_read();
	nano_display_io(resp, FTextBase::ALTR);
}

void nano_MARK_is(int val)
{
	progdefaults.nanoIO_polarity = val;
	chk_nanoIO_polarity->value(val);
}

void nano_set_baud(int bd)
{
	std::string resp;
	nano_serial_write('~');
	nano_serial_write(
		bd == 3 ? '9' : // 100.0 baud
		bd == 2 ? '7' : // 75.0 baud
		bd == 1 ? '5' : // 50.0 baud
		'4');			// 45.45 baud
	resp = nano_serial_read();
	nano_display_io(resp, FTextBase::ALTR);
}

void nano_baud_is(int val)
{
	int index = 2;
	if (val == 45) index = 0;
	if (val == 50) index = 1;
	if (val == 100) index = 2;
	progdefaults.nanoIO_baud = index;
	sel_nanoIO_baud->index(index);
}

static int pot_min, pot_rng;
static bool nanoIO_has_pot = false;

void init_pot_min_max()
{
	nanoIO_has_pot = true;

	btn_nanoIO_pot->activate();
	nanoIO_use_pot();

	cntr_nanoIO_min_wpm->activate();
	cntr_nanoIO_rng_wpm->activate();

	cntr_nanoIO_min_wpm->value(pot_min);
	cntr_nanoIO_rng_wpm->value(pot_rng);
	cntr_nanoIO_min_wpm->redraw();
	cntr_nanoIO_rng_wpm->redraw();
}

void disable_min_max()
{
	btn_nanoIO_pot->deactivate();
	cntr_nanoIO_min_wpm->deactivate();
	cntr_nanoIO_rng_wpm->deactivate();
}

// this function must be called from within the main UI thread
// use REQ(nano_parse_config, s);

void nano_parse_config(std::string s)
{
	nano_display_io(s, FTextBase::ALTR);

	size_t p1 = 0;
	if (s.find("nanoIO") == std::string::npos) return;

	if (s.find("HIGH") != std::string::npos)  nano_MARK_is(1);
	if (s.find("LOW") != std::string::npos)   nano_MARK_is(0);
	if (s.find("45.45") != std::string::npos) nano_baud_is(45);
	if (s.find("50.0") != std::string::npos)  nano_baud_is(50);
	if (s.find("75.0") != std::string::npos)  nano_baud_is(75);
	if (s.find("100.0") != std::string::npos) nano_baud_is(100);
	if ((p1 = s.find("WPM")) != std::string::npos) {
		p1 += 4;
		int wpm = progdefaults.CWspeed;
		if (sscanf(s.substr(p1).c_str(), "%d", &wpm)) {
			progdefaults.CWspeed = wpm;
			cntCW_WPM->value(wpm);
			cntr_nanoCW_WPM->value(wpm);
			sldrCWxmtWPM->value(wpm);
		}
	}
	if ((p1 = s.find("/", p1)) != std::string::npos) {
		p1++;
		int wpm = progdefaults.CW_keyspeed;
		if (sscanf(s.substr(p1).c_str(), "%d", &wpm)) {
			progdefaults.CW_keyspeed = wpm;
			cntr_nanoCW_paddle_WPM->value(wpm);
		}
	} else { // ver 1.1.x
		if ((p1 = s.find("WPM", p1 + 4)) != std::string::npos) {
			p1++;
			int wpm = progdefaults.CW_keyspeed;
			if (sscanf(s.substr(p1).c_str(), "%d", &wpm)) {
				progdefaults.CW_keyspeed = wpm;
				cntr_nanoCW_paddle_WPM->value(wpm);
			}
		}
	}
	if ((p1 = s.find("dash/dot ")) != std::string::npos) {
		p1 += 9;
		float val = progdefaults.CWdash2dot;
		if (sscanf(s.substr(p1).c_str(), "%f", &val)) {
			progdefaults.CWdash2dot = val;
			cntCWdash2dot->value(val);
			cnt_nanoCWdash2dot->value(val);
		}
	}
	if ((p1 = s.find("PTT")) != std::string::npos) {
		if (s.find("NO", p1 + 4) != std::string::npos)
			progdefaults.disable_CW_PTT = true;
		else
			progdefaults.disable_CW_PTT = false;
	} else
		progdefaults.disable_CW_PTT = false;
	nanoIO_set_cw_ptt();

	if ((p1 = s.find("Speed Pot")) != std::string::npos) {
		size_t p2 = s.find("ON", p1);
		int OK = 0;
		p2 = s.find("minimum", p1);
		if (p2 != std::string::npos)
			OK = sscanf(&s[p2 + 8], "%d", &pot_min);
		p2 = s.find("range", p1);
		if (p2 != std::string::npos)
			OK = sscanf(&s[p2 + 6], "%d", &pot_rng);
		if (OK)
			init_pot_min_max();
	} else
		disable_min_max();

	return;
}

int open_port(std::string PORT)
{
	if (PORT.empty()) return false;

	nano_serial.Device(PORT);
	nano_serial.Baud(9600);
	nano_serial.Timeout(10);
	nano_serial.Retries(5);
	nano_serial.Stopbits(1);

	if (!nano_serial.OpenPort()) {
		nano_display_io("\nCould not open serial port!", FTextBase::ALTR);
		LOG_ERROR("Could not open %s", progdefaults.nanoIO_serial_port_name.c_str());
		return false;
	}

	use_nanoIO = true;

	nano_display_io("Connected to nanoIO\n", FTextBase::RECV);

	return true;
}

std::string nano_serial_read()
{
	static char buffer[4096];

	memset(buffer, '\0',4096);

	int rb = nano_serial.ReadBuffer((unsigned char *)buffer, 4095);
	if (rb)
		return buffer;
	return "";
}

void nano_serial_flush()
{
	static char buffer[1025];
	REQ(rcvd,"nano_serial_flush():");
	while (nano_serial.ReadBuffer((unsigned char *)buffer, 1024) ) ;
}

void no_cmd(void *)
{
	nano_display_io("Could not read current configuration\n", FTextBase::ALTR);
}

void close_nanoIO()
{
	nano_serial.ClosePort();
	use_nanoIO = false;

	nano_display_io("Disconnected from nanoIO\n", FTextBase::RECV);

	if (nano_morse) {
		delete nano_morse;
		nano_morse = 0;
	}

	progStatus.nanoCW_online = false;
	progStatus.nanoFSK_online = false;
	nanoIO_isCW = false;

	enable_rtty_quickchange();

}

bool open_nano()
{
	if (use_nanoIO)
		return true;
	if (!open_port(progdefaults.nanoIO_serial_port_name))
		return false;

	return true;
}

bool open_nanoIO()
{
	std::string rsp;

	progStatus.nanoCW_online = false;
	progStatus.nanoFSK_online = false;

	if (open_nano()) {

		set_nanoIO();

		nano_sendString("~?");
		rsp = nano_read_string(100, "PTT");

		size_t p = rsp.find("~?");
		if (p == std::string::npos) return false;
		rsp.erase(0, p + 3);

		if (rsp.find("eyer") != std::string::npos)
			REQ(nano_parse_config, rsp);

		progStatus.nanoFSK_online = true;
		nanoIO_isCW = false;

		disable_rtty_quickchange();

		return true;
	}
	return false;
}

bool open_nanoCW()
{
	if (nano_morse == 0) nano_morse = new cMorse;

	progStatus.nanoCW_online = false;
	progStatus.nanoFSK_online = false;

	std::string rsp;
	if (open_nano()) {

		set_nanoCW();

		nano_sendString("~?"); 
		rsp = nano_read_string(100, "PTT");

		size_t p = rsp.find("~?");
		if (p == std::string::npos) return false;
		rsp.erase(0, p + 3);

		if (rsp.find("eyer") != std::string::npos)
			REQ(nano_parse_config, rsp); 

		progStatus.nanoCW_online = true;

		return true;
	}
	return false;
}

void set_nanoIO()
{
	if (progStatus.nanoFSK_online) return;

	std::string cmd = "~F";
	std::string rsp;
	int count = 10;
	while (rsp.empty() && count--) {
		nano_sendString(cmd);
		rsp = nano_read_string(10, cmd);
	}
}

void set_nanoCW()
{
	std::string cmd = "~C";
	std::string rsp;
	int count = 10;

	while (rsp.empty() && count--) {
		nano_sendString(cmd);
		rsp = nano_read_string(10, cmd);
	}

	if (rsp.find(cmd) != std::string::npos) {
		nanoIO_isCW = true;
		set_nanoWPM(progdefaults.CWspeed);
		set_nano_dash2dot(progdefaults.CWdash2dot);
	}

}

void set_nanoWPM(int wpm)
{
	static char szwpm[10];
	if (wpm > 60 || wpm < 5) return;
	snprintf(szwpm, sizeof(szwpm), "~S%ds", wpm);
	nano_sendString(szwpm);

	std::string rsp  = nano_read_string(100, szwpm);
	REQ(rcvd,rsp);
}

void set_nano_keyerWPM(int wpm)
{
	static char szwpm[10];
	if (wpm > 60 || wpm < 5) return;
	snprintf(szwpm, sizeof(szwpm), "~U%du", wpm);
	nano_sendString(szwpm);

	std::string rsp  = nano_read_string(100, szwpm);
	REQ(rcvd,rsp);
}

void set_nano_dash2dot(float wt)
{
	static char szd2d[10];
	if (wt < 2.5 || wt > 3.5) return;
	snprintf(szd2d, sizeof(szd2d), "~D%3dd", (int)(wt * 100) );
	nano_sendString(szd2d);

	std::string rsp = nano_read_string(100, szd2d);

	REQ(rcvd,rsp);
}

void nano_CW_query()
{
	nano_serial_flush();
	nano_sendString("~?");
	string resp = nano_read_string(100, "PTT");

	REQ(rcvd,resp);
	nano_display_io(resp, FTextBase::ALTR);
	REQ(nano_parse_config, resp);
}

void nano_help()
{
	nano_serial_flush();
	nano_sendString("~~");
	string resp = nano_read_string(500, "cmds");
	REQ(rcvd,resp);
	nano_display_io(resp, FTextBase::ALTR);
}

void nano_CW_save()
{
	nano_sendString("~W");
	std::string rsp = nano_read_string(100, "~W");
	REQ(rcvd,rsp);
}

void nanoCW_tune(int val)
{
	if (val) nano_sendString("~T");
	else     nano_sendString("]");
}

void set_nanoIO_incr()
{
	std::string s_incr = "~I";
	s_incr += progdefaults.nanoIO_CW_incr;
	nano_sendString(s_incr);
	std::string rsp = nano_read_string(100, s_incr);
	REQ(rcvd,rsp);
}

void set_nanoIO_keyer(int indx)
{
	std::string s;
	if (indx == 0) s = "~A";
	if (indx == 1) s = "~B";
	if (indx == 2) s = "~K";
	nano_sendString(s);
	std::string rsp = nano_read_string(100, s);
	REQ(rcvd,rsp);
}

void nanoIO_set_cw_ptt()
{
	std::string s = "~X";
	s += progdefaults.disable_CW_PTT ? '0' : '1';
	nano_sendString(s);
	std::string rsp = nano_read_string(100, s);
	REQ(rcvd, rsp);
}

void nanoIO_read_pot()
{
	if (!use_nanoIO) return;
	if (!nanoIO_has_pot) return;
	if (!progdefaults.nanoIO_speed_pot) return;
	if (nanoIO_busy) return;

// reread the current pot setting
	static char szval[10];
	std::string rsp;
	snprintf(szval, sizeof(szval), "~P?");
	nano_sendString(szval);
	rsp = nano_read_string(100, szval);
	REQ(rcvd, rsp);
	int val = 0;
	size_t p = rsp.find("wpm");
	if (p != std::string::npos) {
		rsp.erase(0,p);
		if (sscanf(rsp.c_str(), "wpm %d", &val) == 1) {
			REQ(set_paddle_WPM, val);
		}
	}
}

void nanoIO_use_pot()
{
	std::string s = "~P";
	if (progdefaults.nanoIO_speed_pot) s += '1';
	else s += '0';
	nano_sendString(s);
	std::string rsp = nano_read_string(100, s);
	REQ(rcvd, rsp);
	nanoIO_read_pot();
}

void set_paddle_WPM (int wpm)
{
	cntr_nanoCW_paddle_WPM->value(wpm);
	cntr_nanoCW_paddle_WPM->redraw();
}

void set_nanoIO_min_max()
{
	static char szval[10];
	std::string rsp;
// set min value for potentiometer
	snprintf(szval, sizeof(szval), "~M%dm", (int)cntr_nanoIO_min_wpm->value());
	nano_sendString(szval);
	rsp  = nano_read_string(100, szval);
	REQ(rcvd, rsp);
// set range value of potentiometer
	snprintf(szval, sizeof(szval), "~N%dn", (int)cntr_nanoIO_rng_wpm->value());
	nano_sendString(szval);
	rsp = nano_read_string(100, szval);
	REQ(rcvd, rsp);
	nanoIO_read_pot();
}

