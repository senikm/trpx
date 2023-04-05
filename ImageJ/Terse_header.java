// Package JPAlib.Java.Terse_Reader;

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


public class Terse_Reader implements PlugIn {

    public void run(String arg) {
        // Ask user to select a file
        OpenDialog dialog = new OpenDialog("Select a Prolix data file", null);
        String filePath = dialog.getPath();
        if (filePath == null) return;  // User cancelled the dialog

     	// Call the prolix function to read in the data
   		short[] data;
		try {
    		Terse_Reader.Prolix prolix = new Terse_Reader.Prolix(); // create instance of inner class
    		data = prolix.run(filePath); // convert short[] to List<Short>
		} catch (IOException e) {
    		e.printStackTrace();
    		return;
		}

        // Create a new image
        ImagePlus imp = NewImage.createShortImage("Prolix Data", 512, 512, 1, NewImage.FILL_BLACK);
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

        public short[] run(String filename) throws IOException {

		// Parameters required for unpacking .trs files
		long d_prolix_bits;
        long d_signed;
        long d_block;
        long d_terse_data_size;
        long d_size;

		// Read the XML header of a .trs file in Terse format
		String terse_header;
		BufferedReader br = new BufferedReader(new FileReader(filename));
		terse_header = br.readLine();
    
		// Scan the header to get the parameters required for unpacking the .trs file
        Scanner scanner = new Scanner(terse_header);
        scanner.findWithinHorizon("<Terse prolix_bits=\"(\\d+)\" signed=\"(\\d+)\" block=\"(\\d+)\" memory_size=\"(\\d+)\" number_of_values=\"(\\d+)\"/>", 0);
        MatchResult match = scanner.match();
        d_prolix_bits = Long.parseLong(match.group(1));
        d_signed = Long.parseLong(match.group(2));
        d_block = Long.parseLong(match.group(3));
        d_terse_data_size = Long.parseLong(match.group(4));
        d_size = Long.parseLong(match.group(5));

		// Find the start of the terse pixel values
        int data_start_index = terse_header.indexOf("/>") + 2;
        
        // Read the terse pixel values into a byte array (a bit roundabout: first read in all the data into a buffer, then copies
        // from the start of the terse data into a byte array that is 3 bytes larger than the total number of terse data. These extra
        // 3 bytes are required to prevent an array overflow in BitRange.ToShort(int) for the last pixels).
        byte[] buffer = Files.readAllBytes(Paths.get(filename));
        d_terse_data = Arrays.copyOfRange(buffer, data_start_index, (int)(data_start_index + d_terse_data_size + 3));

		// The uncompressed data will be stored in prolix_data.
        short[] prolix_data = new short[512*512];

		// Unpack the data... here the magic happens.
      	short significant_bits = 0;   
        for (int from = 0; from < d_size; from += d_block) {
            int to = (int) Math.min(d_size, from + d_block);
            if (ToShort(1) == 0) {
                if (7 == (significant_bits = ToShort(3))) {
                    if (10 == (significant_bits += ToShort(2))) {
                        significant_bits += ToShort(6);
                    }
                }
            }
            if (significant_bits == 0) {
                Arrays.fill(prolix_data, from, to, (short)0);
            } else {
                for (int i = from; i < to; ++i) {
                    prolix_data[i] = ToShort(significant_bits);
                    }
                }
            }
            return prolix_data;
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
