*XSettings* provides a basic persistent storage backend for a specific
set of variables in [open62541](https://github.com/open62541/open62541)
based OPC/UA servers. It is implemented in the simplest way I know of,
i.e. directly mmap()ing the struct containing all variable values to a
binary file. Any time a variable changes, the mapped memory is msync()ed
**asynchronously**, so this code can be used in real-time contexts.

How to use
----------

To get the introspection data needed, the code heavily leverages the
preprocessor using the [X-Macro](https://en.wikipedia.org/wiki/X_macro)
technique (hence the X in *XSettings*). This in turn means this project
cannot be a library: the source code must be embedded in some way into
your project.

The typical way of doing that is:

1. copy `xsettings.c` and `xsettings.h` into your source folder;
2. define your schema by creating `xsettings-schema.h`;
3. call the relevant *XSettings* APIs from your code;
4. integrate your build system accordingly.

Refer to the `demo` folder for a basic example. Actually `UA_Boolean`,
`UA_Int32`, `UA_UInt32`, `UA_Double` and `UA_String` (up to 255 bytes)
types are implemented.

How it works
------------

When you call `xsettings_register()`, *XSettings* populates the folder
you specified with all the settings you defined in your schema. Their
last values will be remembered via the mapped binary file.

WARNING! You must create the binary file backing up your *XSettings*
schema before using them, otherwise `xsettings_register()` will fail.
The following shell session highlights the issue using the demo program
incuded in this project as an example:

```sh
$ ./xsettings-demo
... Failure: status code is BadNotFound
$ ./xsettings-demo -d
... Failure: status code is BadNotFound
$ ./xsettings-demo -c
... Creating settings
$ ./xsettings-demo -d
#define XSTRING_LEN 255
#define XSETTINGS \
    /*NAME     TYPE       DEFAULT   DESCRIPTION */ \
    X(Boolean, XBOOLEAN,     true, "Boolean flag setting") \
    X(Int32,   XINT32,       -123, "Integer (32 bits) setting") \
    X(UInt32,  XUINT32,       321, "Unsigned integer (32 bits) setting") \
    X(Double,  XDOUBLE,    -9.876, "Double floating point setting") \
    X(String,  XSTRING, "String", "String setting, up to 255 chars") \
/* EOF */
$ ./xsettings-demo
... TCP network layer listening on ...
```

Documentation
-------------

There are only 5 public functions.

- `XSettings xsettings_new(const char *file)`<br>
  It must be called before any other function, passing the name of the
  file to be mapped. If that file does not exist, it can be created by
  a subsequent `xsettings_reset()` call. The resulting opaque pointer
  must be freed with `xsettings_free()` when done.
- `void xsettings_free(XSettings xsettings)`<br>
  No other *XSettings* function should be called after.
- `UA_StatusCode xsettings_reset(XSettings xsettings)`<br>
  It creates the file to be mapped (or overwrites it) using the default
  values specified in `xsettings-schema.h`.
- `UA_StatusCode xsettings_dump(XSettings xsettings)`<br>
  It dumps to stdout the contents of the mapped file. The format is
  purposedly compatible with `xsettings-schema.h`, so you can easily
  overwrite it to e.g. update the default values.
- `UA_StatusCode xsettings_register(XSettings xsettings, UA_Server *opcua, UA_NodeId folder)`<br>
  The real meat of this project: mmap() the file (so it must exists!)
  and register all fields found in your schema as
  [data source](https://www.open62541.org/doc/1.3/server.html#data-source-callback)
  variables under the `folder` node.

Building the demo
-----------------

To build the demo program you must have `open62541` preinstalled. Then
you can compile it in the usual `meson` way:
```sh
$ meson setup build
...
$ meson compile -C build
...
$ cd build/demo
$ ./xsettings-demo -c
... Creating settings
$ ./xsettings-demo
...
```
