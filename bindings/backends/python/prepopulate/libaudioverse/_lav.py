#implements lifting the raw ctypes-basedd api into something markedly pallatable.
#among other things, the implementation heree enables calling functions with keyword arguments and raises exceptions on error, rather than dealing with ctypes directly.
from __future__ import absolute_import
import ctypes
import collections
import functools
from . import _libaudioverse
import six

#These are not from libaudioverse.
#Implement a method by which the public libaudioverse module may register its exception classes for error code translation.
class PythonBindingsCouldNotTranslateErrorCodeError(Exception):
    """An exception representing failure to translate a libaudioverse error code into a python exception.  If you see this, report it as a bug with Libaudioverse because something has gone very badly wrong."""
    pass

errors_to_exceptions = dict()

def bindings_register_exception(code, cls):
    errors_to_exceptions[code] = cls

def make_error_from_code(err):
    """Internal use.  Translates libaudioverse error codes into exceptions."""
    return errors_to_exceptions.get(err, PythonBindingsCouldNotTranslateErrorCodeError)()

#Handle marshalling and automatic refcount stuff:
@functools.total_ordering
class _HandleBox(object):

    def __init__(self, handle):
        self.handle= int(handle)
        first_access= ctypes.c_int()
        _libaudioverse.Lav_handleGetAndClearFirstAccess(handle, ctypes.byref(first_access))
        if not first_access:
            _libaudioverse.Lav_handleIncRef(handle)

    def __eq__(self, other):
        if not isinstance(other, _HandleBox): return False
        else: return self.handle == other.handle

    def __lt__(self, other):
        if not isinstance(other, _HandleBox): return True #other classes are "less" than us.
        return self.handle < other.handle

    def __hash__(self):
        return self.handle

    def __del__(self):
        #Guard against interpreter shutdown.
        if self.handle is None: return
        deleter = getattr(_libaudioverse, 'Lav_handleDecRef', None)
        if deleter is not None:
            deleter(self.handle)
        self.handle = None

    def _to_handle(self):
        return self.handle

def reverse_handle(handle):
    return _HandleBox(handle)
