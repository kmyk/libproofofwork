#!/usr/bin/env python
from distutils.core import setup
from distutils.command.sdist import sdist
from distutils.command.build import build
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

class custom_build(build):
    def run(self):
        # workaround
        os.chdir('proofofwork')
        subprocess.check_call([ 'cmake', '.' ], stdout=sys.stdout, stderr=sys.stderr)
        subprocess.check_call([ 'make' ], stdout=sys.stdout, stderr=sys.stderr)
        os.chdir('..')
        return build.run(self)

setup(
    name='proofofwork',
    version='0.0.5',
    description='Simple hash-mining library',
    author='Kimiyuki Onaka',
    author_email='kimiyuki95@gmail.com',
    url='https://github.com/kmyk/libproofofwork',
    license='MIT License',
    packages=[ 'proofofwork' ],
    cmdclass=dict(
        sdist=custom_sdist,
        build=custom_build,
    ),
    package_data={
        'proofofwork' : [
            'CMakeLists.txt',
            'src/*.c',
            'include/*.h',
            'lib*.so'
        ],
    },
    classifiers=[
        'Environment :: Console',
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Programming Language :: C',
        'Topic :: Security :: Cryptography',
    ],
)
