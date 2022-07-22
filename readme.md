This is the repository containing the code for my bachelors thesis. 

# Building the application
## Windows

When building on Windows, almost all the required resources are included. The only thing missing is the 
Vulkan SDK along with the glslc compiler used to compile shaders into SPIRV. Please note that not all features may not be fully working
on Windows as the it is primarily developed using Linux.

### Steps
1. Download and install the Vulkan SDK from [Here](https://vulkan.lunarg.com/sdk/home#windows).
2. Go to the repository root directory and from the command line call **premake5 vs2022** (or the version of Visual Studio that you are using).
3. From the command line call **compile\_shaders\_windows.bat**. (You will need to call this any time you change the shader source code to recompile them. This will be changed later to be a part of the VS compilation process)
4. Open the generated visual studio solution.
5. Switch the solution platform from Linux to Windows.
6. Compile and Run the application.

## Linux
When building on Linux, the following dynamic libraries have to be installed: [glfw](https://www.glfw.org)
and [vulkan-dll](https://vulkan.lunarg.com/sdk/home#linux) 

### Steps
1. Download and install Vulkan SDK and GLFW.
2. Go to the root directory and from the command line call **premake5 gmake**.
3. From the command line call **compile\_shaders\_linux.sh**. (You will need to call this any time you change the shader source code to recompile them. This will be changed later to be a part of the VS compilation process)
4. From the command line call **make config=debug\_linux**.
5. Run the executable stored in bin/Linux/Debug/Atmosphere.

## Assets

The assets (textures) used by the application are stored on my google drive due to their size. To succesfully run the application download the assets folder from [here](https://drive.google.com/file/d/1ClGyf0kVHEH8CMl51A2YLXd42YAYZG7J/view?usp=sharing) and extract it to the **atmosphere-bac** directory (next to source, shaders...).
