# Documentation {#documentation}

## General

The entire documentation is done with doxygen.
Doxygen allows the generation of multiple kinds of documentation, while keeping external dependencies
and being manageable for users.

The documentation consists out of two parts, the user documentation and the internal documentation.
Both can be built independently or as a joint document.

For the required configuration steps, please read [here](@ref building)

This document describes the internal structure of the documentation and how to expand it.

## Adding a page

To add a page the following steps have to be taken:
1. Create a page.md file in tiscamera/doc/pages or a subfolder thereof.
2. Give the file a header with a unique id e.g. `# New page {#new-page}`  
   The id new-page will be used by doxygen to reference this page
3. Add the page to the variable 'doc_file_list' in tiscamera/doc/CMakeLists.txt  
   Do not forget the linebreak '\\' as the variable must be a single string.  
   Please be aware that the order in which the files are read in the CMakeLists.txt is important.
4. Recompile the project to generate a documentation with your new page.

## Snippets

To add code snippets to the documentation you can use the following command:

```
\snippet python/trigger-images trigger
```

The file python/trigger-images can be found in the folder that is defined as the doxygen samples folder.
For this project this is `tiscamera/examples`.

The words after the filename are the name of the snippet that shall be referenced.

In the trigger images example the comment

```
#! [trigger]
```

is used to begin and end the snippet.

For C and C++ the syntax would because

```
//! [trigger]
```
