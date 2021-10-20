# Change Log

## 3.2.0 - 2021-06-11

### Fixed

* 🐛 Added default value for `exports` in JavaScript. Removes error that `exports` is not defined on page

### Added

* 👌 Use of [`sphinx` HTML assets policy](https://github.com/sphinx-doc/sphinx/issues/9115) to decide whether include assets

## 3.1.0 - 2021-06-11

### Added

* 👌 Added new sphinx config `sphinx_tabs_disable_css_loading` option to disable css from loading

## 3.0.0 - 2021-05-10

### Fixed

* 🐛 Update for compatibility with Sphinx 4

## 2.1.0 - 2021-03-10

### Fixed

*  🐛 Tabs with nested content becoming hidden on click

### Improved

* 👌 Added new sphinx config option `sphinx_tabs_disable_tab_closing` to allow new tab closing functionality to be disabled globally (better backwards compatibility with sphinx-tabs<2)


## 2.0.1 - 2021-02-07

### Fixed

* Nested parsing of tab labels, to support use of reST roles within label text
* Parsing tabs with unsupported builders, including for pdf

### Added

* Test for successful pdf building using `rinohtype`

## 2.0.0 - 2021-01-24

♻️ Refactor to reduce JS/CSS payload size and improve accesibility:

* Removed semantic-ui assets
* Removed unused 'sphinx_tabs_nowarn' sphinx option
* JS/CSS assets are now copied across by sphinx when builing, rather than being copied by the extension
* Changed tab HTML to use tab roles
* Changed tab label colour, to increase contrast with background
* Added ARIA labels for tabs and panels
* Added tabindex atributes to allow users to focus and switch tabs using a keyboard
* Added a margin below images inside tab content

✨ New features:

* Selecting an open tab now closes the tabs panel
* The last selected group tab persists between pages (if the browser supports session storage)

## 1.3.0 - 2020-09-08

♻️ Major refactor of tabs, group-tabs and code-tabs (thanks to [@foster999](https://github.com/foster999)):

* Removes hard-coded reStructuredText for group and code tabs, so that this works with other parsers (inc `MyST Markdown`)
* group-tab directive now subclasses `TabDirective` to remove duplicated run code
* likewise, `code-tab` now subclasses `group-tab`
* `TabDirective` and `TabsDirective` now subclass `SphinxDirective` for easier access to directive `env`

🧪 testing infrastructure has now fully moved from unittest to pytest

✨ New features:

* Can now pass `code-tabs` with a second argument (allowing whitespace) to provide an alternative tab label
* `code-tabs` can now use custom lexers, which are added to the sphinx app in `conf.py`

## v1.2.1 - 2020-08-20

🐛 FIX: Remove `app.add_javascript` use for sphinx v2

## v1.2.0 - 2020-08-18

`sphinx-tabs` has now moved to the [EBP organisation](https://executablebooks.org) 🎉

### Deprecated 🗑

- Python < 3.5 and Sphinx < 2 support dropped

### Improved 👌

- Lots of code and CI restructuring (see [EBP Migration (#76)](https://github.com/executablebooks/sphinx-tabs/commit/6342ed3f1f7d4cb50891001f26d4e3c4c08ee422))

### Fixed 🐛

- Replace `add_javascript` by `add_js` for sphinx>=3 (removes warning!),
  thanks to [@Daltz333](https://github.com/Daltz333)

## v1.1.13

- Marked the extension as parallel safe (#46)

## v1.1.12

- Fix bug in Internet Explorer (#33)
- Drop support for Python 3.4 (as lxml no longer supports it)

## v1.1.11

- Support for Sphinx 2
- Update to Semantic UI 2.4.1
- Fix oversize tabs in Sphinx 2 (#38)
- Various bug fixes

## v1.1.10

- Fix group tabs behaviour (#31)

## v1.1.9

- Fix copying of tab assets (JS and CSS) when module is installed as a python egg
- Make tabs wrap on narrow screens (#30)
- Add support for nested tabs (#29)

## v1.1.8

- Add support for :linenos: to code tabs (#22)
- Improve fallback for non-HTML builders (#19)
- Add spelling builder to list of builders (#20)
- Fix naming a tab as a number (#24)

## v1.1.7

- Fix css files not being removed when tabs are not used on a page
- Now require sphinx>=1.4
- Fix issues with script and css includes on readthedocs (#17)

## v1.1.6

- Don't emit an additional error in the copy assets handler if the build process fails

## v1.1.5

- Add sphinx_tabs_nowarn option to disable warning about incompatible builder. Useful when running sphinx-build with the -W flag

## v1.1.4

- Fix broken javascript file inclusion

## v1.1.3

- Insert CSS and JS files after custom ones add in conf.py

## v1.1.2

- Add support for dirhtml builder

## v1.1.1

- Allow multiple group tabs with the same name (the ordering of the tabs is used to determine the group ids)
- Use b64 encoding of group id for group tabs to avoid illegal character in CSS class

## v1.1.0

- Fix issue with multiple words in tab titles
- Add support for markup in tab titles
- Fix graphical glitches caused by illegal characters in group tab ids

## v1.0.1

- Fix possible exception when adding/removing CSS/JS files from the context

## v1.0.0

- Initial version
