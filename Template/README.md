# Ray Tracing C++ Template

This project includes a template for a Ray Tracing renderer in C++. The template is based on the [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The template uses [SFML](https://www.sfml-dev.org) to create an output window, among other things. See the [SFML documentation](https://www.sfml-dev.org/) for more.

## Compilation

The compilation is based on this [video](https://www.youtube.com/watch?v=t0z3RojiKFg). Watch this for a step-by-step guide to make sure you have C++ and SFML correctly installed and configured on your device.

You should have CMake installed on your device. I recommend [CMake GUI](https://cmake.org/download); if you are on Windows, just download and install the .msi of the latest version.

Then, clone this repository, and open it. I recommend [Visual Studio Community 2022](https://visualstudio.microsoft.com/downloads). You should also install some additional C++ libraries - please watch the video linked above to see what you should install.

After you have VS Community 2022 and SFML correctly installed on your device, open a command or Powershell prompt. Navigate to where the project is located (the path should end in "Template" if you haven't changed any of the names).

Create a new directory called "build", such that your path to this folder ends with "Template/build". Navigate to this folder inside your prompt, and type in the command `cmake ..`.

On some devices, this will not work - you should use the command `cmake -G "Visual Studio 17 2022" ..` instead.

This will build a VS Community solution that you can then open. This solution, called `RayTracer.sln`, will be located in the "build" folder. Open it with Visual Studio Community 2022.

When opened, open the Solution Explorer (which should be on the right-side of the screen), and right click on the "RayTracer" project (**NOT** the solution at the top, but the Ray Tracer in the "subfolder" of the solution). Make this project the start-up project of this solution.

Then, you can hit "Run" at the top of the screen, and both the console (outputting the progress) and the window of the Ray Tracer should pop up. Happy tracing!

## Features

- Primitives: spheres
- Materials: diffuse, reflective, refractive
- Anti-aliasing
- Depth of field
- Field of view
- Positionable camera

## Renders

A nice render of three types of spheres on a big sphere

<img width="1914" height="1075" alt="image" src="https://github.com/user-attachments/assets/2056c6c3-d511-492e-8f37-b8b3fa62abe0" />

<br/>

Same render, but camera positioned elsewhere, and depth of view applied

<img width="1915" height="1075" alt="image" src="https://github.com/user-attachments/assets/232be032-48cd-4fb4-b9f7-758c54dc54aa" />
