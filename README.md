# ascii-tetris

<img src=docs/preview.gif width=400px>

## Installation and usage

### Linux

Open a terminal and execute the following steps:

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev # ncurses is the only required dependency
git clone https://github.com/pablozabo/ascii-tetris && cd ascii-tetris
make
cd build/debug/bin
./tetris
```

### Windows

First download and install [mingw](https://www.mingw-w64.org/downloads/#w64devkit).

**NOTE:** there's a dependency to pdcurses library (ncureses port for Windows platform) that it's
already compiled and located on `ascii-snake\external\pdcurses\lib` folder. You can also
download the source code from `https://github.com/wmcbrine/PDCurses` and compile it.

Finally open a terminal and execute the following steps:

```bash
git clone https://github.com/pablozabo/ascii-tetris && cd ascii-tetris
make
cd build\debug\bin
.\tetris.exe
```

### Controls:

- <kbd>↑</kbd> shape rotation
- <kbd>→</kbd> move right
- <kbd>←</kbd> move left
- <kbd>↓</kbd> speed up
- <kbd>SPACE</kbd> hard drop
- <kbd>P</kbd> pause
- <kbd>S</kbd> shape shadow (easy mode)
- <kbd>ESC or F1</kbd> exit
