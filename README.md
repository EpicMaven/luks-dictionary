# luks-dictionary
luks-dictionary performs a LUKS password test on a device using a password
dictionary.  This is useful for testing the strength of a LUKS password, or
recovering a password when forgotten (but mostly remembered).

## Build
```bash
cd /path/to/checkout
qmake-qt5
make
```

## Usage
```bash
./luks-dictionary [options] passwordFilename failFilename device
```

## License
Artistic license 2.0
