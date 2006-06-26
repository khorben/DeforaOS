/* settings.h */


#ifndef CONFIGURE_SETTINGS_H
# define CONFIGURE_SETTINGS_H

# include <System.h>
# include "configure.h"


/* functions */
int settings(Prefs * prefs, Config * config, String const * directory,
		String const * package, String const * version);

#endif /* !CONFIGURE_SETTINGS_H */
