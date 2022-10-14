![Showcase Image](/showcase_image.png?raw=true "Showcase Image")
This is the repository containing the code for my bachelors thesis. The text to the thesis can be located [here](https://github.com/MatejSakmary/atmosphere-bac-text)

# Building the application

When building on Windows, almost all the required resources are included. The only thing missing is the
Vulkan SDK along with the glslc compiler used to compile shaders into SPIRV. You also need cmake (version 3.21 or higher) and ninja. Please note that not all features may not be fully working on Windows as the it is primarily developed using Linux.

# Building
(credit for this entire section goes to awesome how-to-build page for [Daxa](https://github.com/Ipotrick/Daxa) described by [Gabe Rundlett](https://github.com/GabeRundlett). On Windows you can use either msvc compiler or clang. To install either of them you will need to install Visual Studio.

## Chocolatey
The easiest way to get Ninja (and optionaly clang) is by getting the system package manager “Chocolatey”. To install choco (if the instructions found on the website haven't changed) we just need to open powershell as administrator and paste the following command

```
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```
Powershell might ask you to enable the running of scripts, which can be enabled with by running Set-ExecutionPolicy AllSigned. 
## Ninja
To install ninja you then just call `choco install ninja`.
## Clang
To install clang by calling `choco install llvm`. Please note that you need to have Visual Studio installed for this to work.

# Build Steps

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
