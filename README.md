# Extract Authenticode signature data from PE format file

[ [Japanese (日本語)](./README.ja.md) / English ]

[
https://github.com/trueroad/extract-authenticode
](https://github.com/trueroad/extract-authenticode)

This tool can extract Authenticode signature data from PE format file.
(e.g. .exe, .dll etc.)

## Required

For building this tool,

- Microsoft Visual Studio Express 2013 for Windows Desktop
- MinGW-W64

etc.

## Build

### Visual Studio

You run "nmake" to get "extract-authenticode.exe".

### MinGW-W64

```
gcc -municode -o extract-authenticode.exe extract-authenticode.c
```

## Usage

Example. To extract Authenticode data of explorer.exe,

```
extract-authenticode C:\Windows\explorer.exe out.bin
```

Example. To parse output file (required openssl),

```
openssl asn1parse -in out.bin -inform der -i
```

## License

Copyright (C) 2014, 2019 Masamichi Hosoda. All rights reserved.

License: BSD-2-Clause

See [LICENSE](./LICENSE).
