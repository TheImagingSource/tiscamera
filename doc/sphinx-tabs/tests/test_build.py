import sys
import pytest
import sphinx
from sphinx.application import Sphinx


@pytest.mark.sphinx(testroot="basic")
def test_basic():
    pass


@pytest.mark.sphinx(testroot="notabs")
def test_no_tabs(app, check_asset_links):
    check_asset_links(app, cssPresent=False, jsPresent=False)


@pytest.mark.parametrize("docname", ["index", "no_tabs1", "no_tabs2"])
@pytest.mark.sphinx(testroot="conditionalassets")
def test_conditional_assets(app, docname, check_asset_links):
    if docname == "index":
        check_asset_links(app)
    else:
        check_asset_links(
            app, filename=docname + ".html", cssPresent=False, jsPresent=False
        )


@pytest.mark.noautobuild
@pytest.mark.parametrize("docname", ["index", "no_tabs1", "no_tabs2"])
@pytest.mark.sphinx(testroot="conditionalassets")
@pytest.mark.skipif(
    sphinx.version_info[:2] < (4, 1), reason="Test uses Sphinx 4.1 config"
)
def test_conditional_assets_html_assets_policy(
    app,
    docname,
    status,
    warning,
    check_build_success,
    get_sphinx_app_doctree,
    regress_sphinx_app_output,
    check_asset_links,
):
    app.set_html_assets_policy("always")

    # Following lines are copied from ``auto_build_and_check`` since we need to
    # set a config in the build object before auto build. Because of this, we
    # need to use ``noautobuild``.
    app.build()
    check_build_success(status, warning)
    get_sphinx_app_doctree(app, regress=True)
    regress_sphinx_app_output(app)

    check_asset_links(app, filename=docname + ".html")


@pytest.mark.sphinx(testroot="linenos")
@pytest.mark.skipif(
    sphinx.version_info[:2] >= (4, 0), reason="Test uses Sphinx 3 code blocks"
)
def test_other_with_assets(app, check_asset_links):
    check_asset_links(app)


@pytest.mark.sphinx(testroot="linenos")
@pytest.mark.skipif(
    sphinx.version_info[:2] <= (4, 0), reason="Test uses Sphinx 4 code blocks"
)
def test_other_With_assets_new_style(app, check_asset_links):
    check_asset_links(app)


@pytest.mark.sphinx(testroot="nestedmarkup")
def test_nested_markup(app, check_asset_links):
    check_asset_links(app)


@pytest.mark.sphinx(testroot="customlexer")
def test_custom_lexer(app, check_asset_links):
    check_asset_links(app)


@pytest.mark.noautobuild
@pytest.mark.sphinx("rinoh", testroot="rinohtype-pdf")
@pytest.mark.skipif(
    sys.version_info < (3, 8), reason="Unknown dependency conflict in lower versions"
)
def test_rinohtype_pdf(
    app, status, warning, check_build_success, get_sphinx_app_doctree
):
    app.build()
    check_build_success(status, warning)
    get_sphinx_app_doctree(app, regress=True)
    # Doesn't currently regression test pdf output


@pytest.mark.sphinx(testroot="disable-closing")
def test_disable_closing(app, check_asset_links):
    check_asset_links(app)


@pytest.mark.sphinx(testroot="disable-css-loading")
def test_disable_css_loading(app, check_asset_links):
    check_asset_links(app, cssPresent=False)
