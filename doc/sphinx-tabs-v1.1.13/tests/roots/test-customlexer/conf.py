from pygments.lexer import RegexLexer
from pygments.token import Text, Keyword

project = "sphinx-tabs test"
master_doc = "index"
source_suffix = ".rst"
extensions = ["sphinx_tabs.tabs"]
pygments_style = "sphinx"


class BYOLexer(RegexLexer):
    name = "BYO"
    aliases = ["byo"]
    filenames = ["*.byo"]
    tokens = {
        "root": [
            (r"Test\n", Keyword),
            (r".*\n", Text),
        ]
    }


def setup(app):
    try:
        app.add_lexer("byo", BYOLexer)
    except:
        # Passing instance is depreciated in Sphinx 4
        app.add_lexer("byo", BYOLexer())
