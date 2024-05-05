# Kero Modular Server Engine

This project is component based modular server engine.  
It was implemented from scratch using `C++ Standard 20` and `Linux epoll`.

The example is a 1:1 matching multiplayer [Rock, Paper, Scissors, Lizard, Spock](examples/rock_paper_scissors_lizard_spock/README.md) game.  

## Features

- Component based modular system
- Async IO event loop system
- Actor system
- Asynchronous structured logging system

## Prerequisites

### Python

`python` >= 3.8

The build script was written in `Python`.

### CMake

`cmake` >= 3.28

### Ninja

`ninja` >= 1.10

### cmake-format

`cmake-format` >= 0.6

```sh
pip install cmake_format
```

## How to Build

```sh
python dev.py build
```
