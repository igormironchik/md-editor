This is a `Markdown` editor for Qt 6.

Almost ready for use, there some minor issues with `QWebEngineView` in Qt 6.4.3,
but you can play with it already.

This application can be used from command line as rendered `HTML` preview.

And one feature - this application can load all linked `Markdown` files from root
document to preview all, for example, `GitHub` `Markdown` pages as one `HTML` book,
this is quite usefull.

And a cherry on the cake - `LaTeX Math` injections are rendered in preview. Look at the screenshots.

One more, guys, you can convert your `Markdown` document into `PDF` from the `File` menu. To activate
this option you need to place `md-pdf-gui` executable in the same folder with `md-editor`
executable. Sources of `md-pdf-gui` you can get [here](https://github.com/igormironchik/md-pdf).

Thanks.

# Installers

You can get `x64` installers for `Linux` and `Windows` [here](https://github.com/igormironchik/markdown).

# Building

When you cloned repository don't forget to update submodules!

```
git submodule update --init --recursive
```

Build process is quite easy, just use `QtCreator` to open `CMakeLists.txt` and build in IDE, or use
`CMake` from a shell.

# Screenshots

![md-editor](md-editor-edit-mode.png)

![md-editor](md-editor-view-mode.png)

![md-editor](md-editor-latext-math.png)
