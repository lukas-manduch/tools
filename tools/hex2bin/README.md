# Hex2bin: Raw hex to binary converter

This is simple tool, that I didn't find anywhere in GNU/Linux environment.

Hex2bin converts hex values from input/file and writes them as binary values to
specified file. All non hex values are ignored.

Warning: If file contains for example `0x1234` this will be interpreted as
`0123`, because first zero will be taken as part of hex number. Character '4'
will be ignored, because it is missing a pair. 

If file contains non hex and non whitespace characters, hex2bin will report
first occurence and continue processing file.

