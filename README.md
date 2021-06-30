# Technical Assessment
A very simple Match3 Game implementation in C++ using OpenGL and SDL2

Controls:
- Key A : Step Game
- Key L : Toggle AI Log
- Key P : Print world as it is to Console.
- Key Space : Toggles AI Lock (Steps without A Input)

To Build:
- Visual Studio can pull most packages using nuget.
- GLEW is required to compile/run

Known Problems:
- 'AI' will do any Vertical move before any available Horizontal moves
- For some reason I made all matches work from the middle, so no Edge matches could work. A crude fix was made with what limited time I gave myself to complete.
