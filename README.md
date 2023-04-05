# TERSE/PROLIX C++ Fast and Efficient diffraction data compressor


The objective of this project is to develop an *efficient compression* technique for *diffraction data*, followed by the application of machine learning algorithms to select positions of interest.

## How to run it

    git clone git@github.com:Senikm/simed.git 
> Navigate to the project directroy

    mkdir build
    cd build
    cmake ..
    make 

## Test dataset:

https://drive.google.com/drive/folders/16UVHtia6GAK9WFO3RtO32tImhSlgdxEd?usp=sharing

## Running compression and decompression with the provided build files (For MacOS only) or when compiled by the end-user

> Compression

    ./Compress -path "path to your folder containing the .tiff files"

> Decompression

    ./Decompress -path "path to your folder containing the .tiff files"


## ImageJ plugin

> For compilation, use the java version that came with Fiji, to ensure java compatibility. Also make sure the ij-1.??.jar package is included in the compilation:
> For example, compile with:

    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/javac -cp /Applications/Fiji.app/jars/ij-1.53t.jar Terse_Reader.java

> Then create the .jar files with: 
    Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/jar -cvf Terse_Reader.jar Terse_Reader*.class

> Then copy Terse_Reader.jar to the "plugins" directory of Fiji:

    cp Terse_Reader.jar /Applications/Fiji.app/plugins/.

> Then restart Fiji, and Terse Reader is in the plugins menu.
    


