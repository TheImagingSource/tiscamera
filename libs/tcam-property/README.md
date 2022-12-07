# tcam-property

Linux gobject introspection interface.

This is the standalone version of the tcam-property API in tiscamera.

Use this for your own libraries or for TIS closed source tools.

## Build

### Dependencies

Dependency versions are based on the current reference system (Ubuntu 18.04 LTS)

libglib2.0-0 (>= 2.48.2)
libgirepository-1.0-1 (>= 1.46.0)
libgirepository1.0-dev

To install all compilation dependencies, execute:

    sudo apt install libglib2.0-dev libgirepository1.0-dev

### Compile

```
    mkdir build
    cd build
    cmake ..
    make
    make package
    sudo apt install ./tiscamera-tcamproperty*.deb
```

## Remove

```
    sudo apt remove tiscamera-tcamproperty
```

## Replacement

tiscamera-tcamproperty and tiscamera provide the tcam-property interface.
Installing the tiscamera debian package will automatically
remove installed tiscamera-tcamproperty packages.

## License

All files in this project have to be considered open source under the Apache 2.0 license.

New files have to contain the apache 2.0 license header
