🚀 SPACEWAR
SPACEWAR is a 2D pixel-art side-scrolling space shooter developed in C++17 using raylib and Visual Studio 2022.

Pilot your spaceship through dangerous space environments, fight enemy waves, dodge asteroids, and face powerful bosses in an arcade-inspired experience.

Project status: Foundation completed — gameplay development is the next milestone.

🌌 STORYLINE
In the vastness of space, danger awaits at every turn.

Take control of a spaceship and embark on a journey through hostile space environments filled with enemy forces, dangerous obstacles, and powerful adversaries.

Your mission is to survive enemy encounters, eliminate incoming threats, and ultimately confront powerful bosses standing in your way.

Your survival depends on your reflexes, precision, and ability to adapt.

The detailed narrative, characters, and universe will be expanded as development progresses.

🎮 GAMEPLAY
SPACEWAR is designed around classic arcade shooter mechanics, combining fast-paced combat, movement, and survival.

Core Gameplay Loop

Move — Navigate through space and dodge incoming threats.

Shoot — Fire projectiles to eliminate enemy spaceships.

Survive — Avoid enemy attacks and environmental hazards while managing your health.
Defeat the Boss — Face powerful bosses with unique attack patterns.
Planned Features
Smooth spaceship movement and keyboard controls.
Player health and lives management.
Projectile shooting mechanics.
Multiple enemy types and enemy waves.
Asteroids and environmental hazards.
Collision detection and damage systems.
Boss encounters with health bars and attack phases.
Score system and HUD.
Game states: Menu, Playing, Paused, Game Over, and Victory.
Pixel-art sprites, animations, and visual effects.
Sound effects and background music.
Visual Direction
2D side-scrolling gameplay.
Pixel-art aesthetic.
Dark navy and purple space environments.
Cyan/blue player projectiles and contrasting enemy attacks.
Colorful nebulae, stars, and space backgrounds.
🛠️ TECHNOLOGIES & TOOLS
Technology	Purpose
C++17	Main programming language
raylib	2D graphics, input, audio, and game development
Visual Studio 2022	Integrated development environment
MSVC	C++ compiler
vcpkg	C++ package management
Git & GitHub	Version control and project hosting
📁 PROJECT STRUCTURE
SpaceWar/
├── SpaceWar.sln
├── SpaceWar.vcxproj
├── src/
│   ├── main.cpp
│   ├── Game.h / .cpp
│   ├── Player.h / .cpp
│   ├── Bullet.h / .cpp
│   ├── Enemy.h / .cpp
│   ├── Asteroid.h / .cpp
│   ├── Boss.h / .cpp
│   └── AssetManager.h / .cpp
│
├── assets/
│   ├── backgrounds/
│   ├── sprites/
│   │   ├── player/
│   │   ├── enemies/
│   │   ├── boss/
│   │   ├── projectiles/
│   │   └── effects/
│   ├── fonts/
│   └── audio/
│       ├── music/
│       └── sfx/
│
└── README.md
Class Responsibilities
Class	Responsibility
Game	Game loop, states, and overall coordination
Player	Player spaceship, movement, and health
Bullet	Projectile behavior and damage
Enemy	Standard enemy spaceships
Asteroid	Environmental obstacles
Boss	Boss health, phases, and attacks
AssetManager	Centralized resource management
⚙️ INSTALLATION & REQUIREMENTS
Prerequisites
Visual Studio 2022 with Desktop development with C++ workload (v143 toolset).
Git.
vcpkg.
raylib installed for x64-windows.
raylib Setup
If raylib is not already installed through vcpkg:

vcpkg install raylib:x64-windows
vcpkg integrate install
How to Build and Run
Clone the repository:
git clone https://github.com/YOUR-USERNAME/SPACEWAR.git
Open SpaceWar.sln in Visual Studio 2022.
Select the Debug or Release configuration.
Select the x64 platform.
Build the solution using Ctrl + Shift + B.
Run the project using F5 or Ctrl + F5.
The debugger working directory is configured to $(ProjectDir) so relative paths such as assets/... resolve consistently.

📚 SOURCES & REFERENCES
Development Resources
raylib — Official Website
raylib — GitHub Repository
Microsoft Visual Studio Documentation
vcpkg — Official Documentation
Asset Resources
Sprite sheets, backgrounds, fonts, and audio resources will be credited here as they are introduced into the project.

Each external asset will be documented with its original creator and source link, respecting its licensing requirements.

🎯 DEVELOPMENT ROADMAP
SPACEWAR will be developed incrementally, with each milestone tested before moving to the next.

 Development environment setup.
 Visual Studio 2022 project foundation.
 raylib integration.
 Initial class architecture and project structure.
 Space environment and scrolling background.
 Player movement and controls.
 Shooting mechanics and projectiles.
 Enemy waves and asteroid hazards.
 Collision detection and combat systems.
 Boss encounters.
 HUD, menus, and game states.
 Audio, animations, and visual polish.
 Final playable Windows build.
👩‍💻 DEVELOPMENT
This project is being developed as a learning and software engineering project, with an emphasis on:

Object-Oriented Programming (OOP).
C++ application architecture.
Game development fundamentals.
Resource and asset management.
Version control using Git and GitHub.
One milestone at a time. Build, test, improve, repeat.
