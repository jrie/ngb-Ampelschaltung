# ngb-Ampelschaltung
Project "traffic lights/Ampelschaltung" for ngb coding competition

This was the initial commits for the the first coding competition of the german ngb board website.
Primary goal of the competition was the implementation of a traffic light system, secondary goals where also to show traffic with different behaviours.

### Why different versions?

Because I didn't have any history for the files here on Github, thats why the different files with version numbers.
Also the changes from version 0.1 to 0.2 have been rather minimal. Only red-yellow color has been added and the code cleaned up prior to version 0.1.

The SDL version is the only version which has some sort of traffic graphical traffic simulation, the other ones are straight console apps in German.

### Why everything is written in German

Well, that wasnt a straight forward thing to do... the circumstances that the competition was held on a German web bulletin board lead to the quick thought that only German should be used for all the naming inside the files.
Just to clarify, next time I would make use of English language instead, its much more straight forward this way to code then to mess around with such a language as German.

## Requirements and how to compile?

Youve got to install SDL and SDL2.0.4 at least. Under Linux, you might want to install first SDL, then install SDL2.0.4 as 2.0.4 - at this very moment - does not properly deliver all requirements and you would run into errors.
Under Windows, you have to compile SDL2.0.4, but I didnt test if there is the same need to install SDL1.x first like said its on Linux the case.

You can compile the non-SDL version using g++ with:
"g++ ampelschaltung_v0.2.c -w -o ampelschaltung"

Or the SDL version linking to SDL2:
"g++ ampelschaltung_v0.2_SDL.c -w -lSDL2 -o ampelschaltung_sdl"
