# libInanna

Marko Grönroos 1998-2005.

libInanna is an Artificial Neural Network computation and learning library for research purposes.
It uses an object-oriented neural model that allows complex structures.

For learning, Backpropagation and RProp are supported.

The library requires [MagiCLib++](/magi42/magiclib).
It is expected to be compiled under the MagiCLib++ source tree, to be able to use and develop the base library more easily.
To have it compile there, you need to include it in the `Makefile` of MagiCLib++, with the `makemodules` parameter:

```
makemodules = libmagic libinanna
```

Then, you can run `make` to compile both MagiCLib++ and `libinanna`.

## Projects

Under the `projects`, there are applications using the library.

* `prediction` for making time series predictions
