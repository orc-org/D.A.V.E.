from setuptools import find_packages
from setuptools import setup

setup(
    name='enigma_machine',
    version='0.0.0',
    packages=find_packages(
        include=('enigma_machine', 'enigma_machine.*')),
)
