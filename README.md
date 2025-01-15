<div align="center">
<img src="tools/website/assets/logo.png" width="150"/>
<h2>Fennix Operating System</h2>
</div>

<p align="center">
 <p align="center">
    <img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/EnderIce2/Fennix">
    <img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/EnderIce2/Fennix/makefile.yml">
	<img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/EnderIce2/Fennix">
    <img alt="GitHub License" src="https://img.shields.io/github/license/EnderIce2/Fennix">
 </p>
</p>

<p align='center'>Fennix is an operating system built from scratch using C and C++.</p>

---

> [!CAUTION]
> The project is still in development and is not yet ready for production use.
>
> Please use a virtual machine to run the OS.

## Features

- 🧩 Flexible and modular
  - You can easily choose the components you want to include in the kernel
- 🎛️ Compatible with multiple platforms
  - Supports i386, AMD64 and AArch64 platforms
- 📦 Support for different operating system binaries
  - You can run Linux and Windows binaries
- 🖨️ Driver support
  - Supports a variety of drivers for different hardware components
- 📖 Detailed documentation
  - The project is well-documented using Doxygen

> [!IMPORTANT]
> Some features are still in development and may not be fully functional.

## Getting Started

### 💾 Download ISO Image

You can download the development ISO image from the [Actions](https://github.com/EnderIce2/Fennix/actions/workflows/makefile.yml) tab on the GitHub repository.


### 🛠️ Prerequisites

To build the project, you need to have the following tools installed:

- `make`
- `gcc`
- `ld`
- ... and other standard Unix tools
  - These can be installed using `build-essential` on Ubuntu or `base-devel` on Arch Linux
- `doxygen` (optional)
  - To generate the documentation

### 🚀 Installation

You can install the project by building it from source.

#### Cloning the Repository

First, clone the repository using Git:

```sh
git clone https://github.com/EnderIce2/fennix.git
cd fennix
```

#### Preparing the Environment

Before building the project, you need to build the cross-compiler toolchain and QEMU. You can do this by running the following command:

```sh
make setup
```

This will clone, patch, and build the required tools for you.

#### Building the Project

To build the project, run:

```sh
make build
```

This will build the kernel, userspace, and drivers. The iso image will be `Fennix.iso`.

#### Running the OS

You can run the OS using QEMU. First, build the project, then run:

```sh
make run
```

#### Additional Configuration

You can configure the project by editing the `config.mk` file.

## 🧪 Debugging

If you use Visual Studio Code, you can press `F5` to start debugging the OS.
The configuration is already set up for you.

> [!TIP]
> Make sure you built the project at least once before debugging.

## 📚 Documentation

The project documentation is generated using Doxygen.
To generate the documentation, run:

```sh
make docs
```

The documentation will be available in the `doxygen-doc/docs` directory.

## 👨‍💻 Contributing

Contributions are welcome!
Please read the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines on how to contribute to this project.

### Contributors

<a href="https://github.com/EnderIce2/Fennix/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=EnderIce2/Fennix" />
</a>

## 📃 License

Fennix is licensed under the GNU General Public License v3.0.
See the [LICENSE.md](LICENSE.md) file for more details.

Other licenses used in the project can be found in the [LICENSES.md](LICENSES.md) file.
