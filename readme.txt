XXX.  ,XXX'                           XX
`XXX..XXX'     XXXx. XX XxXx .xXx. .xXXX
  `XXXX'  XXXX XX XX XX XX'` XXxXX XX XX
,XXX'`XXX.     XXxX' XX XX   `Xxx  `XXXX
XXX'  `XXX.    XX
****************************************
- General, X-pired, X-pir-Ed-It



****************************************
General:
****************************************
- WWW,    Authors,   Thanks,    Licence,
Libraries

*** WWW:
http://xpired.temnet.org

*** Authors:
Programming, graphics & levels by:
               Miroslav Kucera [Wendigo]
Track composed by:
                     Jiri Kubrt [Bubrak]
Additional programming, Linux build:
                   Michal Belda [Micval]
Additional leveldesign:
                     Martin Kucera [MaK]
Additional levels, testing, Linux b.:
                    Jakub Drnec [JayDee]
Additional programming:
                 Rogan Creswick [Largos]

*** Many thanks for:
Promotion in Japan:
           Shinji Iioka [Linux magazine]
Debian packages:
                   Roman Porizka [arTee]

*** Licence:
This program is  free software;  you can
redistribute  it and/or  modify it under
the   terms of  the  GNU General  Public
License  as  published   by   the   Free 
Software Foundation; either version 2 of
the  License,  or  (at  your option) any
later version.

This program  is distributed in the hope
that it will be  useful, but WITHOUT ANY
WARRANTY;   without   even  the  implied
warranty  of MERCHANTABILITY or  FITNESS
FOR  A PARTICULAR  PURPOSE.  See the GNU
General Public License for more details.

You  should have  received a copy of the
GNU General  Public  License  along with
this   program;   if  not,  write to the
Free Software Foundation, Inc.
59 Temple Place, Suite 330
Boston, MA  02111-1307  USA

*** Libraries:
SDL 1.2 Library:
http://www.libsdl.org/download-1.2.html
SLD_gfx 2.0 Library:
http://www.ferzkopp.net/Software
 /SDL_gfx-2.0/
SDL_image Library:
http://www.libsdl.org/projects/SDL_image
SDL_mixer Library:
http://www.libsdl.org/projects/SDL_mixer

All libraries are distributed  under the
terms of GNU LGPL.



****************************************
X-pired:
****************************************
- Keys,  Passwords,  DemoMode,   Command
line options, Level elements

*** Keys:
AltS            - setup volume & shadows
AltF,AltEnter	- toggle fullsc/windowed
Cursor keys     - move
Ctrl            - run (push harder)
Esc             - kill (restart level)
AltX,AltQ,AltF4 - exit

*** Passwords:
Password (if level has one) is displayed
at window caption in square brackets.
If you  want to  use it, just type it in
title screen and hit ENTER or CTRL.
If game is over,  the latest password is
automaticaly prepared to enter.

*** DemoMode:
To  play  stored  demo,  enter  password
'demo'.

*** Command line options:
-j, --joystick   Enable Joystick 0
-f, --fullscreen Toggle fullscreen/wind.
-n, --nosound    Disable sound
-l <file>        Load levels from <file>
-d <file>        Use demorec from <file>
--version        Display version no.
-h, --help       Display this help scr.

*** Level elements info:
- Wall: Can't be destroyed.
- TinyWall:    Could   be  destroyed  by
  explosion.
- Crate: Pushable. Could be destroyed by
  fire or explosion.
- Barrel:  Pushable.  Would  explode  if
  pushed  harder  and  colide with other
  object or  could be initiated by  near
  explosion.
- HiExplosive:  The same as Barrel,  but
  with doubled explosion range.
- BEM: Mysterious monster. Step aside it
  and you will be dead.  Could be killed
  by fire or explosion.  It don't absorb
  explosions. Exits are not active while
  any of BEMs is alive.
- Exit: Get there!
- Teleport:  Objects moved into teleport
  will  be transported  to  the  nearest
  teleport  in  south  direction in  the
  same  column or to the  first teleport
  in east  direction.  If there  is   no
  teleport  in  this  directions, object
  will  be send to  first  teleport from
  the left.  Object will leave  teleport
  in the  same  direction  and with  the
  same speed as after  teleportation. If
  it couldn't  leave teleport,  teleport
  will be blocked by this object.
- Ice: You will slide on it. Barrels and
  HiExplosives would be from ice  pushed
  harder.
- Hot!:  Would  destroy  crate,  barrel,
  player,  etc. without explosion.  Then
  is burned out.
- Flamable:    Could   be   ignited   by
  explosion.   Burning  flamable   could
  explode Barrels,   HiExplosives,  kill
  BEMs or you and destroy Crates.
- Retarder:   Would  stop  fast   moving
  Barrel or HiExplosive.



****************************************
X-pir-Ed-It:
****************************************
- Keys, Command line options, General

*** Keys:
1-0             - place message
LeftShift       - clear message tag
Cursor up/dn    - level up/dn
AltF4           - exit

*** Command line options:
-f, --fullscreen Toggle fullscreen/wind.
-l <file>        Load levels from <file>
--version        Display version no.
-h, --help       Display this help scr.


*** General:
Don't forget to save before exiting...

