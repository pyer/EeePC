/* Public domain. */

#ifndef SIG_H
#define SIG_H

extern void sig_catch(int,void (*)());
extern void sig_block(int);
extern void sig_unblock(int);
extern void sig_blocknone(void);
extern void sig_pause(void);

/*
#define sig_ignore(s)  (sig_catch((s),SIG_DFL))
#define sig_uncatch(s) (sig_catch((s),SIG_DFL))
*/
#define sig_default(s) (sig_catch((s),SIG_DFL))

#endif
