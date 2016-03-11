# mangapp-server

OS | Compiler | Status
---|----------|--------
Linux (Ubuntu 14.04)|g++ 4.8.2|[![Build Status](https://travis-ci.org/pierobot/mangapp-server.svg?branch=master)](https://travis-ci.org/pierobot/mangapp-server)|

Still in development and **probably** not ready for usage. Use at your own risk.

This is **not** an application to download manga from various sources. If that's what you're after, look elsewhere.

##Features
* Support for the following archive formats:
 * cbz/zip
 * cbr/rar
 * cb7/7z
* Retrieval of details from mangaupdates
* TLS/SSL (OpenSSL)


##Dependencies
icu:    http://site.icu-project.org/download </br>
boost:  http://www.boost.org/ </br>
openssl: https://github.com/openssl/openssl </br>
zlib:   http://zlib.net/ </br>
libzip: http://www.nih.at/libzip/ </br>
unrar:  http://www.rarlabs.com/rar_add.htm </br>
mstch:  https://github.com/no1msd/mstch </br>
opencv: https://github.com/itseez/opencv </br>

##Build instructions
#####GNU/Linux
https://github.com/pierobot/mangapp-server/wiki/Build-on-GNU-Linux

#####Windows
https://github.com/pierobot/mangapp-server/wiki/Build-on-Windows
