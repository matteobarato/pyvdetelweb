from setuptools import setup, find_packages

with open('README.md') as f:
    readme = f.read()

setup(name='pyvdetelweb',
      version='0.1',
      author="Matteo Barato",
      author_email="matteo.barato@studio.unibo.it",
      description='Managment tools for VDE',
      long_description=readme,
      keywords='vde vdetelweb vde2 vde_switch circus',
      url='https://github.com/matteobarato/pyvdetelweb',
      project_urls={
          'Documentation': 'http://wiki.v2.cs.unibo.it/wiki/index.php?title=Main_Page',
          'Say Thanks!': 'https://github.com/rd235',
      },
      license='MIT',
      packages=find_packages(),
      install_requires=['Flask'],
      scripts=['bin/pyvdetelweb'],
      package_data={
          '': ['*.html','*.css','*.js']
      },
      data_files=[('/etc/vde/pyvdetelweb/', ['samples/vdetelweb.rc'])],

      include_package_data=True,
      zip_safe=False)
