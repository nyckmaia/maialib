import platform

if platform.system() != "Windows":
    from .maiacore import *
else:
    from .Release.maiacore import *
