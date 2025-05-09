import os
import re
import subprocess
import sys
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

# Lê a versão do arquivo VERSION
version_path = Path(__file__).parent / "VERSION"
version = version_path.read_text().strip()

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}

# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.


class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        # type: ignore[no-untyped-call]
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        # Using this requires trailing slash for auto-detection & inclusion of
        # auxiliary "native" libs

        debug = int(os.environ.get("DEBUG", 0)
                    ) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
        # from Python.
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}maialib{os.sep}maiacore{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
            f"-DPYBIND_LIB=ON",
            f"-DSQLITECPP_RUN_CPPLINT=OFF",
            f"-DSQLITECPP_RUN_CPPCHECK=OFF",
            f"-DSQLITECPP_BUILD_TESTS=OFF",
            f"-DSQLITECPP_USE_STATIC_RUNTIME=ON",
            f'-DMAIALIB_VERSION_INFO={version}',
        ]
        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [
                item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        # In this example, we pass in the version to C++. You might not need to.
        # type: ignore[attr-defined]
        cmake_args += [
            f"-DMAIALIB_VERSION_INFO={self.distribution.get_version()}"]

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator or cmake_generator == "Ninja":
                try:
                    import ninja  # noqa: F401

                    ninja_executable_path = Path(ninja.BIN_DIR) / "ninja"
                    cmake_args += [
                        "-GNinja",
                        f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja_executable_path}",
                    ]
                except ImportError:
                    pass

        else:

            # Single config generators are handled "normally"
            single_config = any(
                x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}maialib{os.sep}maiacore{os.sep}"
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += [
                    "-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        subprocess.run(
            ["cmake", ext.sourcedir] + cmake_args, cwd=build_temp, check=True
        )
        subprocess.run(
            ["cmake", "--build", "."] + build_args, cwd=build_temp, check=True
        )


with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("LICENSE.txt", "r", encoding="utf-8") as fh:
    license_txt = fh.read()

setup(
    name="maialib",
    version=version,
    author="Nycholas Maia",
    author_email="nyckmaia@gmail.com",
    description="A C++/Python library to manipulate sheet music data",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="GNU General Public License v3 or later (GPLv3+)",
    url="https://github.com/nyckmaia/maialib",
    project_urls={
        "Bug Tracker": "https://github.com/nyckmaia/maialib/issues",
    },
    keywords=["music", "score", "sheet music", "analysis"],

    packages=['maialib', 'maialib.maiacore', 'maialib.maiapy'],

    package_dir={"maialib": "maialib",
                 "maiacore": "maialib/maiacore",
                 "maiapy": "maialib/maiapy"},
    package_data={"maialib": [
        "py.typed", "*.pyi", "**/*.pyi",
        "xml-scores-examples/Bach_Cello_Suite_1.mxl",
        "xml-scores-examples/Beethoven_Symphony_5_mov_1.xml",
        "xml-scores-examples/Chopin_Fantasie_Impromptu.mxl",
        "xml-scores-examples/Dvorak_Symphony_9_mov_4.mxl",
        "xml-scores-examples/Mahler_Symphony_8_Finale.mxl",
        "xml-scores-examples/Mozart_Requiem_Introitus.mxl",
        "xml-scores-examples/Strauss_Also_Sprach_Zarathustra.mxl"],
        "maiapy": ["py.typed", "*.pyi", "**/*.pyi"],
        "maiacore": ["py.typed", "*.pyi", "**/*.pyi"]},
    include_package_data=True,

    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Operating System :: OS Independent",
        "Intended Audience :: Science/Research",
        "Natural Language :: English",
        "Topic :: Software Development :: Libraries"
    ],
    install_requires=["pandas>=2.0.0", "plotly", "kaleido", "nbformat"],
    python_requires=">=3.8.0",
    zip_safe=False,
    ext_modules=[CMakeExtension(name="maiacore")],
    cmdclass={"build_ext": CMakeBuild}
)
