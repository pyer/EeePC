/* Public domain. */

#ifndef SIG_H
#define SIG_H

extern void sig_catch(int,void (*)());
extern void sig_block(int);
extern void sig_unblock(int);

#define sig_default(s) (sig_catch((s),SIG_DFL))

#endif
