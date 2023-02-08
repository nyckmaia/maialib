from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("LICENSE.txt", "r", encoding="utf-8") as fh:
    license_txt = fh.read()

setup(
    name="maialib",
    version="dev",
    author="Nycholas Maia",
    author_email="nyckmaia@gmail.com",
    description="A C++/Python library to manipulate music data",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="GNU General Public License v3 or later (GPLv3+)",
    url="https://github.com/nyckmaia/maia",
    project_urls={
        "Bug Tracker": "https://github.com/nyckmaia/maia/issues",
    },
    keywords=["music", "score", "sheet music", "analysis"],
    packages=find_packages(),
    # package_data={
    #     '': ['*.so', '*.pyd', '__init__.pyi', 'maiacore/__init__.pyi', "xml-scores-examples/cello_suite_1_violin.xml",
    #          "xml-scores-examples/Symphony_5th_1Mov.xml"]
    # },

    # package_dir={"": "",
    #              "": "maiacore",
    #              "": "maiapy"},
    package_data={"": ['*.so', '*.pyd', '__init__.pyi', 'maiacore/__init__.pyi',
                       "py.typed", "*.pyi", "**/*.pyi",
                       "xml-scores-examples/cello_suite_1_violin.xml",
                       "xml-scores-examples/Symphony_5th_1Mov.xml"]},


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
