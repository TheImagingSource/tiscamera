import os
import pytest
from pathlib import Path
from bs4 import BeautifulSoup
from sphinx.testing.path import path

from sphinx_tabs.tabs import FILES

pytest_plugins = "sphinx.testing.fixtures"


@pytest.fixture(scope="session")
def rootdir():
    """Pytest uses this to find test documents."""
    return path(__file__).parent.abspath() / "roots"


@pytest.fixture(autouse=True)
def auto_build_and_check(
    app,
    status,
    warning,
    check_build_success,
    get_sphinx_app_doctree,
    regress_sphinx_app_output,
    request,
):
    """
    Build and check build success and output regressions.
    Currently all tests start with this.
    Disable using a `noautobuild` mark.
    """
    if "noautobuild" in request.keywords:
        return
    app.build()
    check_build_success(status, warning)
    get_sphinx_app_doctree(app, regress=True)
    regress_sphinx_app_output(app)


@pytest.fixture()
def manual_build_and_check(
    app,
    status,
    warning,
    check_build_success,
    get_sphinx_app_doctree,
    regress_sphinx_app_output,
):
    """
    For manually triggering app build and check.
    """
    app.build()
    check_build_success(status, warning)
    get_sphinx_app_doctree(app, regress=True)
    regress_sphinx_app_output(app)


@pytest.fixture
def check_build_success():
    """Check build is successful and there are no warnings."""

    def check(status, warning):
        assert "build succeeded" in status.getvalue()
        warnings = warning.getvalue().strip()
        assert warnings == ""

    return check


@pytest.fixture
def regress_sphinx_app_output(file_regression, get_sphinx_app_output):
    """
    Get sphinx HTML output and regress inner body (other sections are
    non-deterministic).
    """

    def read(
        app, buildername="html", filename="index.html", encoding="utf-8", replace=None
    ):
        content = get_sphinx_app_output(app, buildername, filename, encoding)

        if buildername == "html":
            soup = BeautifulSoup(content, "html.parser")
            doc_div = soup.findAll("div", {"class": "documentwrapper"})[0]
            doc = doc_div.prettify()
            for find, rep in (replace or {}).items():
                doc = text.replace(find, rep)
        else:
            doc = content
        file_regression.check(
            doc, extension="." + filename.split(".")[-1], encoding="utf8"
        )

    return read


@pytest.fixture
def get_sphinx_app_doctree(file_regression):
    """Get sphinx doctree and optionally regress."""

    def read(app, docname="index", resolve=False, regress=False, replace=None):
        if resolve:
            doctree = app.env.get_and_resolve_doctree(docname, app.builder)
            extension = ".resolved.xml"
        else:
            doctree = app.env.get_doctree(docname)
            extension = ".xml"

        # convert absolute filenames
        for node in doctree.traverse(lambda n: "source" in n):
            node["source"] = Path(node["source"]).name

        if regress:
            text = doctree.pformat()  # type: str
            for find, rep in (replace or {}).items():
                text = text.replace(find, rep)
            file_regression.check(text, extension=extension)

        return doctree

    return read


@pytest.fixture
def check_asset_links(get_sphinx_app_output):
    """
    Check if all stylesheets and scripts (.js) have been referenced in HTML.
    Specify whether checking if assets are ``present`` or not ``present``.
    """

    def check(
        app,
        buildername="html",
        filename="index.html",
        encoding="utf-8",
        cssPresent=True,
        jsPresent=True,
    ):
        content = get_sphinx_app_output(app, buildername, filename, encoding)

        css_assets = [f for f in FILES if f.endswith(".css")]
        js_assets = [f for f in FILES if f.endswith(".js")]

        soup = BeautifulSoup(content, "html.parser")
        stylesheets = soup.find_all("link", {"rel": "stylesheet"}, href=True)
        css_refs = [s["href"] for s in stylesheets]

        scripts = soup.find_all("script", src=True)
        js_refs = [s["src"] for s in scripts]

        all_refs = css_refs + js_refs

        if cssPresent:
            css_present = all(any(a in ref for ref in all_refs) for a in css_assets)
            assert css_present
        else:
            assert not "sphinx_tabs" in css_refs
        if jsPresent:
            js_present = all(any(a in ref for ref in js_refs) for a in js_assets)
            assert js_present
        else:
            assert not "sphinx_tabs" in js_refs

    return check


@pytest.fixture()
def get_sphinx_app_output():
    """Get content of a sphinx app output file."""

    def get(app, buildername="html", filename="index.html", encoding="utf-8"):
        outpath = Path(app.srcdir) / "_build" / buildername / filename
        if not outpath.exists():
            raise IOError("No output file exists: {}".format(outpath.as_posix()))

        return outpath.read_text(encoding=encoding)

    return get
