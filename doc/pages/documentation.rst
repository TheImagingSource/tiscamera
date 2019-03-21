#############
Documentation
#############

This document describes the internal structure of the documentation and how to expand it.

General
=======

The entire documentation is done with doxygen.
Doxygen allows the generation of multiple kinds of documentation, while keeping external dependencies minimal and manageable for users.

The documentation consists out of two parts, the user documentation and the internal documentation.
Both can be built independently or as a joint document.

For the required configuration steps, please read [the build instructions](@ref building).

Adding a page
=============

To add a page the following steps have to be taken:

1. Create a page.rst file in tiscamera/doc/pages or a subfolder thereof.
2. Give the file a header with a unique id e.g.

   .. code-block:: rst
                   
      ########
      New page
      ########
      
3. Add the page to the variable 'doc_file_list' in tiscamera/doc/CMakeLists.txt
   Do not forget the linebreak '\\' as the variable must be a single string.
   Please be aware that the order in which the files are read in the CMakeLists.txt is important.
4. Recompile the project to generate a documentation with the new page.

Images that are only used in the documentation should be stored in doc/images

