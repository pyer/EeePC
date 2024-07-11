
#ifndef LOG_H
#define LOG_H

extern size_t wlen(const char *);
extern void log_info( const char *,const char *,const char *);
extern void log_warn( const char *,const char *,const char *);
extern void log_error(const char *,const char *,const char *);

#endif
