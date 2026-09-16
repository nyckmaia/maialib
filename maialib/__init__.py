from .maiacore import *

# `import *` skips dunder names, so `__version__` needs an explicit re-export here.
# The `as __version__` alias is the standard convention ruff/pyflakes recognize as
# an intentional re-export, so it does not trigger an "unused import" (F401) finding.
from .maiacore import __version__ as __version__
from .maiapy.other import *
from .maiapy.plots import *
from .maiapy.sethares_dissonance import *
