try:
    from .maiacore import *

    # `import *` skips dunder names, so `__version__` (set by the compiled
    # extension) needs an explicit re-export here to reach `maialib.maiacore.__version__`.
    from .maiacore import __version__ as __version__
except ImportError:
    from .Release.maiacore import *
    from .Release.maiacore import __version__ as __version__
