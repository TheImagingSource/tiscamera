project = "sphinx-tabs test"
master_doc = "index"
source_suffix = ".rst"
extensions = ["sphinx_tabs.tabs", "rinoh.frontend.sphinx"]
pygments_style = "sphinx"

rinoh_documents = [
    dict(doc=master_doc, target="test"),
]
