# Spellculator

A math-based action game built with **C++** and **raylib**. Play as a wizard, fight slime enemies, and solve math questions during timed battles. The difficulty increases as you progress through the levels.

![Gameplay](screenshots/gameplay.png)

## Features

- **Player movement:** Move around the game world with animated wizard and slime sprites.
- **Math battles:** Solve math questions within a 10-second time limit to defeat enemies.
- **Progressive difficulty:** Questions start with addition and subtraction, then introduce algebra, multiplication, and division in later levels.
- **Combat system:** Dodge fireballs while managing your health and lives.
- **Scoring and progression:** Clear enemies to advance through levels.
- **Game settings:** Includes a main menu, instructions, credits, and separate music and sound effect controls.

## Tech Stack

- **Language:** C++
- **Graphics Library:** raylib
- **Build System:** CMake

## How to Play

| Action | Control |
|---|---|
| Move the wizard | Arrow keys |
| Start a battle | Touch a slime |
| Enter an answer | Number keys + Enter |
| Delete a digit | Backspace |
| Adjust audio settings | Arrow keys |

You start with 3 lives. Wrong answers, running out of time, and getting hit by fireballs each cost one life. Clear all enemies to move to the next level.

## Getting Started

### Prerequisites

- A C++ compiler
- CMake 3.16 or higher

### Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/YOUR-USERNAME/math-wizard-adventure.git
   ```

2. Navigate to the project folder:

   ```bash
   cd math-wizard-adventure
   ```

3. Configure and build the project:

   ```bash
   cmake -B build
   cmake --build build --config Release
   ```

raylib is downloaded and built automatically during the first build.

### Running the Game

- **Windows:** `build\Release\MathWizard.exe`
- **Linux/macOS/MinGW:** `./build/MathWizard`

Visual Studio users can also open the project folder using **File → Open → Folder**. CMake will be detected automatically.

## Credits

- Game design and code: **Hania Qaisar**
- Wizard and slime sprites: [Adriano Verissimo](https://adrverissimo.itch.io/slime-monster-pixel-art)
- Backgrounds: AI-generated
- Music and sound effects: "Enchanted Fantasy" by Sonican from Pixabay
- Font: raylib default font

Built as a semester project for **CS-110 Fundamentals of Computer Programming**.
