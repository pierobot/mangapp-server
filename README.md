# mangapp-server

Still in development and **probably** not ready for usage. Use at your own risk.

This is **not** an application to download manga from various sources. If that's what you're after, look elsewhere.

The website that uses this server is not ready yet, just the reader is left to finish, but it will be available here https://github.com/pierobot/mangapp-web

##Basic Usage
**``/mangapp/list``**: Retrieves a list of the available manga in the following JSON format:</br>
```
{
  "key": "",
  "name": ""
}
```
**``/mangapp/thumbnail/key``**: Retrieves the thumbnail image, if any, for the manga specified by ``key``</br></br>
**``/mangapp/details/key``**: Retrieves the details of the manga specified by ``key`` in the following JSON format:</br>
```
{
  "Result": "",
  "Id": "",
  "AssociatedNames": [ "" ],
  "Genres": [ "" ],
  "Authors": [ "" ],
  "Artists": [ "" ],
  "Year": "" 
}
```
**``/mangapp/files/key``**: Retrieves the available files, if any, for the manga specified by ``key`` in the following JSON format: </br>
```
{
  [ "", ""], // The first element contains the file's key and the second element contains the UTF-8 name
  [ "", ""],
  ...
}
```
**``/mangapp/reader/mangakey/filekey/index``**: Retrieves the image for the manga specified by ``mangakey``, the archive specified by ``filekey``, and the index specified by ``index``.

##Dependencies
icu:    http://site.icu-project.org/download </br>
boost:  http://www.boost.org/ </br>
zlib:   http://zlib.net/ </br>
libzip: http://www.nih.at/libzip/ </br>
unrar:  http://www.rarlabs.com/rar_add.htm </br>

##Build instructions
#####GNU/Linux
Install icu, zlib, libzip, unrar, and boost if you haven't already. </br>
```
sudo apt-get install build-essential libicu55 libicu-dev zlib1g zlib1g-dev libzip2 libzip-dev libboost-dev
```
Download, build, and install the unrar library.
```
wget http://www.rarlab.com/rar/unrarsrc-5.3.11.tar.gz && tar -zxvf unrarsrc-5.3.11.tar.gz && cd unrar && make lib && sudo make install-lib
```
Make sure ``unrar.hpp`` and ``dll.hpp`` are in an include directory.
This can be in either ``mangapp-server/include/unrar``, or ``/usr/local/include/unrar``.

Download this project and compile. 
```
git clone https://github.com/pierobot/mangapp-server && cd mangapp-server && cmake . && make
```

