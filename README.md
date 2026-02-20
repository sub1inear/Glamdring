# Glamdring, Chess Engine
Glamdring is UCI-compatible chess engine written in C++, written for learning. It can run on x86-64 and ARM (or any other architecture).

Features:
* UCI (subset)
* Alpha-beta Pruning with Move Ordering
* Piece-Square Tables-Based Evalutaion
* PEXT bitboards (for portability, emulated on other architectures)
* Transposition Table with Zobrist Hashing
* Polyglot Opening Books
    * Defaults uses `Titans.bin` from https://github.com/gmcheems-org/free-opening-books
* Cross-Platform Support


Build:
```bash
git clone https://github.com/sub1inear/Glamdring
mkdir build
cd build
cmake ..
```

Usage:
```
uci
id name Glamdring
id author sublinear
uciok
position startpos
go depth 10
...
```
