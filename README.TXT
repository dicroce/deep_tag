
deep_tag
Quickly create VOC annotations and images for training neural networks.

Compiling

Linux (Ubuntu)
    sudo apt install build-essential git cmake qtbase5-dev qtmultimedia5-dev
    git clone https://github.com/dicroce/deep_tag
    mkdir deep_tag/build
    pushd deep_tag/build
    cmake ..
    cmake --build .
    sudo make install        # everything is installed to /usr/local/deep_tag

Windows
    Install Visual Studio 2019 (Free Community edition is fine)
    Install cmake (let it modify your path)
    Install git with git bash
    Install Qt
       (Note: When installing Qt there is a point when you select what components to install. By
       default it will avoid installing Qt5 for Visual Studio. If it works you should end up with
       a C:\Qt directory containing a Qt release like C:\Qt\5.15.2.)
    Create a system environment variable called QTDIR=C:\Qt\5.15.2 or wherever your Qt is installed.
    Add C:\Qt5\5.15.2\msvc2019_64\bin to PATH environment variable
    Launch Git bash, cd to some work directory and type:

        git clone https://github.com/dicroce/deep_tag
        mkdir deep_tag/build
        pushd deep_tag/build
        cmake ..
        cmake --build .
        cmake --build . --target install

    At this point your build directory should contain an INSTALL_ROOT directory. deep_tag can run
    from this folder (and you should be able to move the folder wherever you'd like). At this point
    you could optionally install inno setup and then use it to open the deep_tag.iss file and generate
    a deep_tag_setup.exe.

Release Builds
    Modigy the "cmake --build ." line in the above instructions to be:
        "cmake --build . --config Release"
