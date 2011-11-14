import os
import sipconfig
import pyqtconfig

# The name of the SIP build file generated by SIP and used by the build
# system.
build_file = "QWave2.sbf"

# Get the SIP configuration information.
config = pyqtconfig.Configuration()

# Run SIP to generate the code.
os.system(" ".join([config.sip_bin, "-t", "WS_WIN", "-t", "Qt_3_2_0", "-c", "../python", "-b", build_file, "-w", "-I", "c:/python24/sip", "-I", ".", "main.sip"]))

# Create the Makefile.
makefile = pyqtconfig.QtModuleMakefile(config, build_file, makefile="../python/Makefile")

# Add the library we are wrapping.  The name doesn't include any platform
# specific prefixes or extensions (e.g. the "lib" prefix on UNIX, or the
# ".dll" extension on Windows).
makefile.extra_include_dirs = ["../src","../../include"]
makefile.extra_lib_dirs = ["../src"]
makefile.extra_libs = ["libqwave2"]

# Generate the Makefile itself.
makefile.generate()
