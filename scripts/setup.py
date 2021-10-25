import setuptools

# with open("README.md", "r", encoding="utf-8") as fh:
#    long_description = fh.read()
long_description = ""

setuptools.setup(
    name="cpp2py_plotly",
    version="0.0.1",
    author="Example Author",
    author_email="author@example.com",
    description="A small example package",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/pypa/sampleproject",
    project_urls={
        "Bug Tracker": "https://github.com/pypa/sampleproject/issues",
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    package_dir={"": "cpp2py_plotly"},
    packages=setuptools.find_packages(where="cpp2py_plotly"),
    python_requires=">=3.6",
)
