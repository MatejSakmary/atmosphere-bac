This is the repository containing the code for my bachelors thesis.

# Building the application

When building on Windows, almost all the required resources are included. The only thing missing is the
Vulkan SDK along with the glslc compiler used to compile shaders into SPIRV. You also need cmake (version 3.21 or higher) and ninja. Please note that not all features may not be fully working on Windows as the it is primarily developed using Linux.

### Steps

1. Download and install the Vulkan SDK from [Here](https://vulkan.lunarg.com/sdk/home#windows).
2. Download assets as described in the Assets section
3. Build using Cmake - on Linux with clang or gcc, on Windows either with msvc or clang.
    - First you need to call `cmake --preset=<specify preset here>` to configure and generate cmake files
        - To list all available presets call `cmake --list-presets`
    - To build you then call `cmake --build --preset=<specify build preset here>`
        - Similarly to the first command you can list all available build presets by calling `cmake --build --list-presets`
    - The resulting executable will then be located in `build/<preset name>/<build mode (Debug, Release ...)>`

### Assets

The assets (textures) used by the application are stored on my google drive due to their size. To succesfully run the application download the assets folder from [here](https://drive.google.com/file/d/1ClGyf0kVHEH8CMl51A2YLXd42YAYZG7J/view?usp=sharing) and extract it to the **atmosphere-bac** directory (next to source, shaders etc). Make sure to extract/copy only the contents of the directory (the resulting structure should be **atmosphere-bac/assets/textures** not **atmosphere-bac/assets/assets/texture**).
