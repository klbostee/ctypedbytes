from setuptools import setup, Extension
setup(name='ctypedbytes',
      version='0.1.8',
      description='A fast Python module for dealing with so called "typed bytes"',
      author='Klaas Bosteels',
      author_email='klaas@last.fm',
      license = 'Apache Software License (ASF)',
      url='http://github.com/klbostee/ctypedbytes',
      py_modules=['ctypedbytes'],
      ext_modules=[Extension('fastb', ['fastb.c'])],
      install_requires = ['typedbytes>=0.3.7'],
      test_suite='nose.collector',
      tests_require=['nose']
     )
