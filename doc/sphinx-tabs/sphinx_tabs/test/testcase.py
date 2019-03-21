# pylint: disable=import-error,no-name-in-module
from distutils.version import StrictVersion
import unittest
import re
import os
import pkg_resources
from sphinx import __version__ as __sphinx_version__
from sphinx.builders.html import StandaloneHTMLBuilder
from lxml import etree

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO


def _parse(xml):
    xml = xml.replace('&copy;', 'copy_sym')
    parser = etree.XMLParser(ns_clean=True, remove_blank_text=True)
    return etree.parse(StringIO(xml), parser)


def _strip_xmlns(xml):
    return xml.replace(' xmlns="http://www.w3.org/1999/xhtml"', '')


def get_scripts(xml):
    tree = _parse(xml)
    scripts = tree.findall('.//{*}script')
    scripts = [x.get('src') for x in scripts]
    return [x.replace('_static/', '') for x in scripts if x is not None]


def get_stylesheets(xml):
    tree = _parse(xml)
    stylesheets = tree.findall('.//{*}link[@rel="stylesheet"]')
    return [x.get('href').replace('_static/', '') for x in stylesheets]


def get_body(xml):
    tree = _parse(xml)
    body = tree.find('.//{*}div[@class="bodywrapper"]')[0][0]
    return _strip_xmlns(etree.tostring(body).decode('utf-8'))


def normalize_xml(xml):
    content = re.sub(r'>\s+<', '><', xml)
    content = etree.tostring(
        _parse(content), pretty_print=True).decode('utf-8')
    return content


class TestCase(unittest.TestCase):
    def tearDown(self):  # pylint: disable=invalid-name
        # Reset script and css files after test
        if StrictVersion(__sphinx_version__) < StrictVersion('1.8.0'):
            StandaloneHTMLBuilder.script_files = \
                StandaloneHTMLBuilder.script_files[:3]
        if StrictVersion(__sphinx_version__) > StrictVersion('1.6.0'):
            # pylint: disable=no-name-in-module
            from sphinx.builders.html import CSSContainer
            StandaloneHTMLBuilder.css_files = CSSContainer()
            # pylint: enable=no-name-in-module
        elif StrictVersion(__sphinx_version__) < StrictVersion('1.8.0'):
            StandaloneHTMLBuilder.css_files = []

    @staticmethod
    def get_result(app, filename):
        path = os.path.join(app.outdir, filename+'.html')
        with open(path, 'r') as handle:
            return handle.read()

    @staticmethod
    def get_expectation(dirname, filename):
        provider = pkg_resources.get_provider(__name__)
        resource = '%s/%s.html' % (dirname, filename)
        if provider.has_resource(resource):
            return pkg_resources.resource_string(
                __name__, resource).decode('utf-8')
        result = []
        for i in range(10):
            resource_i = '%s.%d' % (resource, i)
            if provider.has_resource(resource_i):
                result.append(pkg_resources.resource_string(
                    __name__, resource_i).decode('utf-8'))
        return result

    def assertXMLEqual(  # pylint: disable=invalid-name
            self, expected, actual):
        if isinstance(expected, list):
            actual = normalize_xml(get_body(actual))
            for expected_candidate in expected:
                expected_candidate = normalize_xml(expected_candidate)
                if expected_candidate == actual:
                    return
            self.fail('XML does not match')
        else:
            expected = normalize_xml(expected)
            actual = normalize_xml(get_body(actual))
            self.assertEqual(expected, actual)

    def assertHasTabsAssets(  # pylint: disable=invalid-name
            self, xml):
        stylesheets = get_stylesheets(xml)
        scripts = get_scripts(xml)

        def filter_scripts(x):
            return x != 'documentation_options.js' and 'mathjax' not in x

        scripts = [x for x in scripts if filter_scripts(x)]
        self.assertEqual(stylesheets, [
            'alabaster.css',
            'pygments.css',
            'sphinx_tabs/tabs.css',
            'sphinx_tabs/semantic-ui-2.2.10/segment.min.css',
            'sphinx_tabs/semantic-ui-2.2.10/menu.min.css',
            'sphinx_tabs/semantic-ui-2.2.10/tab.min.css',
            'custom.css'
        ])
        self.assertEqual(scripts, [
            'jquery.js',
            'underscore.js',
            'doctools.js',
            'sphinx_tabs/tabs.js',
            'sphinx_tabs/semantic-ui-2.2.10/tab.min.js'
        ])

    def assertDoesNotHaveTabsAssets(  # pylint: disable=invalid-name
            self, xml):
        stylesheets = get_stylesheets(xml)
        scripts = get_scripts(xml)
        for stylesheet in stylesheets:
            self.assertTrue('sphinx_tabs' not in stylesheet)
        for script in scripts:
            self.assertTrue('sphinx_tabs' not in script)

    def assertStylesheetsEqual(  # pylint: disable=invalid-name
            self, expected, xml):
        actual = get_stylesheets(xml)
        self.assertEqual(expected, actual)

    def assertScriptsEqual(  # pylint: disable=invalid-name
            self, expected, xml):
        actual = get_scripts(xml)
        self.assertEqual(expected, actual)
