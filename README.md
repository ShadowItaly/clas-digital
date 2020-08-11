# clas-digital
A C++ implementation of a webserver providing a login/search/upload implementation to catalogue and categorize texts.

<br/>
<br/>

## Table of contents
1. [Installation](#Installation)
2. [Programming concept](#Programming-concept)

<br/>
<br/>

## Installation
After a bit of consideration we decided to use the package manager [conan](#https://conan.io) to manage the dependencies. 

__Why use a package manager at all?__

When developing a huge project like clas-digital one tends to use a lot of third party libraries. Clas-digital for example uses tesseract. Tesseract depends on leptonica which depends on libtiff and libjpeg and libpng. As one can see it is a lot of work to always install all dependencies. Not even mentioning the pitfalls when intalling on diffrent linux distributions. In order to improve this dire process we decided to use a package manager. Conan seems to be one of the most advanced cross plattform package managers there are for C++.

__Compilation has not been tested on Windows and Mac!__

Installation prerequisites:
- Working C++ compiler which supports at least C++17. Recommended (Linux: GCC), (Windows: CLang), (Mac: Clang)
- Python 3 and pip
- CMake version 2.8 or newer

__Instructions for Linux like systems:__
```
mkdir build
cd build
conan install .. cppstd=17
cmake ..
cmake --build .
cmake . --target install
```

<br/>
<br/>

## Programming concept
The programm itself is not a standalone webserver. It does the login management the authorization of content and handles searching and uploading of content. While the doing this, it would be unnecessary overload to also meddle with serving files and caching static content. In order to provide a full featured website one has to use another webserver like nginx or apache to serve the static content.
### Book Manager
### Book
### CMetadata

The programming concept part of the document should introduce the reader a bit to the logic and structure of the code of clas digital. This should help further maintainers to get themselves acquainted with the code. 

### Book Management


C++ implementation of a ancient literature database




map liste mit buechern sortieren

# Updating clas-digital

Make install will install to the devlopment version by default
~~~~
make install 
~~~~

If one needs to recreate the static files generated by the server once a day he/she has to exeute the following command first
~~~~
make reset_static
~~~~

One can also seperately recreate the catalogue only or the book static files only by issuing
~~~~
make reset_static_books
make reset_static_catalogue
~~~~

## For installing to the stable version
~~~~
make install_stable
~~~~
and 
~~~~
make reset_static_stable
~~~~
and
~~~~
make reset_static_books_stable
make reset_static_catalogue_stable
~~~~

# Installation
Warning executing "sudo make install" overrides your current nginx settings, creates a new folder in /etc/clas-digital and registers /etc/clas-digital/clas-digital.o as service with the name clas-digital. Only proceed when you know what you are doing!
~~~~
git clone https://github.com/ShadowItaly/clas-digital
cd clas-digital
./install.sh
cd build
cmake ..
make
sudo make install
~~~~

S7EDM6SZ
HC8T3DTG

# ToDo-List
Filter results nach Haeufigkeit
old fuzzy search name change
alphatical bibliography
copyright show bibliographic data
snippet ocr for copyright books
no hits on search
hits over multiple pages while searching
Update from zotero replace by zotero
create map of all words
download ocrs by selecting books in the search funktion


style apple



# Style ideas
scorlling of ocr while scrolling pages
stay on same page
open link in new tab
change homepage
color rethinking, easier reading
remove a bit of overwriting
cant print on tuesdays

# To recreate certificate request
openssl req -newkey rsa:2048 -sha256 -outform PEM -out certreq.pem


#Nginx fancy index
