/* GToolkit.h */



#ifndef GTOOLKIT_GTOOLKIT_H
# define GTOOLKIT_GTOOLKIT_H

# include "GToolkit/GWindow.h"


/* functions */
int g_init(void);
int g_quit(void);

/* useful */
int g_error(char const * message, int ret);
int g_main(void);

#endif /* !GTOOLKIT_GTOOLKIT_H */
