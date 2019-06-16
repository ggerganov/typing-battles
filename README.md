# typing-battles

A multiplayer typing game.

<a href="https://i.imgur.com/Alm6Cvj.png" target="_blank">![typing-battles](https://i.imgur.com/Alm6Cvj.png)</a>

## Running a server

    $ git clone https://github.com/ggerganov/typing-battles --recursive
    $ cd typing-battles
    $ mkdir build && cd build
    $ cmake ..
    $ make
    
    $ ./src/main 3000
    
## Connecting to a server

Open a browser and point it the server's ip:port. For example http://127.0.0.1:3000/
