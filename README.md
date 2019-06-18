# typing-battles

A multiplayer typing game.

<a href="https://i.imgur.com/drL2D9E.png" target="_blank">![typing-battles](https://i.imgur.com/drL2D9E.png)</a>

<a href="https://i.imgur.com/YuTNEzt.png" target="_blank">![typing-battles](https://i.imgur.com/YuTNEzt.png)</a>

![Battle-Royale gameplay](https://github.com/ggerganov/typing-battles/raw/master/data/gameplay-0.gif "BR gameplay")



## Running a server

    $ git clone https://github.com/ggerganov/typing-battles --recursive
    $ cd typing-battles
    $ mkdir build && cd build
    $ cmake ..
    $ make
    
    $ ./src/main 3000
    
## Connecting to a server

Open a browser and point it the server's ip:port. For example http://127.0.0.1:3000/
