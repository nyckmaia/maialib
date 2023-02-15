import platform as _plt

if _plt.system() != "Windows":
    from .maiacore import *
else:
    from .Release.maiacore import *
