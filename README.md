# roam
![screenshot](/screenshot.jpg)

A nostalgic way to wander around the world. Displays street-level imagery in the terminal as ASCII.

Programmed in ANSI C.

## Table of Contents
- [Installing](#installing)
- [Usage](#usage)
- [Building](#building)
- [Credits](#credits)

## Installing
Simply clone the project using `git clone https://github.com/perspector/roam`

Or alternatively, download the project as a .zip file and extract that to your computer.

## Usage
Tested on Fedora 44 GNU/Linux. Looks best in [cool-retro-term](https://github.com/Swordfish90/cool-retro-term) with RGB colors enabled.

Simply run with `./roam` , or by double-clicking the file (may need permissions to execute as a program).

## Building
If you would like to build the code yourself, you can compile and run with `gcc -g -fsanitize=address -fno-omit-frame-pointer main.c ascii-view/src/image.c ascii-view/src/print_image.c -lcurl -lcjson -lm -Iascii-view/include && ./a.out`

## Credits
- [Mapillary](https://mapillary.com) for excellent crowdsourced street-level imagery, also with a nice [API](https://www.mapillary.com/developer/api-documentation/)
    - Everyone who contributed to the platform!
- [Xander Gouws](https://github.com/gouwsxander) for an incredible ASCII viewer [ascii-view](https://github.com/gouwsxander/ascii-view)
    - His [video](https://youtu.be/t8aSqlC_Duo?si=wkXH4ePjsUNdruUc) on the topic is phenomenal
- [Nominatim](https://nominatim.org) for a wonderful geocoding API
