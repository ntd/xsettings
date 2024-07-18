#define XSTRING_LEN 255
#define XSETTINGS \
    /*NAME        TYPE      DEFAULT   DESCRIPTION */ \
    X(Bool_true,  XBOOLEAN, true,     "True value") \
    X(Bool_false, XBOOLEAN, false,    "False value") \
    X(I32_12,     XINT32,   123,      "12 signed 32 bits") \
    X(I32__23,    XINT32,   -123,     "-23 signed 32 bits") \
    X(U32_0,      XUINT32,  0,        "0 unsigned 32 bits") \
    X(U32_34,     XUINT32,  34,       "34 unsigned 32 bits") \
    X(Double_56,  XDOUBLE,  5.6,      "5.6 double") \
    X(Double__78, XDOUBLE,  -7.8,     "-7.8 double") \
    X(Empty,      XSTRING,  "",       "Empty string") \
    X(String,     XSTRING,  "String", "Generic string value") \
/* EOF */
