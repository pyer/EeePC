/* Public domain. */

#ifndef WAIT_H
#define WAIT_H

extern int wait_pid();
extern int wait_nohang();

#define wait_crashed(w) ((w) & 127)
#define wait_exitcode(w) ((w) >> 8)

#endif
