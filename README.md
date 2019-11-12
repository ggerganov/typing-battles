[![Build Status](https://travis-ci.org/ggerganov/typing-battles.svg?branch=master)](https://travis-ci.org/ggerganov/typing-battles?branch=master)

# typing-battles

A multiplayer typing game.

## Public server

https://typing-battles.ggerganov.com

## Screenshots

![Battle-Royale gameplay](https://github.com/ggerganov/typing-battles/raw/master/data/gameplay-0.gif "BR gameplay")

<a href="https://i.imgur.com/drL2D9E.png" target="_blank">![typing-battles](https://i.imgur.com/drL2D9E.png)</a>

<a href="https://i.imgur.com/YuTNEzt.png" target="_blank">![typing-battles](https://i.imgur.com/YuTNEzt.png)</a>

## Running a server

    $ git clone https://github.com/ggerganov/typing-battles --recursive
    $ cd typing-battles
    $ mkdir build && cd build
    $ cmake ..
    $ make
    
    $ ./src/typing-battles 3000
    
## Connecting to a server

Open a browser and point it the server's ip:port. For example https://127.0.0.1:3000/
