from setuptools import setup, Extension
setup(name='ctypedbytes',
      version='0.1.3',
      description='A fast Python module for dealing with so called "typed bytes"',
      author='Klaas Bosteels',
      author_email='klaas@last.fm',
      url='http://github.com/klbostee/ctypedbytes',
      py_modules=['ctypedbytes'],
      ext_modules=[Extension('fastb', ['fastb.c'])],
      install_requires = ['typedbytes'],
      test_suite='nose.collector',
      tests_require=['nose']
     )
