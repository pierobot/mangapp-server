# mangapp-server

Still in development and **probably** not ready for usage. Use at your own risk.

This is **not** an application to download manga from various sources. If that's what you're after, look elsewhere.

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
boost:  http://www.boost.org/ </br>
zlib:   http://zlib.net/ </br>
libzip: http://www.nih.at/libzip/ </br>
unrar:  http://www.rarlabs.com/rar_add.htm </br>
