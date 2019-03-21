from setuptools import setup

setup(
    name = 'sphinx-tabs',
    version = '1.1.10',
    author = 'djungelorm',
    author_email = 'djungelorm@users.noreply.github.com',
    packages = ['sphinx_tabs', 'sphinx_tabs.test'],
    test_suite='sphinx_tabs.test',
    package_data = {
        'sphinx_tabs': [
            'tabs.js',
            'tabs.css',
            'semantic-ui-2.2.10/*',
        ],
    },
    data_files = [("", ["LICENSE"])],
    url = 'https://github.com/djungelorm/sphinx-tabs',
    license = 'MIT',
    description = 'Tab views for Sphinx',
    install_requires = ['sphinx>=1.4'],
    tests_require = ['sphinx>=1.4', 'docutils', 'pygments', 'sphinx_testing', 'lxml'],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Plugins',
        'Environment :: Web Environment',
        'Framework :: Sphinx :: Extension',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Natural Language :: English',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python',
        'Topic :: Documentation :: Sphinx',
        'Topic :: Documentation',
        'Topic :: Software Development :: Documentation',
        'Topic :: Text Processing',
        'Topic :: Utilities',
    ]
)
