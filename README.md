XSettings provides a simple persistent storage backend for a specific
set of variables in [open62541](https://github.com/open62541/open62541)
based OPC/UA servers. It is implemented in the simple way I know of,
i.e. directly mmap()ing the struct containing all variables to a binary
file. Any time a variable changes, the mapped memory is msync()ed
**asyncrhronously**, so this code can be used in real-time contexts.

How to use
----------

To get the introspection data needed, the code heavily leverages the
XMacros technique (hence the X in XSettings). This in turn means this
cannot be a library: the source code must be embedded in your project.

A typical way to include XSettings in you project could be:

1. copy `xsettings.c` and `xsettings.h` to your source folder
2. define `xsettings-schema.h`
3. call the relevant XSettings APIs from your code
4. link together

See the `demo` folder for a basic example. Actually, only `UA_Boolean`,
`UA_Int32`, `UA_UInt32` and `UA_Double` types are implemented.

How it works
------------

The main function is `xsettings_register()`. When you call it, XSettings
populates the folder you specified with all the settings you defined in
`xsettings-schema.h`. Their last values will be stored in the mapped
binary file.

WARNING! You must create the binary file backing up your XSettings
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
#define XSETTINGS \
    /*NAME     TYPE       DEFAULT   DESCRIPTION */ \
    X(Boolean, XBOOLEAN,     true, "Boolean flag setting") \
    X(Int32,   XINT32,       -123, "Integer (32 bits) setting") \
    X(UInt32,  XUINT32,       321, "Unsigned integer (32 bits) setting") \
    X(Double,  XDOUBLE,    -9.876, "Double floating point setting") \
/* EOF */
$ ./xsettings-demo
... TCP network layer listening on ...
```

Documentation
-------------

There are only 5 public functions.

- `XSettings xsettings_new(const char *file)`<br>
  This must be called before any other API. It requires the name of the
  file to be mapped and it is not needed to exist, e.g. you can create a
  new file by calling `xsettings_reset()`. The resulting opaque pointer
  must be freed with `xsettings_free()` when done.
- `void xsettings_free(XSettings xsettings)`<br>
  Closes whatever opened by `xsettings_new`.
- `UA_StatusCode xsettings_reset(XSettings xsettings)`<br>
  It creates the file to be mapped (or overwrite it) using the default
  values specified by the schema.
- `UA_StatusCode xsettings_dump(XSettings xsettings)`<br>
  It dumps to stdout the contents of the mapped file. The format used is
  purposedly compatible with `xsettings-schema.h`, so you can easily
  overwrite it to e.g. update the default values.
- `UA_StatusCode xsettings_register(XSettings xsettings, UA_Server *opcua, UA_NodeId folder)`<br>
  The real meat of this project: `mmap()` the file (so it must exists)
  and register all fields found in `xsettings-schema.h` as
  [data source](https://www.open62541.org/doc/1.3/server.html#data-source-callback)
  variables under `folder`.
