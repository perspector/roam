# roam
![screenshot](/screenshot.png)

A nostalgic way to wander around the world. Displays street-level imagery in the terminal as ASCII.

Written in ANSI C.

Looks best in [cool-retro-term](https://github.com/Swordfish90/cool-retro-term) with a width of 182 characters and RGB colors enabled.

## Table of Contents
- [Installing](#installing)
- [Getting a Mapillary API Key](#getting-a-mapillary-api-key)
- [Usage](#usage)
- [Building](#building)
- [Credits](#credits)

## Installing
| Platform        | Supported? |
|-----------------|------------|
| Fedora 44       |    Yes     |
| Other GNU/Linux |  Probably  |
| Windows 11      |     No     |
| MacOS           |     No     |
* In the future, we may add support for other platforms. Contributions are always welcome!

### Quick-Setup
Just want the precompiled binary? Then head over to the [releases](https://github.com/perspector/roam/releases/latest) and download the appropriate file for your system.

### Full Download and Compile Yourself
Simply clone the project using `git clone https://github.com/perspector/roam`

Or alternatively, download the project as a .zip file and extract that to your computer.

Then navigate to the directory, run `make` to compile, and then continue following the instructions below except use `./roam` to run.

## Getting a Mapillary API Key
You will also need a Mapillary API token, which you can get by making a developer account at [Mapillary](https://www.mapillary.com/dashboard/developers).

Then just register a new application, add: app name, quick description, and company/university name. You can leave the company website and redirect URL blank.
Make sure to give READ access! Then, just copy the `Access Token`.

### Option 1:
Save your API access token to an environment variable:\
In GNU/Linux: `export MAPILLARY_TOKEN_ROAM="MLY|0123|456"`\
Replace `"MLY|0123|456"` with your access token.
### Option 2:
Enter the API access token when prompted by the program.

## Usage
Tested on Fedora 44 GNU/Linux.

Simply run with `./roam_v1_0_0_linux_amd64` or another version from the terminal.\
Alternatively, try double-clicking the file (may need permissions to execute as a program).
<!-- or `.\roam.exe` depending on your platform, or by double-clicking the file (may need permissions to execute as a program). -->

## Building
If you would like to build the code yourself, you can compile and run with
```
gcc -g -fsanitize=address -fno-omit-frame-pointer main.c ascii-view/src/image.c ascii-view/src/print_image.c -lcurl -lcjson -lm -Iascii-view/include && ./a.out
```

## Credits
- [Mapillary](https://mapillary.com) for excellent crowdsourced street-level imagery, also with a nice [API](https://www.mapillary.com/developer/api-documentation/)
    - Everyone who contributed to the platform!
- [Xander Gouws](https://github.com/gouwsxander) for an incredible ASCII viewer [ascii-view](https://github.com/gouwsxander/ascii-view)
    - His [video](https://youtu.be/t8aSqlC_Duo?si=wkXH4ePjsUNdruUc) on the topic is phenomenal
- [Nominatim](https://nominatim.org) for a wonderful geocoding API
