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
Install icu, boost, zlib, libzip, and unrar if you haven't already. </br>
```
sudo apt-get install build-essential libicu-dev libboost-dev zlib1g-dev libzip-dev 
```
Download, build, and install the unrar library.
```
wget http://www.rarlab.com/rar/unrarsrc-5.3.11.tar.gz && tar -zxvf unrarsrc-5.3.11.tar.gz && cd unrar && make lib && sudo make install-lib && sudo mkdir /usr/local/include/unrar && sudo cp *.hpp /usr/local/include/unrar
```
Download this project and compile. 
```
git clone https://github.com/pierobot/mangapp-server && cd mangapp-server && cmake . && make
```

#####Windows (VS 2013)
Install CMake for Windows if you haven't already. https://cmake.org/download/

For the sake of organization, create an easily accessible folder with the following structure:
```
mylibs
  icu (if using precompiled)
    include
    lib
  boost (if using precompiled)
    include
    lib
  zlib
    include
    lib
  libzip
    include
    lib
  unrar
    include
      unrar
    lib
```

Download zlib from http://zlib.net/. Once you've extracted the archive, go inside the folder and then into ``contrib/vstudio/v11`` and open the solution file ``zlibvc.sln``. </br>
Set the configuration type to ``ReleaseWithoutAsm`` and set the platform to whatever your heart desires. </br>
From the Solution Explorer on the left hand side, right-click ``zlibstat``, and click ``Build``. </br>
If all went right, the output log should indicate the path of the resulting ``zlibstat.lib`` file. </br>
Copy ``zlibstat.lib`` to the folder we created earlier ``mylibs/zlib/lib``. </br>
Go back three directories into the zlib folder you extracted. You should see ``zconf.h`` and ``zlib.h``. Copy these files to ``mylibs/zlib/include``. </br>

Download libzip from http://www.nih.at/libzip/. Once you've extracted the archive, go into the folder and then into lib. Open ``CMakeLists.txt``, find the following lines, and remove all the ``#`` characters: 
```
#ADD_LIBRARY(zipstatic STATIC ${LIBZIP_SOURCES} ${LIBZIP_EXTRA_FILES} ${LIBZIP_OPSYS_FILES})
#SET_TARGET_PROPERTIES(zipstatic PROPERTIES VERSION 3.0 SOVERSION 3 )
#TARGET_LINK_LIBRARIES(zipstatic ${ZLIB_LIBRARY})
#INSTALL(TARGETS zipstatic
#  RUNTIME DESTINATION bin
#  ARCHIVE DESTINATION lib
#  LIBRARY DESTINATION lib)
```
Open the CMake GUI and supply it with the source directory. Add ``/build`` to the ``where to build the binaries`` path.</br>
Click ``Configure`` and an error should come up saying ``Could NOT find ZLIB (missing: ZLIB_LIBRARY ZLIB_INCLUDE_DIR)``. </br>
You should see a bunch of red items in a list, look for ``ZLIB_LIBRARY``, click on it and then click on the ``...`` button to the right most side. Point it to ``mylibs/zlib/lib/zlibstat.lib``. </br>
Find ``ZLIB_INCLUDE_DIR`` and point it to ``mylibs/zlib/include``. </br>
Click ``Configure`` once again. This time you should not get any errors. Now click on ``Generate`` button. </br>
From the current libzip directory, go into the lib folder, and you should see ``zip.h``. Copy it to ``mylibs/libzip/include``. </br>
Go into the build directory and copy ``zipconf.h`` to ``/mylibs/libzip/include``. </br>
Go to the source path and into the ``build`` folder, open the solution file ``libzip.sln``. </br>
Set the configuration type to Release, and the platform to what you chose with zlib. </br>
Build the library with the same steps you used for zlib. </br>
Copy ``zipstatic.lib`` to ``/mylibs/libzip/lib``

Download the unrar source from http://www.rarlab.com/rar_add.htm. Extract it. Go into the unrar folder and open ``UnRARDll.vcproj``.</br>
In the Solution Explorer, right click the UnRAR project, and click on Properties. Change the Configuration to Release and set the Platform to the same as zlib and libzip. </br>
Under General, change the target extension to ``.lib`` and under Project Defaults, the Configuration Type to ``Static library``.
Build the library, copy all the ``.hpp`` files to ``mylibs/unrar/include/unrar``, and copy ``UnRAR.lib`` to ``/mylibs/unrar/lib``.

Google how to build Boost with ICU on Windows and copy the appropriate headers and libraries to their respective ``/lib/xxx/include`` and ``/lib/xx/lib`` path.
If you do not want to build icu or boost from source, you can find precompiled libraries here: </br>
    http://www.npcglib.org/~stathis/blog/precompiled-boost/ </br>
    http://www.npcglib.org/~stathis/blog/precompiled-icu/ </br>
    Make sure you download the ``MSVC 2013`` versions. </br>

We now have all the library dependencies taken care of.
Clone this repository somewhere, open the CMake GUI and point the source directory to it. Like before, add ``/build`` to the ``where to build the binaries`` path. </br>
Click on Configure and you'll get some errors. Do the same as we did earlier and point each individual missing path to the appropriate ones we created in ``/mylibs``. </br>
If all went well, there should no longer be any errors. Click on Generate and open ``mangapp-server.sln`` in the ``build`` directory. </br>
Set the configuration type to Release and the platform to the same as earlier.
Build the project and if all went well, you should have a shiny ``mangapp-server.exe``.
