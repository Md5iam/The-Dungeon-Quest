# 🏰 The Dungeon Quest

A **3D first-person adventure quest game** built entirely with **OpenGL (GLUT)** and **C++**. The player must explore a mysterious dungeon, solve unique puzzles in each chamber, collect keys, and escape through the correct doors — all rendered in real-time 3D with textured environments, dynamic lighting, and atmospheric effects.

> **University Course:** Computer Graphics and Animation

---

## 📸 Game Overview

The player begins standing outside the heavy gates of an ancient dungeon. To win, they must successfully navigate through **four distinct chambers**, each presenting a unique challenge and a test of intellect.

**Progression Flow:**

```
Outside Dungeon → Chamber 1 → Chamber 2 → Chamber 3 → Chamber 4 → Victory
```

Each chamber is locked. The player must **solve the puzzle** to reveal a Golden Key, then **choose the correct exit door** to advance. Choosing the wrong door triggers a trap (screen shake, fall animation, and puzzle reset).

---

## 🎮 Chambers & Puzzles

### Chamber 1 — The Hall of Statues (Sequence Puzzle)
- Four animal statues stand on stone pillars: **Wolf**, **Lion**, **Eagle**, **Snake**.
- Walk up to each statue and press **[ENTER]** to select it.
- Select them in the **correct order** (Wolf → Lion → Eagle → Snake) to solve the puzzle.
- Incorrect order causes a blackout and resets.
- Once solved, the central pedestal opens and reveals a **Golden Key**.
- Two exit doors: **"SUN"** (Left) and **"MOON"** (Right) — choose wisely!

### Chamber 2 — The Ring Lock (Rotation Puzzle)
- Three concentric glowing rings float above a pedestal.
- Use keyboard controls to **rotate each ring** to match the target alignment angles.
- When all three rings align correctly, the puzzle is solved.
- Two exit doors: **"CLOCK"** (Left) and **"HOURGLASS"** (Right).

### Chamber 3 — The Dragon Illusion (Perspective Alignment Puzzle)
- A rotating **Dragon Relic Hologram** floats above the pedestal, surrounded by spinning neon rings and spiraling particles.
- Press **[L]** to toggle the room light OFF (Dark Mode required to solve).
- In Dark Mode, a glowing **Target Frame** appears on the back wall.
- Move to the exact position, set the correct pitch/yaw rotation, and adjust **zoom FOV** using **[** / **]** to align the hologram inside the target frame.
- Secret key: Press **[U]** to toggle a Solution Beacon showing the target position.
- Two exit doors: **"EYE"** (Left) and **"MIRROR"** (Right).

### Chamber 4 — The Ghost Escape Hall (Stealth Game)
- A large octagonal hall divided into a **Central Hall** and **4 side rooms** by stone walls.
- A **tattered ghost** patrols the hall. If it detects the player, it chases at high speed.
- **Hide in cabinets** placed in room corners to become invisible to the ghost.
- Collect **4 colored gem fragments** from the side rooms without being caught.
- Once all gems are collected, approach the **Key Box Pedestal** and press **[ENTER]** to retrieve the final key.
- If caught: jumpscare (red flash, screen shake) and collected items reset.
- Single exit door: **"EXIT"** (Center).

### Outside — The Truth
- Upon exiting the dungeon, the player is met with a pitch-black screen and the message:
  > **"AI is replacing software engineers, go back to the dungeon!!!"**
- Press **[R]** to return to the dungeon.

---

## 🕹️ Controls

| Key | Action |
|-----|--------|
| `W/A/S/D` or `Arrow Keys` | Move forward / left / backward / right |
| `Mouse Drag` | Look around (rotate camera) |
| `Page Up / Page Down` | Tilt camera up / down |
| `E` or `ENTER` | Interact (select statue, toggle switch, collect key, open door) |
| `L` | Toggle room light (Chamber 3 only) |
| `U` | Toggle Solution Beacon (Chamber 3 only) |
| `[` / `]` | Zoom FOV in / out (Chamber 3 only) |
| `1` / `2` | Toggle lights (Chamber 4) |
| `R` | Reset / Return to dungeon entrance |
| `Q` or `ESC` | Quit game |

### Admin Controls
| Key | Action |
|-----|--------|
| `K` | **Admin Skip** — Instantly teleport to the next chamber (for demonstration) |

---

## 🛠️ Technical Features

### Graphics & Rendering
- **OpenGL Fixed-Function Pipeline** with GLUT
- **Textured 3D environments** using `.jpg` textures loaded via `stb_image.h`
- **Octagonal room geometry** with textured stone walls, floors, and ceilings
- **Dynamic fog** (GL_FOG) for atmospheric depth
- **Back-face culling** for performance

### Lighting
- **Multi-light setup** (GL_LIGHT0 through GL_LIGHT3)
- **Point lights** with attenuation for torches and ambient sources
- **Dynamic flickering torches** at the dungeon entrance
- **Per-chamber lighting presets** (bright, dark, horror atmosphere)
- **Flashlight effect** in Chamber 4 (player-facing spotlight)

### Animation & Effects
- **Smooth door opening animation** with camera walk-through transition
- **Floating/rotating Golden Keys** with sine-wave bobbing
- **Spinning hologram rings** and **particle systems** (Chamber 3)
- **Ghost AI** with patrol, detection, chase, and jumpscare (Chamber 4)
- **Screen shake** (rumble) on traps and jumpscares
- **Fade-in/fade-out** transitions between chambers
- **Sliding pedestal lids** that open when puzzles are solved

### Camera System
- **First-person camera** using `gluLookAt` with Euler angle rotation
- **Mouse-drag look** with clamped pitch limits
- **Keyboard WASD movement** with collision detection against walls and doors

### 3D Text Rendering
- **Exit door labels** rendered using `glutStrokeCharacter` (3D line-stroke text centered on door surfaces)
- **HUD text overlay** using `glutBitmapCharacter` for status, instructions, and coordinates

---

## 📁 Project Structure

```
The Dungeon Quest/
├── image/                  # All texture assets
│   ├── box.jpg             # Pedestal / key box texture
│   ├── dragon.jpg          # Dragon hologram texture
│   ├── eagle.jpg           # Eagle statue texture
│   ├── ground.jpg          # Outdoor ground texture
│   ├── lion.jpg            # Lion statue texture
│   ├── sky.jpg             # Skybox texture
│   ├── snake.jpg           # Snake statue texture
│   ├── stone.jpg           # Stone wall texture
│   ├── wolf.jpg            # Wolf statue texture
│   └── wood.jpg            # Wooden door texture
├── chamber1.h              # Chamber 1 header + shared globals/enums
├── chamber1.cpp            # Chamber 1: Statue sequence puzzle
├── chamber2.h              # Chamber 2 header
├── chamber2.cpp            # Chamber 2: Ring rotation puzzle
├── chamber3.h              # Chamber 3 header
├── chamber3.cpp            # Chamber 3: Perspective alignment puzzle
├── chamber4.h              # Chamber 4 header
├── chamber4.cpp            # Chamber 4: Ghost stealth escape
├── outside.h               # Outside scenario header
├── outside.cpp             # Outside scenario (exit message screen)
├── dungeon_game.cpp        # Main game engine (rendering, input, camera, HUD)
├── main.cpp                # Standalone Dragon Relic 3D Animation demo
├── stb_image.h             # Single-header image loading library
├── Makefile                # Build configuration
└── README.md               # This file
```

---

## 🔧 Build & Run

### Prerequisites
- **Linux** (tested on Ubuntu)
- **g++** (C++ compiler)
- **OpenGL**, **GLU**, **GLUT** development libraries

Install dependencies (Ubuntu/Debian):
```bash
sudo apt-get install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
```

### Compile
```bash
cd "The Dungeon Quest"
make
```

This produces two executables:
- **`DungeonGame`** — The full dungeon quest game
- **`DragonAnimation`** — Standalone 3D dragon relic animation demo

### Run
```bash
./DungeonGame          # Launch the full game
./DragonAnimation      # Launch the animation demo
```

### Clean
```bash
make clean
```

---

## 🎨 Bonus: Dragon Relic Animation (`main.cpp`)

A separate standalone demo showcasing:
- **Textured rotating cube** with dragon imagery on all 6 faces
- **Metallic gold corner borders** and decorative sphere accents
- **Orbiting colored point light** (cycle with `[C]`)
- **Golden particle system** swirling around the relic
- **Interactive HUD** with real-time toggle status
- Controls: `[T]` Texture, `[L]` Lighting, `[P]` Auto-Rotate, `[V]` Particles, `[+/-]` Zoom, `[R]` Reset

---

## 📝 OpenGL Concepts Demonstrated

| Concept | Where Used |
|---------|-----------|
| `gluLookAt` Camera | First-person navigation in all chambers |
| `glFrustum` / `gluPerspective` | Zoom-based projection in Chamber 3 |
| Texture Mapping (`glTexCoord2f`) | Walls, floors, doors, statues, hologram |
| Multi-Light Setup (`GL_LIGHT0-3`) | Per-chamber atmospheric lighting |
| Light Attenuation | Torches, flashlight, point lights |
| Fog (`GL_FOG`) | Depth-based atmospheric haze |
| Blending (`GL_BLEND`) | Transparent particles, ghost, HUD overlays |
| Display Lists / Immediate Mode | All geometry rendering |
| Stroke Font Rendering | 3D text on door surfaces |
| Bitmap Font Rendering | 2D HUD text overlay |
| Back-face Culling (`GL_CULL_FACE`) | Performance optimization |
| Matrix Transformations | `glTranslatef`, `glRotatef`, `glScalef` throughout |
| Timer-based Animation | `glutTimerFunc` at ~60 FPS |
| Keyboard/Mouse Input | WASD movement, mouse-drag look, interactive keys |
| Collision Detection | Wall boundaries, door locks, ghost proximity |
| Particle Systems | Chamber 3 sparks, Chamber 4 ghost trail |

---

## 👤 Author

**University Computer Graphics and Animation Project**

---
