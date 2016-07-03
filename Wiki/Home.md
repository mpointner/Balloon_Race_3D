# Balloon Race 3D

BalloonRace3D is a multiplayer air race with ballons. The players have to fly a given amount of rounds in a course of rings. The rings have to be taken in a given order. Both players start at the same distance from the first ring and the player who passes the last ring first wins.

![cover](https://github.com/ippon1/Balloon_Race_3D/blob/master/Screenshots/cover.png)

## Controls

The ballons have an airscrew in the back of the basket so they can move forwards/backwards and are steered with a rudder. They move up and down as usual by heating or letting out air.

NOTE: A detailed description for controlling the Actors is in the Design Document.

| Key Player 1/2 (with XBOX 360 Gamepad) | Effect |
|----------------------------------------|--------|
| Joystick left | Balloon Accelerate/Decelerate/Pan left/Pan right |
| Joystick right | Balloon Rotate view around balloon|
| A BUTTON (Gamepad) | Balloon Fire rocket |
| RT | Balloon Go up |
| LT | Balloon Go down |
| ESC | Quit game |
| F1 | Help (in Consol) |
| F2 | Frame Time on/off|
| F3 | Toggle wireframe |
| F4 | Textur-Sampling-Quality*: Nearest Neighbor/Bilinear |
| F5 | Mip Mapping-Quality*: Off/Nearest Neighbor/Linear|
| F6 | HDR on/of |
| F7 | - |
| F8 | Viewfrustum-Culling on/off |
| F9 | Transparency on/off|

*changes can be best seen at the cube map

## Features

   * fun game :)
   * 2 player split-screen multiplayer
   * Collision Detection
   * GPU-Particle System of the rockets; Balloon can shoot
   * Changing Speed of the actor (slows down if hit by a rocket; accelerate if payer "collects" speed cube)
   * Passed detection for rings
   * Displaying each player (seperately) which ring it should fly through next (next ring glows)
  
## Design Document
[Design Document](https://github.com/ippon1/Balloon_Race_3D/wiki/Design-Document)

## Screenshots
![Screenshot](https://raw.githubusercontent.com/ippon1/Balloon_Race_3D/master/Screenshot/balloonrace3dneu.jpg)