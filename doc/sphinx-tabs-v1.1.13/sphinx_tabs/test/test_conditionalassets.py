import unittest
import pkg_resources
from sphinx_testing import with_app
from .testcase import TestCase


class ConditionalAssetsTest(TestCase):
    @with_app(
        buildername='html',
        srcdir=pkg_resources.resource_filename(__name__, 'conditionalassets'))
    def test_build_html(
            self, app, status, warning):  # pylint: disable=unused-argument
        app.builder.build_all()
        for filename in ('index', 'other', 'other2'):
            actual = self.get_result(app, filename)
            expected = self.get_expectation('conditionalassets', filename)
            self.assertXMLEqual(expected, actual)
            if filename.startswith('other'):
                self.assertDoesNotHaveTabsAssets(actual)
            else:
                self.assertHasTabsAssets(actual)


if __name__ == '__main__':
    unittest.main()
