sphinx-tabs [![Build Status](https://travis-ci.org/djungelorm/sphinx-tabs.svg?branch=master)](https://travis-ci.org/djungelorm/sphinx-tabs)
========================================

Create tabbed content in [Sphinx documentation](http://www.sphinx-doc.org) when building HTML.

For example, see the [Raw] code of [test/index.rst](test/index.rst) which generates the following:

A live demo can be found here: https://djungelorm.github.io/sphinx-tabs/

![Tabs](/images/tabs.gif)

Installation
----------------------------------------

```bash
pip install sphinx-tabs
```

To enable the extension in Sphinx, add the following to your conf.py:

```python
extensions = ['sphinx_tabs.tabs']
```

Basic Tabs
----------------------------------------

Basic tabs can be coded as follows:

```rst
.. tabs::

   .. tab:: Apples

      Apples are green, or sometimes red.

   .. tab:: Pears

      Pears are green.

   .. tab:: Oranges

      Oranges are orange.
```

![Tabs](/images/tabs.gif)

Grouped Tabs
----------------------------------------

Tabs can be grouped, so that changing the current tab in one area changes the current tab in the
another area. For example:

```rst
.. tabs::

   .. group-tab:: Linux

      Linux Line 1

   .. group-tab:: Mac OSX

      Mac OSX Line 1

   .. group-tab:: Windows

      Windows Line 1

.. tabs::

   .. group-tab:: Linux

      Linux Line 1

   .. group-tab:: Mac OSX

      Mac OSX Line 1

   .. group-tab:: Windows

      Windows Line 1
```

![Group Tabs](/images/groupTabs.gif)

Code Tabs
----------------------------------------

Tabs containing code areas with syntax highlighting can be created as follows:

```rst
.. tabs::

   .. code-tab:: c

         int main(const int argc, const char **argv) {
           return 0;
         }

   .. code-tab:: c++

         int main(const int argc, const char **argv) {
           return 0;
         }

   .. code-tab:: py

         def main():
             return

   .. code-tab:: java

         class Main {
             public static void main(String[] args) {
             }
         }

   .. code-tab:: julia

         function main()
         end

   .. code-tab:: fortran

         PROGRAM main
         END PROGRAM main
```

![Code Tabs](/images/codeTabs.gif)
