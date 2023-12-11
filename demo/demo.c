/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 *
 * This code is an adaptation of the basic example found in Open62541:
 * https://github.com/open62541/open62541/blob/v1.3.9/examples/server.cpp
 */

#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>

#include <signal.h>
#include <stdlib.h>

#include "../xsettings.h"


static UA_Boolean running = true;

static void
info(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    UA_Log_Stdout->log(UA_Log_Stdout->context,
                       UA_LOGLEVEL_INFO, UA_LOGCATEGORY_SERVER,
                       format, args);
    va_end(args);
}

static void
loop_stop(int signo)
{
    info("Stopping the OPC/UA server");
    running = false;
}

/* Not caring that much about proper cleaning here: leaving
 * this function will lead to exiting the program anyway
 */
static UA_StatusCode
opcua_server(XSettings xsettings)
{
    UA_StatusCode status;
    UA_Server *server;
    UA_NodeId folder;

    info("Configuring the OPC/UA server");
    server = UA_Server_new();
    status = UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    if (UA_StatusCode_isBad(status)) {
        return status;
    }

    info("Creating the container folder");
    status = UA_Server_addObjectNode(server, UA_NODEID_NULL,
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                     UA_QUALIFIEDNAME(1, "Settings"),
                                     UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
                                     UA_ObjectAttributes_default, NULL, &folder);
    if (UA_StatusCode_isBad(status)) {
        return status;
    }

    info("Populating the folder with XSettings");
    status = xsettings_register(xsettings, server, folder);
    if (UA_StatusCode_isBad(status)) {
        return status;
    }

    info ("Running the OPC/UA server");
    signal(SIGINT,  loop_stop);
    signal(SIGTERM, loop_stop);
    signal(SIGQUIT, loop_stop);
    return UA_Server_run(server, &running);
}

/* Usage: `./xsettings-demo [-c | -d]`
 * -c: create default XSettings file (*required* to work properly)
 * -d: dump current XSettings file
 *
 * Without any argument, an OPC/UA server is started exposing the
 * current XSettings values inside the `Root/Objects/Settings` folder.
 */
int
main(int argc, char *argv[])
{
    XSettings xsettings;
    UA_StatusCode status;

    xsettings = xsettings_new("/tmp/demo.xsettings");
    if (argc <= 1) {
        /* No arguments: start the OPC/UA server */
        info("Starting OPC/UA server");
        status = opcua_server(xsettings);
    } else if (strcmp(argv[1], "-c") == 0) {
        /* `-c`: create settings */
        info("Creating settings");
        status = xsettings_reset(xsettings);
    } else if (strcmp(argv[1], "-d") == 0) {
        /* `-d`: dump settings */
        status = xsettings_dump(xsettings);
    } else {
        info("Unrecognized option: %s", argv[1]);
        status = UA_STATUSCODE_BADINVALIDARGUMENT;
    }
    xsettings_free(xsettings);

    if (UA_StatusCode_isGood(status)) {
        return EXIT_SUCCESS;
    } else {
        info("Failure: status code is %s", UA_StatusCode_name(status));
        return EXIT_FAILURE;
    }
}
