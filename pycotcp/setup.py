#!/usr/bin/env python
# -*- coding: utf-8 -*-

from setuptools import setup, find_packages, Extension


with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

m_pyco = Extension('pycoclib',
                    include_dirs = ['./pycoclib/include','./pycoclib/headers'],
                    libraries = ['vdeplug'],
                    ##sotto era commentato
                    library_dirs = ['./pycoclib/lib'],
                    extra_objects=["./pycoclib/lib/libpicotcp.a"],
                    #extra_link_args=['-static'],
                    sources = ['./pycoclib/sources/pycoclib.c','./pycoclib/sources/pycoutils.c'])

setup(
    name='pycotcp',
    version='0.9.0',
    description='PycoTCP',
    long_description=readme,
    author='Federico Giuggioloni',
    author_email='federico.giuggioloni@gmail.com',
    url='fedegiugi.noip.me',
    license=license,
    scripts=['scripts/pyco-wrapper', 'scripts/pyco-runner.py', 'scripts/pyco-vde-setup'],
    zip_safe=False,
    packages=find_packages(exclude=('tests', 'docs')),
    ext_modules=[m_pyco],
    setup_requires=['cffi>=1.0.0'],
    cffi_modules=['pycotcp/fdpicobuild.py:ffi', 'pycotcp/lwipv6build.py:ffi'],
    install_requires=['cffi>=1.0.0']
)

