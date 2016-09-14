// generated by Fast Light User Interface Designer (fluid) version 1.0303

#ifndef fd_view_h
#define fd_view_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
extern Fl_Output *view_FD_call;
extern Fl_Output *view_FD_class;
extern Fl_Output *view_FD_section;
extern Fl_Output *view_FD_mult;
extern Fl_Output *view_FD_score;
#include <FL/Fl_Box.H>
extern Fl_Output *view_FD_CW[12];
extern Fl_Output *view_FD_CW_OP[12];
extern Fl_Output *view_FD_DIG[12];
extern Fl_Output *view_FD_DIG_OP[12];
extern Fl_Output *view_FD_PHONE[12];
extern Fl_Output *view_FD_PHONE_OP[12];
#include <FL/Fl_Group.H>
#include "flinput2.h"
extern Fl_Input2 *inp_fd_tcpip_addr;
extern Fl_Input2 *inp_fd_tcpip_port;
extern Fl_Input2 *inp_fd_op_call;
#include <FL/Fl_Check_Button.H>
#include "confdialog.h"
#include "fl_digi.h"
extern Fl_Check_Button *btn_fd_connect;
extern Fl_Box *box_fdserver_connected;
Fl_Double_Window* make_fd_view();
#endif
