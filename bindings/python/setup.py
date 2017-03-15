#!/usr/bin/env python3
from distutils.core import setup
from distutils.command.sdist import sdist
from distutils.command.build_clib import build_clib
import os
import sys
import subprocess
import shutil

class custom_sdist(sdist):
    def run(self):
        # copy files to somewhere whose paths don't contain "../", and included via MANIFEST.in
        srcfiles = []
        srctrees = []
        srcfiles += [ '../../CMakeLists.txt' ]
        srctrees += [ '../../src' ]
        srctrees += [ '../../include' ]
        for src in srcfiles:
            dst = os.path.join('proofofwork', os.path.basename(src))
            if os.path.exists(dst):
                os.remove(dst)
            print('copying {} -> {}'.format(src, dst))
            shutil.copy2(src, dst)
        for src in srctrees:
            dst = os.path.join('proofofwork', os.path.basename(src))
            if os.path.exists(dst):
                shutil.rmtree(dst)
            print('copying {} -> {}'.format(src, dst))
            shutil.copytree(src, dst)
        return sdist.run(self)

class custom_build_clib(build_clib):
    def build_libraries(self, libraries):
        # workaround
        subprocess.check_call([ 'cmake', 'proofofwork', '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=build/lib/proofofwork' ], stdout=sys.stdout, stderr=sys.stderr)
        subprocess.check_call([ 'make' ], stdout=sys.stdout, stderr=sys.stderr)

setup(
    name='proofofwork',
    version='0.0.1',
    description='', # TODO
    # long_description=readme,
    author='Kimiyuki Onaka',
    author_email='kimiyuki95@gmail.com',
    url='https://github.com/kmyk/', # TODO
    license='MIT License',
    packages=[ 'proofofwork' ],
    cmdclass=dict(
        sdist=custom_sdist,
        build_clib=custom_build_clib,
    ),
    package_data={
        'proofofwork' : [
            'CMakeLists.txt',
            'src/*.c',
            'include/*.h',
        ],
    },
    libraries=[(
        'proofofwork', dict(
            package='proofofwork',
            sources=[], # dummy
        ),
    )],
    classifiers=[
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3',
    ],
)
