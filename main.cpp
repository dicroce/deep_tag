#include "mainwindow.h"

#include <QApplication>


// Pascal VOC

//  <annotation>
//      <folder>Kangaroo</folder>
//      <filename>00001.jpg</filename>
//      <path>./Kangaroo/stock-12.jpg</path>
//      <source>
//          <database>Kangaroo</database>
//      </source>
//      <size>
//          <width>450</width>
//          <height>319</height>
//          <depth>3</depth>
//      </size>
//      <segmented>0</segmented>
//      <object>
//          <name>kangaroo</name>
//          <pose>Unspecified</pose>
//          <truncated>0</truncated>
//          <difficult>0</difficult>
//          <bndbox>
//              <xmin>233</xmin>
//              <ymin>89</ymin>
//              <xmax>386</xmax>
//              <ymax>262</ymax>
//          </bndbox>
//      </object>
//  </annotation>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
