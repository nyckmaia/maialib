from setuptools import setup, find_packages
from pathlib import Path

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("LICENSE.txt", "r", encoding="utf-8") as fh:
    license_txt = fh.read()

# Lê a versão do arquivo VERSION
version_path = Path(__file__).parent.parent / "VERSION"
version = version_path.read_text().strip()

setup(
    name="maialib",
    version=version + "-dev",
    author="Nycholas Maia",
    author_email="nyckmaia@gmail.com",
    description="A C++/Python library to manipulate music data",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="GNU General Public License v3 or later (GPLv3+)",
    url="https://github.com/nyckmaia/maialib",
    project_urls={
        "Bug Tracker": "https://github.com/nyckmaia/maialib/issues",
    },
    keywords=["music", "score", "sheet music", "analysis"],
    packages=find_packages(),
    package_data={"": ['*.so', '*.pyd', '__init__.pyi', 'maiacore/__init__.pyi',
                       "py.typed", "*.pyi", "**/*.pyi",
                       "xml-scores-examples/Bach_Cello_Suite_1.mxl",
                       "xml-scores-examples/Beethoven_Symphony_5_mov_1.xml",
                       "xml-scores-examples/Chopin_Fantasie_Impromptu.mxl",
                       "xml-scores-examples/Dvorak_Symphony_9_mov_4.mxl",
                       "xml-scores-examples/Mahler_Symphony_8_Finale.mxl",
                       "xml-scores-examples/Mozart_Requiem_Introitus.mxl",
                       "xml-scores-examples/Strauss_Also_Sprach_Zarathustra.mxl"]},

    include_package_data=True,
    py_modules=["maiacore", "maiapy"],
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
    python_requires=">=3.8.0",
    zip_safe=False,
)
