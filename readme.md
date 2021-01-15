# deep_tag
### Quickly create PASCAL/VOC annotations from video (.mp4) files.
#### Uses **OpenCV** trackers to automatically position rectangles around moving objects, you just correct its mistakes.

#### (c) Tony Di Croce, dicroce@gmail.com

![Image of deep_tag](https://github.com/dicroce/deep_tag/raw/main/screenshot1.png)

### Supported Systems
1) Windows
2) Linux

Message me if you want to see a Mac release.

### Current Release
    https://github.com/dicroce/deep_tag/releases/download/1.0.0/deep_tag_setup.exe

### How To Use
1) Create a dataset project by picking a directory to put it in. Annotation and images will be stored in subdirectories.
2) Add classes to your project by clicking the + under the classes list.
3) Add and position rectangles: Pick your class, tracking algorithm and resizing options.
4) Hit enter to write annotions and images and advance to the next frame.

### Keyboard Shortcuts

    enter        - Write annotations and jpgs and advance to the next frame.
    W, A, S, D   - Move the currently selected rectangle around.
    ], [         - Increase & Decrease the size of the current rectangle.
    tab          - Cycle between rectangles.
    ', ;         - Vertically increase or decrease the size of the current rectangle.
    ,, .         - Horizontally increase or decrease the size of the current rectangle.
    shift + drag - Hold down shift and drag rectangles around.
