##  TERSE C++ Fast and Lossless Diffraction Data Compressor


The objective of this project is to develop an *efficient compression* technique for *diffraction data*.

Terse<T> allows efficient and fast compression of integral diffraction data and other integral grayscale
data into a Terse object that can be decoded by the member function Terse<T>::prolix(iterator). The
prolix(iterator) member function decompresses the data starting at the location defined by 'iterator'
(which can also be a pointer). A Terse object is constructed by supplying it with uncompressed data or a
stream that contains Terse data

## How to compile and run it

    git clone git@github.com:Senikm/terse.git 
> Navigate to the project directory
```cmake

    mkdir build
    cd build
    cmake ..
    make 
```


### Running compression and decompression (MacOS build files are provided)

> Compression and decompression
``` c++

    ./terse *                   // all tiff files in this directory are compressed to terse files 
    
    ./prolix *                  // all terse files are expanded to tiff files

    ./terse ˜/dir/frame*.tiff  // compresses all tiff files in the directory ~/dir that start with frame\n"

    ./prolix ˜/dir/frame*.trs   // expands all terse files in the directory ~/dir that start with frame\n"

    ./terse -help              // All available options will be printed
```


## ImageJ plugin for .trs format files

> For compilation, use the java version that came with Fiji, to ensure java compatibility. Also make sure the ij-1.??.jar package is included in the compilation:
> For example, compile with:


    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/javac -cp /Applications/Fiji.app/jars/ij-1.53t.jar Terse_Reader.java

> Then create the .jar files with:

    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/jar -cvf Terse_Reader.jar Terse_Reader*.class

> Then copy Terse_Reader.jar to the "plugins" directory of Fiji:

    cp Terse_Reader.jar /Applications/Fiji.app/plugins/.

> Then restart Fiji, and Terse Reader is in the plugins menu.
    

### Test dataset:

https://drive.google.com/drive/folders/16UVHtia6GAK9WFO3RtO32tImhSlgdxEd?usp=sharing
