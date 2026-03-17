from setuptools import setup, find_packages

setup(
    name="axiom_studio",
    version="1.0.0",
    packages=find_packages(),
    install_requires=[
        "PySide6",
        "psutil"
    ],
    entry_points={
        "console_scripts": [
            "axiom-studio=axiom_studio.__main__:main"
        ]
    }
)