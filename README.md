# Matter Manager

This repository is forked from
[connectedhomeip](https://github.com/project-chip/connectedhomeip) with version
v1.2.0.1. The original `README.md` has been moved to `README.ori.md`.

The following are the primary branches of this repository.

-   mainline: It's forked from
    [connectedhomeip](https://github.com/project-chip/connectedhomeip) with
    version v1.2.0.1. And we'll put main features on this branch.
-   connectedhomeip-mainline: This branch is forked from
    [connectedhomeip](https://github.com/project-chip/connectedhomeip), and we
    are supposed not to change this branch except to pull the latest changes.
-   poc-demo: This branch holds the PoC demo code that shows How to use the CHIP
    core library to manage the Matter network like the CHIP tool. It exposes
    APIs that are designed to be invoked by CDMB. For more details, please refer
    to section [PoC Demo](#poc-demo) and doc
    [Matter-demo](https://quip-amazon.com/t3bqAilWwwca/Matter-demo)
-   ws-based: This branch contains the old design and has been abandoned.

# How to build

We haven't made any changes to the build system, so you can refer to any
connectedhomeip build documents. The following section is a summarization of
[BUILDING.md](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md).

## Prerequisites

Install prerequisites on Linux with the following command.

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

## Download and set up environment

Download this repository with the following command.

```
git clone ssh://git.amazon.com/pkg/MatterManager
```

Download submodules and set up the environment with the following command. This
command checks and downloads submodules, sets up a Python virtual environment,
installs required Python packages, and sets up Bash environment variables. This
command is also required if you start a fresh shell and build again.

```
cd MatterManager
source scripts/activate.sh
```

## Build CHIP-tool

This repository uses GN as its build framework. Use the following command to
generate the build configuration. This command indicates the project/example
root is `examples/chip-tool`, and the artifacts will be put in `out/chip-tool`.

```
gn gen --root=examples/chip-tool out/chip-tool
```

Build it with ninja with the following command.

```
ninja -C out/chip-tool
```

As the build is complete, an executable binary will be in
`out/chip-tool/chip-tool`.

## Build lighting-app

Use the following command to build lighting-app.

```
gn gen --root=examples/lighting-app out/lighting-app
ninja -C out/lighting-app
```

# PoC Demo

The code for the PoC Demo and CHIP-tool has been consolidated, and they now
share the same compile configuration. The source code is located under
`examples/chip-tool/mtmgr/`. Additionally, modifications have been made to the
file `examples/chip-tool/BUILD.gn`, where the `mtmgr` library and `mtmgr-app`
executable binary have been added.

When you build chip-tool example, it'll also build the `mtmgr` library and
`mtmgr-app` executable in the same out folder of chip-tool.

About how to run the demo, please refer to doc
[Matter-demo](https://quip-amazon.com/t3bqAilWwwca/Matter-demo).

# Fork changes

This repository is forked from
[connectedhomeip](https://github.com/project-chip/connectedhomeip) with version
v1.2.0.1. It also comes with the following changes:

-   Rename `README.md`: The original `README.md` is renamed to `README.ori.md`.
-   Remove unrelated submodules: Due to the original repository updating all
    submodules, including solutions for non-Linux platforms, fetching this code
    takes a considerable amount of time. Additionally, the total size of the
    repository exceeds 20GB. To reduce the repository size, we need to remove
    submodules not marked as Linux platforms from .gitmodules.
