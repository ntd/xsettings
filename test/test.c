#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../xsettings.h"

#define XSETTINGS_FILE "/tmp/test.xsettings"


const UA_VariableAttributes UA_VariableAttributes_default;
const UA_DataType UA_TYPES[UA_TYPES_COUNT];

void
__wrap_UA_Variant_setScalar(UA_Variant *variant, void *src,
                            const UA_DataType *type)
{
    check_expected(variant);
    check_expected(src);
    check_expected(type);
}

UA_StatusCode
__wrap_UA_Server_addDataSourceVariableNode(UA_Server *server,
                                           const UA_NodeId requestedNewNodeId,
                                           const UA_NodeId parentNodeId,
                                           const UA_NodeId referenceTypeId,
                                           const UA_QualifiedName browseName,
                                           const UA_NodeId typeDefinition,
                                           const UA_VariableAttributes attr,
                                           const UA_DataSource dataSource,
                                           void *nodeContext,
                                           UA_NodeId *outNewNodeId)
{
    check_expected(server);
    return mock_type(UA_StatusCode);
}


static int
setup(void **state)
{
    unlink(XSETTINGS_FILE);
    return 0;
}

static int
teardown(void **state)
{
    unlink(XSETTINGS_FILE);
    return 0;
}


static void
sanity_checks(void **state)
{
    XSettings xsettings;
    UA_StatusCode status;

    xsettings = xsettings_new("");
    assert_null(xsettings);

    xsettings = xsettings_new(XSETTINGS_FILE);
    assert_non_null(xsettings);

    /* The XSettings file is absent, so `dump` should fail */
    status = xsettings_dump(xsettings);
    assert_int_equal(status, UA_STATUSCODE_BADNOTFOUND);

    /* Creating the XSettings file... */
    status = xsettings_reset(xsettings);
    assert_int_equal(status, UA_STATUSCODE_GOOD);

    /* ...so now `dump` should be successful */
    status = xsettings_dump(xsettings);
    assert_int_equal(status, UA_STATUSCODE_GOOD);

    xsettings_free(xsettings);
}

static void
data_sources(void **state)
{
    XSettings xsettings;
    UA_StatusCode status;
    UA_Server *server;
    UA_NodeId folder;

    xsettings = xsettings_new(XSETTINGS_FILE);
    xsettings_reset(xsettings);

    server = (UA_Server *) teardown;
    memset(&folder, 0, sizeof(UA_NodeId));

    /* Check some random failures */
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_BAD);
    status = xsettings_register(xsettings, server, folder);
    assert_int_equal(status, UA_STATUSCODE_BAD);

    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_BADTIMEOUT);
    status = xsettings_register(xsettings, server, folder);
    assert_int_equal(status, UA_STATUSCODE_BADTIMEOUT);

    /* 8 fields in the XSettings schema = 8 data sources */
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    expect_value(__wrap_UA_Server_addDataSourceVariableNode, server, server);
    will_return(__wrap_UA_Server_addDataSourceVariableNode, UA_STATUSCODE_GOOD);
    status = xsettings_register(xsettings, server, folder);
    assert_int_equal(status, UA_STATUSCODE_GOOD);

    xsettings_free(xsettings);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(sanity_checks),
        cmocka_unit_test(data_sources),
    };
    return cmocka_run_group_tests(tests, setup, teardown);
}
