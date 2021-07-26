# Technical Assessment
A very simple Match3 Game implementation in C++ using OpenGL and SDL2

The "AI" will pick the first valid move it finds, the cells will be swapped and display larger than the other cells.
The next step will consume valid matches and each step after will move cells down until no cells can be created and the AI will chose its next move.

#### Controls:
- Key A : Step Game
- Key L : Toggle AI Log
- Key P : Print world as it is to Console.
- Key Space : Toggles Game tick speed between 50ms and 250ms (250ms default) ~~AI Lock (Steps without A Input)~~

#### Build:
To build, open the project using Visual Studio, and install the required packages using Nuget Package Manager or use Nuget Restore.
Once the packages have been installed, you should be able to build the project without any additional libraries.
~~- GLEW is required to compile/run~~

#### Known Problems:
- 'AI' will do any Vertical move before any available Horizontal moves
- For some reason I made all matches work from the middle, so no Edge matches could work. A crude fix was made with what limited time I gave myself to complete so time complexity to solve problem is larger than a much more possbile solution.

#### Visual Demonstration:

![SbQISobfTS](https://user-images.githubusercontent.com/8342701/123944456-7a23cb80-d9e0-11eb-938f-07da13be667d.gif)
