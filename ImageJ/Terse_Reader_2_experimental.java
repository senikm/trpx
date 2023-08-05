//package JPAlib.Java.TerseReader;

// For compilation, use the java version that came with Fiji, to ensure java compatibility. Also make sure the ij-1.??.jar package is included in the compilation:
// For example, compile with:
// /Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/javac -cp /Applications/Fiji.app/jars/ij-1.53t.jar TerseReader.java
// 
// Then create the .jar files with: 
// /Applications/Fiji.app/java/macosx/zulu8.60.0.21-ca-fx-jdk8.0.322-macosx_x64/jre/Contents/Home/bin/jar -cvf TerseReader.jar TerseReader*.class
//
// Then copy TerseReader.jar to the "plugins" directory of Fiji:
// cp TerseReader.jar /Applications/Fiji.app/plugins/.
//
// Then restart Fiji, and TerseReader is in the plugins menu.

import java.util.Arrays; // for Arrays.fill method
import ij.ImagePlus; // for ImagePlus class
import ij.process.ImageProcessor; // for ImageProcessor class
import ij.gui.NewImage; // for NewImage class
import ij.plugin.PlugIn;
import ij.ImagePlus;
import ij.io.OpenDialog;
import ij.gui.GenericDialog;
import ij.plugin.filter.PlugInFilter;
import ij.process.ImageProcessor;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.regex.MatchResult;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.regex.Pattern;


public class TerseReader implements PlugIn {

    public void run(String arg) {
        // Ask user to select a file
        OpenDialog dialog = new OpenDialog("Select a Prolix data file", null);
        String filePath = dialog.getPath();
        if (filePath == null) return;  // User cancelled the dialog

     	// Call the prolix function to read in the data
   		short[] data;
   		long dim0 = 0;
        long dim1 = 0;
        long dim2 = 0;
        long nFrames = 0;
		try {
    		TerseReader.Prolix prolix = new TerseReader.Prolix(); // create instance of inner class
    		data = prolix.run(filePath); // convert short[] to List<Short>
    		dim0 = prolix.dim0;
    		dim1 = prolix.dim1;
    		dim2 = prolix.dim2;
    		nFrames = prolix.number_of_frames;
		} catch (IOException e) {
    		e.printStackTrace();
    		return;
		}

        // Create a new image
        ImagePlus imp = NewImage.createShortImage("Prolix Data", dim0, dim1, dim2, 1, nFrames, NewImage.FILL_BLACK);
        ImageProcessor ip = imp.getProcessor();
        short[] pixels = (short[]) ip.getPixels();

        // Copy the data into the image
        for (int i = 0; i < data.length; i++) {
            pixels[i] = data[i];
        }

        // Display the image
        imp.show();
        imp.updateAndDraw();
    }

    public class Prolix {

        private byte[] d_terse_data;
        private long d_bit_start = 0;
        
        public long dim0 = 0;
        public long dim1 = 1;
        public long dim2 = 1;
        public long number_of_frames = 1;

		// Parameters required for unpacking .trs files
		private long d_prolix_bits;
        private long d_signed;
        private long d_block;
        private long d_terse_data_size;
        private long d_size;
        private long d_data_start_index;

        public short[] run(String filename) throws IOException {
        	prolixFileInfo(String filename);
	        // Read the terse pixel values into a byte array (a bit roundabout: first read in all the data into a buffer, then copies
	        // from the start of the terse data into a byte array that is 3 bytes larger than the total number of terse data. These extra
	        // 3 bytes are required to prevent an array overflow in BitRange.ToShort(int) for the last pixels).
	        byte[] buffer = Files.readAllBytes(Paths.get(filename));
	        d_terse_data = Arrays.copyOfRange(buffer, data_start_index, (int)(d_data_start_index + d_terse_data_size + 3));

			// The uncompressed data will be stored in prolix_data.
	        short[] prolix_data = new short[d_size * number_of_frames];

			// Unpack the data... here the magic happens.
			for (int i = 0; i != number_of_frames; ++i) {
	      		short significant_bits = 0;   
	        	for (int from = 0; from < d_size; from += d_block) {
	            	if (0 == ToShort(1)) 
	        	    	if (7 == (significant_bits = ToShort(3))) 
	    	            	if (10 == (significant_bits += ToShort(2))) 
	 	                   		significant_bits += ToShort(6);
	 	           	int to = (int) Math.min(d_size, from + d_block);
	        	    if (significant_bits == 0) 
	    	            Arrays.fill(prolix_data, from, to, (short)0);
		            else 
	        	        for (int i = from; i < to; ++i) 
	    	                prolix_data[i] = ToShort(significant_bits);
	            }
	            d_bit_start = 1 + ((d_bit_start >> 3) << 3);
 		    }
	        return prolix_data;
        }
        
        // Scans the file for the relevant Terse data and optional metadata, and sets the private parameters of the class accordingly.
		private void prolixFileInfo(String filename) throws IOException {
    		try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
        		String line;
        		d_data_start_index = 0;
        		while ((line = br.readLine()) != null) {
        		    int indexTerse = line.indexOf("<Terse ");
            		if (indexTerse == -1) 
            			d_data_start_index += line.length() + 1;
					else {
                    	int indexEndTag = line.indexOf("/>", indexTerse);
                    	if (indexEndTag != -1) {
                			d_data_start_index += indexTerse + indexEndTag + 2;
                			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
                			    scanner.findWithinHorizon("<.*?prolix_bits=\"(\\d+)\".*?/>", 0);
                    			d_prolix_bits = Long.parseLong(scanner.match().group(1));
                			    scanner.findWithinHorizon("<.*?signed=\"(\\d+)\".*?/>", 0);
                    			d_signed = Long.parseLong(scanner.match().group(1));
                			    scanner.findWithinHorizon("<.*?block=\"(\\d+)\"/>", 0);
                    			d_block = Long.parseLong(scanner.match().group(1));
                			    scanner.findWithinHorizon("<.*?number_of_values=\"(\\d+)\".*?/>", 0);
                    			d_size = Long.parseLong(scanner.match().group(1));
                			    scanner.findWithinHorizon("<.*?number_of_frames=\"(\\d+)\".*?/>", 0);
                			    if (scanner.match() != null) 
        							number_of_frames = Long.parseLong(scanner.match().group(1));
                    			number_of_frames = Long.parseLong(scanner.match().group(1));
                			    scanner.findWithinHorizon("<.*?dimensions=\"(\\d+)(?:\\s+(\\d+))?(?:\\s+(\\d+))?\".*?/>", 0);
                			    if (scanner.match().group(1) != null) {
    								dim0 = Long.parseLong(scanner.match().group(1));
    								dim1 = scanner.match().group(2) != null ? Long.parseLong(scanner.match().group(2)) : dim1;
    								dim2 = scanner.match().group(3) != null ? Long.parseLong(scanner.match().group(3)) : dim2;
        						} else {
 		                   			dim0 = Math.sqrt(d_size);
                    				dim1 = dim0;
        						}
        					}
                		}
                		break;
            		}
        		}
    		}
		}

        public short ToShort(int s) {           
       	     int trs = ((d_terse_data[(int)(d_bit_start >> 3)] & 0xFF) +
                        ((d_terse_data[(int)(1 + (d_bit_start >> 3))] & 0xFF) << 8) +
                        ((d_terse_data[(int)(2 + (d_bit_start >> 3))] & 0xFF) << 16));
            short result = (short)((trs >>> (d_bit_start & 7)) & ((1 << s) - 1));
            d_bit_start += s;
            return result;
        }
	}
}
