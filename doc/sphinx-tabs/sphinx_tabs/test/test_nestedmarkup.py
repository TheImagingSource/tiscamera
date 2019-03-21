import unittest
import pkg_resources
from sphinx_testing import with_app
from .testcase import TestCase


class NestedMarkupTest(TestCase):
    @with_app(
        buildername='html',
        srcdir=pkg_resources.resource_filename(__name__, 'nestedmarkup'))
    def test_build_html(
            self, app, status, warning):  # pylint: disable=unused-argument
        app.builder.build_all()
        actual = self.get_result(app, 'index')
        expected = self.get_expectation('nestedmarkup', 'index')
        self.assertHasTabsAssets(actual)
        self.assertXMLEqual(expected, actual)


if __name__ == '__main__':
    unittest.main()
