/*
 * log.h
 */

#define DEBUGGING	0

#if DEBUGGING
# define Debug(mask, message) printf message;
#else /* !DEBUGGING */
# define Debug(mask, message) ;
#endif /* DEBUGGING */

#define	DEXT		0x0001	/* extend flag for other debug masks */
#define	DSCH		0x0002	/* scheduling debug mask */
#define	DPROC		0x0004	/* process control debug mask */
#define	DPARS		0x0008	/* parsing debug mask */
#define	DLOAD		0x0010	/* database loading debug mask */
#define	DMISC		0x0020	/* misc debug mask */
#define	DTEST		0x0040	/* test mode: don't execute any commands */

void log_it(const char *, int, const char *, const char *);
