#define XINT32          UA_TYPES_INT32, UA_Int32
#define XUINT32         UA_TYPES_UINT32, UA_UInt32
#define XBOOLEAN        UA_TYPES_BOOLEAN, UA_Boolean
#define XDOUBLE         UA_TYPES_DOUBLE, UA_Double

#define X(...)          X_(__VA_ARGS__)
#define BINDINGS(...)   BINDINGS_(__VA_ARGS__)

#include "xsettings.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


typedef struct {
#define X_(name, idx, type, defvalue, description) \
    type name;
    XSETTINGS
#undef X_
} Contents;

struct _XSettings {
    Contents *contents;
    /* 128 is just a dummy size, see xsettings_new() for details */
    char file[128];
};


static void
print_type_value(int idx, const void *value)
{
    switch (idx) {
    case UA_TYPES_INT32:
        printf("XINT32,   %8d", * (UA_Int32 *) value);
        break;
    case UA_TYPES_UINT32:
        printf("XUINT32,  %8u", * (UA_UInt32 *) value);
        break;
    case UA_TYPES_BOOLEAN:
        printf("XBOOLEAN, %8s", * (UA_Boolean *) value ? "true" : "false");
        break;
    case UA_TYPES_DOUBLE:
        printf("XDOUBLE,  %8g", * (UA_Double *) value);
        break;
    }
}

#define X_(name, idx, type, defvalue, description) \
    static UA_StatusCode read_ ## name(UA_Server *server, \
            const UA_NodeId *session, void *session_data, \
            const UA_NodeId *node, void *node_data, UA_Boolean timestamp, \
            const UA_NumericRange *range, UA_DataValue *data) { \
        XSettings xsettings = node_data; \
        UA_Variant_setScalar(&data->value, &xsettings->contents->name, &UA_TYPES[idx]); \
        data->value.storageType = UA_VARIANT_DATA_NODELETE; \
        data->hasValue = true; \
        return UA_STATUSCODE_GOOD; \
    } \
    static UA_StatusCode write_ ## name(UA_Server *server, \
            const UA_NodeId *session, void *session_data, \
            const UA_NodeId *node, void *node_data, \
            const UA_NumericRange *range, const UA_DataValue *data) { \
        if (data->hasValue && data->value.data != NULL) { \
            XSettings xsettings = node_data; \
            xsettings->contents->name = * (type *) data->value.data; \
            msync(xsettings->contents, sizeof(Contents), MS_ASYNC); \
        } \
        return UA_STATUSCODE_GOOD; \
    }
XSETTINGS
#undef X_

static UA_StatusCode
register_setting(void *xsettings, UA_Server *server, UA_NodeId folder,
                 const char *name, const char *summary,
                 int idx, const UA_DataSource binding)
{
    UA_VariableAttributes attrs;
    UA_QualifiedName browse;
    UA_NodeId node, reference, definition;

    attrs = UA_VariableAttributes_default;
    attrs.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attrs.displayName = UA_LOCALIZEDTEXT("", (char *) name);
    attrs.description = UA_LOCALIZEDTEXT("", (char *) summary);
    attrs.dataType = UA_TYPES[idx].typeId;

    node = UA_NODEID_STRING(folder.namespaceIndex, (char *) name);
    browse = UA_QUALIFIEDNAME(folder.namespaceIndex, (char *) name);
    reference = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    return UA_Server_addDataSourceVariableNode(server, node, folder,
                                               reference, browse,
                                               definition, attrs,
                                               binding, xsettings, NULL);
}


XSettings
xsettings_new(const char *file)
{
    size_t len;
    XSettings xsettings;

    len = strlen(file);
    if (len <= 0 ) {
        return NULL;
    }

    /* Change the dummy size (128) to the real string length:
     * this should take into account eventual alignment */
    xsettings = malloc(sizeof(struct _XSettings) - 128 + len + 1);
    xsettings->contents = NULL;
    memcpy(xsettings->file, file, len + 1);
    return xsettings;
}

UA_StatusCode
xsettings_reset(XSettings xsettings)
{
    Contents contents;
    int handle;
    ssize_t size;

    handle = open(xsettings->file, O_CREAT | O_WRONLY | O_TRUNC, 00600);
    if (handle < 0) {
        return UA_STATUSCODE_BADUSERACCESSDENIED;
    }

#define X_(name, idx, type, defvalue, description) \
    contents.name = defvalue;
    XSETTINGS
#undef X_

    size = write(handle, &contents, sizeof(contents));
    close(handle);
    if (size != sizeof(contents)) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
xsettings_dump(XSettings xsettings)
{
    int handle, width;
    Contents contents;
    ssize_t size;

    handle = open(xsettings->file, O_RDONLY);
    if (handle < 0) {
        return UA_STATUSCODE_BADNOTFOUND;
    }

    size = read(handle, &contents, sizeof(contents));
    close(handle);

    if (size != sizeof(contents)) {
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    /* Get the longest name */
    width = 0;
#define X_(name, idx, type, defvalue, description) \
    if (strlen(#name) > width) { \
        width = strlen(#name); \
    }
    XSETTINGS
#undef X_
    ++width;

    printf("#define XSETTINGS \\\n"
           "    /*%-*s TYPE       DEFAULT   DESCRIPTION */ \\\n",
           width, "NAME");
#define X_(name, idx, type, defvalue, description) \
    printf("    X(%-*s ", width, #name ","); \
    print_type_value(idx, &contents.name); \
    printf(", \"%s\") \\\n", description);
    XSETTINGS
#undef X_

    /* This line is needed to make the last backslash happy */
    printf("/* EOF */\n");

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
xsettings_register(XSettings xsettings, UA_Server *server, UA_NodeId folder)
{
    int handle;
    UA_StatusCode status;
    UA_DataSource binding;

    handle = open(xsettings->file, O_RDWR);
    if (handle < 0) {
        return UA_STATUSCODE_BADNOTFOUND;
    }

    xsettings->contents = mmap(NULL, sizeof(Contents),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED, handle, 0);
    close(handle);

    if (xsettings->contents == MAP_FAILED) {
        /* The error most likely to cause a map failure is `ENXIO`,
         * e.g. because of the file size being different than
         * sizeof(Contents) because of a schema change. */
        xsettings->contents = NULL;
        return UA_STATUSCODE_BADOUTOFRANGE;
    }

#define X_(name, idx, type, defvalue, description) \
    binding.read = read_ ## name; \
    binding.write = write_ ## name; \
    status = register_setting(xsettings, server, folder, \
                              #name, #description, idx, binding); \
    if (UA_StatusCode_isBad(status)) \
        return status;
    XSETTINGS
#undef X_

    return UA_STATUSCODE_GOOD;
}

void
xsettings_free(XSettings xsettings)
{
    if (xsettings->contents != NULL) {
        munmap(xsettings->contents, sizeof(Contents));
    }
    free(xsettings);
}
