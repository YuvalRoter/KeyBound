# KeyBound – Exercises 1, 2, 3 (C++ – MTA)

Students:

* Name: אריק זסלבסקי  ID: 322598350
* Name: יובל רוטר    ID: 322521097

Course: C++ – MTA
Project: Text Adventure World (Console Game)

---

1. Project Overview

---

KeyBound is a console‑based cooperative text‑adventure / puzzle game developed as part of
Exercises 1–3 in the C++ – MTA course.

The game runs in a standard 80×25 Windows console. Two players are controlled from the same
keyboard and must cooperate to navigate multiple rooms, interact with objects, solve riddles,
use springs, switches and bombs, and reach the final room.

The implementation follows all mandatory requirements of Exercises 1, 2 and 3.

---

2. Development Environment

---

* Visual Studio 2022 (or later)
* Standard C++ (Windows)
* Console size: 80×25

Used APIs / libraries:

* <conio.h>   for _kbhit(), _getch()
* <windows.h> for Sleep() and console color handling
* gotoxy() helper for cursor positioning

---

3. Compilation and Execution

---

Build:

* Open the provided .sln file in Visual Studio.
* Build Solution (Ctrl+Shift+B).

Run (normal mode – Exercise 2 behavior):

* Run from Visual Studio (Ctrl+F5) or from the generated .exe.

Run (Exercise 3 modes):

* adv-world.exe -save
  Records a played game into adv-world.steps and adv-world.result

* adv-world.exe -load
  Loads and replays a game from adv-world.steps and adv-world.result

* adv-world.exe -load -silent
  Replays the game without rendering and checks that the actual results match
  adv-world.result. Prints test PASSED / FAILED.

---

4. Main Menu

---

The main menu supports:
(1) Start a new game
(2) Load game state (bonus – Exercise 3 Part 1)
(3) Present instructions
(4) Toggle color / no‑color mode (bonus)
(5) Controls guide
(9) EXIT

---

5. Controls (case‑insensitive)

---

Player 1:

* RIGHT : D
* DOWN  : S
* LEFT  : A
* UP    : W
* STAY  : SPACE
* Dispose item: E

Player 2:

* RIGHT : L
* DOWN  : K
* LEFT  : J
* UP    : I
* STAY  : M
* Dispose item: O

Global:

* ESC : Pause game

  * ESC again → resume
  * H/h → return to main menu

---

6. Implemented Game Elements

---

Mandatory (Exercise 1):

* Two players
* Walls
* Doors
* Keys
* Springs

Additional (Exercise 2):

* Torches + dark rooms
* Obstacles (pushable blocks)
* Switches (ON / OFF)
* Bombs
* Riddles
* Lives / health

Additional mechanics:

* Mines (trap tiles)
* Fog‑of‑war rendering for dark rooms

---

7. Room Files and Riddles (Exercise 2)

---

* Rooms are loaded from files named: adv-world_XX.screen

* Files are loaded in lexicographical order.

* Legend position is marked with 'L'.

* Riddles are loaded from riddles.txt

* Each riddle contains:

  * Riddle text
  * Expected solution(s)

---

8. Door Logic (Keys + Switches)

---

* Doors may require:

  * Only keys
  * Only switches
  * Both keys and switches

* A door opens only if both requirements are satisfied.

* Keys are consumed when the door opens.

---

9. Movement Rules

---

* Continuous movement in chosen direction until:

  * Wall / obstacle collision
  * STAY key is pressed

* Spring behavior:

  * Springs compress while the player walks over them.
  * On release, player speed and duration depend on compression length.
  * While launched, the player cannot move backward but may strafe.

---

10. Lives / Health System

---

* Players start with 3 lives.

* Mines and bombs can kill players.

* When a player dies:

  * The current room restarts.
  * Picked items and destroyed obstacles are NOT restored.

* After 3 deaths → GAME OVER.

---

11. Pause Behavior

---

* ESC pauses the game.
* ESC again resumes.
* H/h returns to main menu.
* Movement continues exactly as before pause.

---

12. Exercise 3 – File Save / Load

---

Mandatory Part 2:

* adv-world.steps
  Stores game steps with:

  * Time point
  * Player index
  * Key pressed (ASCII) 

* adv-world.result
  Stores expected results:

  * Screen transitions
  * Life loss events
  * Riddle events and answers
  * Game end + score

* Supports:

  * -save mode
  * -load mode
  * -load -silent mode

Bonus Part 1:

* ESC → S/s saves full game state.
* Menu option to load saved game states.

---

13. Exercise 3 – Recording Examples and Test Results

---

This project includes three example recordings for Exercise 3 (steps + results files).
Each example demonstrates a different gameplay outcome and validates the replay and
verification logic.

All recordings use the mandatory file formats:
- adv-world.steps
- adv-world.result

They can be replayed using:
    adv-world.exe -load
or tested using:
    adv-world.exe -load -silent

------------------------------------------------------------
Example 1 – Failed Test (Error Case)
------------------------------------------------------------
Description:
- This example demonstrates a gameplay recording where the expected result file
  does NOT match the actual replayed behavior.

Purpose:
- To verify that the silent replay mode correctly detects mismatches between:
  - adv-world.steps
  - adv-world.result

------------------------------------------------------------
Example 2 – Game Won (Successful Run)
------------------------------------------------------------
Description:
- This example demonstrates a full successful gameplay session in which both players
  reach the final room and complete the game.

Expected behavior:
- Running:
      adv-world.exe -load -silent

- Output:
      TEST PASSED

------------------------------------------------------------
Example 3 – Game Over (Successful Run with Failure Outcome)
------------------------------------------------------------
Description:
- This example demonstrates a gameplay session that ends with GAME OVER
  after both players lose all lives (e.g., due to bombs or mines).

Expected behavior:
- Running:
      adv-world.exe -load -silent

- Output:
      TEST PASSED

------------------------------------------------------------
Notes
------------------------------------------------------------
- These three examples demonstrate that:
  - The recording format is deterministic.
  - The replay engine reproduces gameplay exactly.
  - The silent verification mode correctly detects both success and failure cases.

- The examples can be replaced or extended by recording new sessions using:
      adv-world.exe -save

  and then replaying them with:
      adv-world.exe -load
      adv-world.exe -load -silent

---

14. Self Decisions and Clarifications

---

* HUD is shown on the top row (y = 0).

* The rest of the screen is the room.

* Room transitions:

  * The game follows the second player leaving a room.

* Inventory:

  * Each player can hold only one special item (torch or bomb).

---

15. Use of External / AI‑Generated Code

---

We used AI assistance (ChatGPT and Gemini) and Stack Overflow mainly for:

* Fog‑of‑war logic
* Riddle logic
* Obstacle pushing logic
* Color handling
* Drafting README / bonus.txt

All external logic was reviewed, adapted and integrated by us.

------------------------------------------------------------
Exercise 2 – Previously Deducted Issues (Fixed)
------------------------------------------------------------

The following issues that resulted in point deductions in Exercise 2
were fully fixed in Exercise 3:

- Supporting only 3 rooms (hard coded)
- Hard-coded player starting positions

Both fixes are documented in the accompanying file:
	- Ex2_Fixes.txt