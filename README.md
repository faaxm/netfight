# Netfight

An amazing multiplayer 2D shooter... Or at least it could be ;)

<p align="center">
    <img alt="Netfight Screenshot" src="https://www.f-ax.de/ext_files/netfight_screenshot.png">
</p>

A very old project that is now up for some modernization...
Originally coded this in a week or so while attending a numerical computation seminar at uni,
fixing the urgent deficit of multiplayer games on the uni's linux terminals... ;-)

The plan is to get this code to compile on linux and mac, use cmake instead of the
improvised make file and eventually cleanup the code a bit and add more stuff.
Also, the network code probably doesn't work when running the clients on different platforms.

If you want to create your own map, you can do so by creating a special svg file
(e.g. in Inkscape) and then using a perl script to convert it to a `.lvl` file.

Graphics are also converted from svg using a perl script, but this time they are
converted to c source code with opengl commands.

## How to run the game

The game can either be started in server or in client mode. The game cannot run without connecting to a server.
Always run the game in the parent of the `data` directory.

To start a server, use the `-s` command line flag:
```
$ ./netfight -s
```

Without the `-s` flag, the game will start in client mode and by default connect to `localhost`. Use `-c` to
connect to a different host:
```
$ ./netfight -c netfight.example.net
```

## Keys

| Action        | Key                |
| ------------- | ------------------ |
| Basic Move    | `Arrows`, `Numpad` |
| Strafe        | `A`, `S`, `1`, `3` |
| Shoot         | `Space`            |
| Change Weapon | `Q`, `7`           |
| Change Item   | `W`, `9`           |
| Use Item      | `Shift`            |
| Show Score    | `Tab`              |

## Requirements

* OpenGL
* GLFW3
* Enet