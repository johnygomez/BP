from distutils.core import setup, Extension
import numpy.distutils.misc_util

setup(
    ext_modules=[Extension(
      "_rprop", 
      ["_rprop.c"],
      extra_compile_args = ['-std=gnu99', '-O3', '-ffast-math', '-funroll-loops' ,'-s']
      )],
    include_dirs=numpy.distutils.misc_util.get_numpy_include_dirs(),
)