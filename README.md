# Real-Time Continuous Levels of Detail

## Launching pre-compiled binaries

1. Unzip the archive for either Windows, or MacOS (Apple Silicon only)
2. Launch the file named Realtime_Cell_Collapse.exe. In the case of MacOS launch the program using the command ./Realtime_Cell_Collapse
3. Wait for the program to load the 3D model from disk before there is any feedback. Depending on the system this can take up to a minute.
   
All further information is displayed on the screen.

## Source code setup

This project requires a valid [Vulkan](https://www.vulkan.org/tools#download-these-essential-development-tools) installation and [vcpkg](https://vcpkg.io/) to be compilable. It has been tested working on Windows (x64) and MacOS (arm64). 

### Warning

While this project started out as being very organized and designed to be highly scalable, due to the nature of university projects and deadlines, the last sprint to the finish line has left it in a suboptimal state in terms of code cleanliness. You have been warned. I do intend to work on this.