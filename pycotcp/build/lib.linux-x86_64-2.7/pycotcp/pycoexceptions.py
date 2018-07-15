#!/usr/bin/env python

import inspect

class error(IOError):
    pass

class gaierror(error):
    pass

class herror(error):
    pass

class timeout(error):
    pass

class LibraryException(Exception):
    def __init__(self, errname, errcode, libname):
        caller = inspect.currentframe().f_back.f_back.f_code.co_name
        Exception.__init__(self, "LibraryException: %s in library %s, function %s. (%d)" % (errname, libname, caller, errcode))

class ConfigurationException(Exception):
    def __init__(self, errname, errcode, libname):
        caller = inspect.currentframe().f_back.f_back.f_code.co_name
        Exception.__init__(self, "ConfigurationException (check VDE) : %s in library %s, function %s. (%d)" % (errname, libname, caller, errcode))

class ParseException(Exception):
    def __init__(self, errname, errcode, libname):
        caller = inspect.currentframe().f_back.f_back.f_code.co_name
        Exception.__init__(self, "ParseException (check arguments) : %s in library %s, function %s. (%d)" % (errname, libname, caller, errcode))

