# 🎮 KeyBound – Text Adventure World

> A two-player cooperative console puzzle-adventure game — built in C++ for the MTA course.
> **Final grade: 100 ✅**

---

## What is KeyBound?

KeyBound is a fully featured two-player cooperative puzzle game that runs entirely in the Windows console. Two players share one keyboard and must **work together** to navigate through a series of rooms filled with traps, puzzles, riddles, switches, bombs, and mysteries — all to reach the final room and win.

It was developed as a course project at MTA (מכללת תל אביב) by:

- **אריק זסלבסקי**
- **יובל רוטר**

---

## 🕹️ Gameplay

Both players are dropped into a world of interconnected rooms. To progress, they must cooperate — one player might need to hold a switch while the other walks through a door, or one picks up a torch to light a dark room while the other disarms a bomb.

The world is full of challenges:

- 🔑 **Keys & Doors** — find keys, flip switches, unlock doors
- 🌑 **Dark Rooms** — fog-of-war rooms that require torches to navigate
- 💣 **Bombs & Mines** — deadly traps that cost you lives
- 🧩 **Riddles** — answer correctly to proceed
- 🌀 **Springs** — launch yourself across rooms
- 📦 **Pushable Blocks** — shove obstacles out of your way

Players each have **3 lives**. Lose them all and it's **Game Over**.

---

## 🎮 Controls

Both players play on the **same keyboard**:

| | Player 1 | Player 2 |
|---|---|---|
| Move | `W A S D` | `I J K L` |
| Stay | `SPACE` | `M` |
| Dispose item | `E` | `O` |

Press `ESC` to pause. Press `H` while paused to return to the main menu.

---

## 🏆 How to Win

Navigate through all the rooms, solve the puzzles, survive the traps, and get **both players** to the final room. Cooperate or fail — this game rewards teamwork.

---

## 🚀 How to Run

1. Open `KeyBound.sln` in Visual Studio 2022.
2. Build with `Ctrl+Shift+B`.
3. Run with `Ctrl+F5` and enjoy!

---

## 🛠️ Built With

- **C++** (Windows, Visual Studio 2022)
- Console rendering with `<conio.h>` and `<windows.h>`
- Fully custom game engine — no external game libraries

---

*A project we're proud of — 100/100 🎉*
