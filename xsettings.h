#ifndef XSETTINGS_H_
#define XSETTINGS_H_

#include "xsettings-schema.h"

#ifndef XSETTINGS
    #error "You must define XSETTINGS before including xsettings.h"
#endif

#include <open62541/server.h>

typedef struct _XSettings *XSettings;

XSettings       xsettings_new           (const char *   file);
UA_StatusCode   xsettings_reset         (XSettings      xsettings);
UA_StatusCode   xsettings_dump          (XSettings      xsettings);
UA_StatusCode   xsettings_register      (XSettings      xsettings,
                                         UA_Server *    opcua,
                                         UA_NodeId      folder);
void            xsettings_free          (XSettings      xsettings);

#endif /* XSETTINGS_H_ */
