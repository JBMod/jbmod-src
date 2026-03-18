<p align="center">
  <img src=".github/bling.png" alt="JBMod Logo">
</p>

<p align="center">
  <a href="https://www.jbmod.com/discord">
    <img src="https://img.shields.io/badge/Discord-JBMod-5865F2.svg?style=flat-square&logo=discord" alt="Discord">
  </a>
  <a href="https://store.steampowered.com/app/2158860/JBMod/">
    <img src="https://img.shields.io/badge/Steam-JBMod-blue.svg?style=flat-square&logo=steam" alt="Steam">
  </a>
</p>

The sandbox game that allows you to manipulate physics objects. Widely considered to be the best Source Engine game in the world!

## Download

JBMod is available for free on Steam. Make sure to use the `github` branch for the latest updates.

* [Download on Steam](https://store.steampowered.com/app/2158860/JBMod/)

## Build Instructions

### Windows

**Requirements:**
- JBMod installed via Steam, on the `github` branch
- Visual Studio 2022 with the following workload and components:
  - Desktop development with C++
  - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
  - Windows 11 SDK (10.0.22621.0) or Windows 10 SDK (10.0.19041.1)
- Python 3.13 or later

**Steps:**
1. Navigate to the `src` directory.
2. Run `createallprojects.bat` to generate the Visual Studio solution.
3. Open `everything.sln` and build the solution (`Build > Build Solution`).
4. To run, set the client project (e.g., `Client (JBMod)`) as the startup project and start the debugger.

### Linux

**Requirements:**
- JBMod installed via Steam, on the `github` branch
- [Podman](https://podman.io/)

**Steps:**
1. Navigate to the `src` directory.
2. Run `./buildallprojects` to build against the Steam Runtime.
3. Launch the game from the `game` directory using the generated launcher (e.g., `./jbmod.sh`).

## CI/CD

This project uses GitHub Actions for continuous integration and deployment. The pipeline automatically builds for Windows and Linux and handles packaging and deployment to Steam.  
Some external dependencies are not open source and are not included in the repository, so if you are building from source you will need to install the Steam version of the game to obtain these dependencies.

## Contributing

We welcome contributions! Please refer to the following documents before submitting a pull request:

- [Contributing Guidelines](.github/CONTRIBUTING.md)
- [Code of Conduct](.github/CODE_OF_CONDUCT.md)

## Links

* [Website](https://www.jbmod.com/)
* [Discord](https://www.jbmod.com/discord)
* [Steam](https://store.steampowered.com/app/2158860/JBMod/)

## Legal

This project is distributed under a multi-license structure because it incorporates elements from Valve's Source SDK.

### Original Contributions
All original source code and assets created by The JBMod Authors are licensed under the **Apache License, Version 2.0**. You may obtain a copy of the license in the `LICENSE_APACHE` file included in this repository or at http://www.apache.org/licenses/LICENSE-2.0.

Original contributions will be clearly marked with a permissive license header.

### Source SDK
This project contains code and/or headers derived from Valve's Source SDK. These specific files remain subject to the **Source 1 SDK License**.  

The Source SDK components are restricted to use solely in connection with Valve products. You can find the full text of Valve's license in the `LICENSE` file included in this repository or at https://github.com/ValveSoftware/source-sdk-2013/blob/master/LICENSE.

### Important Notice for Redistributors
While the original code in this repository is provided under the permissive Apache 2.0 terms, any compiled binaries or derivative works that link against the Source SDK are legally bound by the restrictions of the Source 1 SDK License. 
