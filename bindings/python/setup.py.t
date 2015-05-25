from setuptools import setup, find_packages
from glob import glob

__version__ = '0.8'

setup(
	name = 'libaudioverse',
	version = __version__,
	description = """Python bindings for Libaudioverse, a library for 3d and environmental audio""",
	package_dir = {'libaudioverse': 'libaudioverse'},
	packages = find_packages(),
	package_data = {'libaudioverse':
		['*.dll']
	},
	classifiers = [
		'Development Status :: 3 - Alpha',
		'Intended Audience :: Developers',
		'Programming Language :: Python',
		'License :: OSI Approved :: GPL',
		'Topic :: Software Development :: Libraries'
	],
	zip_safe = False
)
