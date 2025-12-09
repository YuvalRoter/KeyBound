=========================
KeyBound – Exercise 1
C++ – MTA
=========================

1. Students details
-------------------
Course: C++ – MTA
Exercise: 1 – Text Adventure World (console game)

Students:
	- Name: אריק זסלבסקי ID: 322598350
	- Name: יובל רוטר    ID: 322521097


2. Project overview
-------------------
	KeyBound is a console-based text adventure / puzzle game implemented for Exercise 1.

	The game runs in an 80x25 Windows console. Two cooperative players must navigate
	through several rooms, collect items (keys, torches etc.), solve challenges and
	reach the final room.

	- 2 player characters, controlled from the same keyboard.
	- Multiple rooms (at least 2 challenge rooms + a final “end”).
	- Movement is continuous in the chosen direction until either:
	  - The player hits a wall / blocking object, or
	  - The player presses the STAY key.
	- Some rooms can be dark and require a torch to see around the player.


3. Compilation & execution
--------------------------
	Environment:
		- Visual Studio 2022 (or later)
		- Standard C++ on Windows
		- Console size: 80x25

	Required libraries / APIs:
		- <conio.h>   for _kbhit(), _getch()
		- <windows.h> for Sleep(), console handle, optional colors
		- gotoxy() helper for positioning the cursor in the console

	Build:
		- Open the provided .sln in Visual Studio.
		- Build Solution (Ctrl+Shift+B).

	Run:
		- Run from Visual Studio (Ctrl+F5) or run the compiled .exe from the Debug/Release folder.
		- The program starts at the main menu.


4. Main menu
------------
	The main menu follows the spec of Exercise 1:

		(1) Start a new game
		(2) Present instructions
		(3) choose color mode
		(4) controls guide
		(9) EXIT
 
 
5. Controls (keys)
------------------
	Input is case-insensitive (both lowercase and uppercase should work).

	Player 1:
		- RIGHT : D
		- DOWN  : S
		- LEFT  : A
		- UP    : W
		- STAY  : " " (SPACE KEY)
		- Dispose element (drop object): E

	Player 2:
		- RIGHT : L
		- DOWN  : K
		- LEFT  : J
		- UP    : I
		- STAY  : M
		- Dispose element (drop object): O

	Global:
		- ESC : Pause game. In pause mode, pressing ESC resumes the game, and pressing ESC and H/h takes to menu.
			  

6. Game elements implemented (Exercise 1)
-----------------------------------------
	Mandatory elements (required by Exercise 1):
		- Two players
		- Walls
		- Keys
		- Doors

	Optional elements implemented:
		- Torch (with dark rooms)
		- Riddle (with simon says - will be explaned in bouns file)
		- Spring

	For each element:

	Character mapping:

	none colored:
		- Player 1: @
		- Player 2: &
		- Wall    : ▒
		- Door    : number between 1 - 9 (indicating number of keys needed)
		- Key     : K
		- Spring  : +
		- Torch   : T
		- Riddle  : ?
		- won char: ▒ 

	colored:
		- Player 1: @ (colored cyan)	
		- Player 2: & (colored red)
		- Wall    : ▒ (colored dark orange) 
		- Door    : number between 1 - 9 (indicating number of keys needed)
		- Key     : K
		- Spring  : +
		- Torch   : T
		- Riddle  : ?
		- won char: ▒ (bright red)

7. Room structure and flow
--------------------------
	- The game contains at least 2 challenge rooms plus a final “end” room.
	- The final room contains no items and marks the end of the game.
	- When one player reaches the final room first:
	  - The camera returns to the room of the second player.
	  - The first player cannot move back from the final room.
	- The game ends when both players have reached the last room and took the won char.

	Movement rules:
		- After a movement key is pressed, the player continues in that direction automatically,
		  one cell per game cycle, until:
		  - STAY key is pressed, OR
		  - Movement is blocked by a wall / obstacle / world boundary.
		- If affected by a spring, movement speed and force follow Exercise 1 rules:
		  - The speed and duration depend on how many spring characters have been compressed.
		  - While under spring effect, the player cannot move back against the release direction,
			but may move sideways at normal speed.


8. Dark rooms and torch 
-----------------------
	- Some rooms are marked as “dark”.
	- In a dark room, only tiles within a certain radius around a player carrying a torch
	  are visible on screen (fog-of-war).
	- If no player holds a torch, the dark room is not visible (except the player tile).
	- Implementation detail (for the grader):
	  - When a torch is picked up, it stays in the player’s inventory until disposed.
	  - When an element is disposed, it is dropped onto the floor and can be picked up again.


9. Springs (summary)
--------------------
	- Springs consist of one or more spring characters in a straight line, adjacent at one end
	  to a wall.
	- When a player moves over a spring the spring compresses.
	- When the player stops or changes direction, the spring releases and sets the player’s
	  speed and duration according to the number of compressed charsame cycles
	- Under spring effect, all other collision rules still apply (walls, players, items).


10. Pause behavior (ESC) 
------------------------
	- Press ESC during the game:
	  - The game enters PAUSE mode:
	  - Press ESC again to resume the game.
	  - Press H/h to stop the current game and return to the main menu.
	- When resuming, all motion continues as if the game was not paused.


11. Self decisions and clarifications
-------------------------------------
	This section lists design decisions we made where the exercise specification left freedom:

	- HUD layout:
	  - We use the top line (y = 0) as a HUD/status bar showing lives, score, and current items.
	  - The rest of the screen (y >= 1) is used for the room itself.

	- Room transitions:
	  - When a player steps on a door that is open / can be opened, the player is transferred
		to the target room according to the door rules.
	  - The game follows the second player leaving the room: only when both players have left,
		or the second player leaves to a different destination, the screen switches.


12. Use of external / AI-generated code
---------------------------------------
	We used AI assistance (ChatGPT) and sites like stack overflow for some parts of the implementation, mainly for:
	- Refining the fog-of-war rendering logic for dark rooms.
	- Riddles logic
	- added colors 
	- Helping draft this readme.txt and bonus.txt and bouns.txt
