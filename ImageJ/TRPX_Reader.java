import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException; 
import java.io.IOException;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Scanner;
import java.util.regex.Pattern;
import ij.IJ;
import ij.ImagePlus;
import ij.process.ImageProcessor;
import ij.gui.NewImage;
import ij.ImageStack;
import ij.plugin.PlugIn;
import ij.io.OpenDialog;

public class TRPX_Reader implements PlugIn {

    public void run(String arg) {

        // Ask user to select a file
        OpenDialog dialog = new OpenDialog("Select a Terse/Prolix data file", null);
        String filePath = dialog.getPath();
        if (filePath == null) return;  // User cancelled the dialog
        if (!filePath.toLowerCase().endsWith(".trpx")) return;
        
		// Parameters required for unpacking the file
		long prolixBits = 0;
        long signed=1;
    	long block=0;
        long terseDataSize = 0;
        long imageSize = 0;
        long dataStartIndex;
        int dim0 = 0;
        int dim1 = 0;
        int nFrames = 1;

		// Get these parameters from the filename
    	try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
    		String line;
    		dataStartIndex = 0;
       		while ((line = br.readLine()) != null) {
       		    int indexTerse = line.indexOf("<Terse ");
           		if (indexTerse == -1) 
           			dataStartIndex += line.length() + 1;
				else {
                    int indexEndTag = line.indexOf("/>", indexTerse) + 2;
                   	if (indexEndTag != -1) {
               			dataStartIndex += indexEndTag;
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
			            	scanner.findWithinHorizon("<.*?prolix_bits=\"(\\d+)\".*?/>", 0);
               				prolixBits = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?signed=\"(\\d+)\".*?/>", 0);
               				signed = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?block=\"(\\d+)\".*?/>", 0);
               				block = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
              				scanner.findWithinHorizon("<.*?memory_size=\"(\\d+)\".*?/>", 0);
               				terseDataSize = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?number_of_values=\"(\\d+)\".*?/>", 0);
               				imageSize = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				if (scanner.findWithinHorizon("<.*?number_of_frames=\"(\\d+)\".*?/>", 0) != null) 
                				nFrames = Integer.parseInt(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
              				if (scanner.findWithinHorizon("<.*?dimensions=\"(\\d+)(?:\\s+(\\d+))?(?:\\s+(\\d+))?\".*?/>", 0) != null) {
               				    dim0 = Integer.parseInt(scanner.match().group(1));
   								dim1 = scanner.match().group(2) != null ? Integer.parseInt(scanner.match().group(2)) : dim1;
               				} else {
	                   			dim0 = (int) Math.sqrt(imageSize);
                   				dim1 = dim0;
       						}
           				}
               		}
            		break;
           		}
       		}
   		}
   		catch (FileNotFoundException e) {return;}
   		catch (IOException e) {return;}
   		
   		// Check if signed equals 0 and prolixBits equals 16
		if (signed != 0 || prolixBits > 16) {
    		System.err.println("Error: Invalid .trpx images for this plugin: images must be unsigned 16 bit.");
    		System.err.println("Data of " + filePath + " are " + ((signed == 0) ? "un" : "") + "signed " + prolixBits + " bit");
	    	return; // Exit the plugin
		}

   		// Create the image stack
		ImagePlus imageStack = NewImage.createShortImage(filePath, dim0, dim1, nFrames, NewImage.FILL_BLACK);
		
		// Read the Terse data from the .trpx file
		dTerseData = new byte[(int) (dataStartIndex + terseDataSize + 2)];
    	Path path = Paths.get(filePath);
    	try (FileInputStream fileInputStream = new FileInputStream(path.toFile())) {
    		fileInputStream.read(dTerseData);
 		} 
		catch (IOException e) { return; }
   		
		// Fill the image stack with the data read from the .trpx file	
		dBitStart = dataStartIndex * 8;
		for (int frameNumber = 1; frameNumber <= nFrames; ++frameNumber) {
	      	short significant_bits = 0;
			imageStack.setSlice(frameNumber);
			ImageProcessor ip = imageStack.getProcessor();
	        short[] pixels = (short[]) ip.getPixels();  
	        for (int from = 0; from < imageSize; from += block) {
	        	if (0 == ToShort(1)) 
    	    	    if (7 == (significant_bits = ToShort(3))) 
    		            if (10 == (significant_bits += ToShort(2))) 
 	        	           	significant_bits += ToShort(6);
	 	       	int to = (int) Math.min(imageSize, from + block);
	        	if (significant_bits == 0) 
    		        Arrays.fill(pixels, from, to, (short)0);
	    	    else 
        		    for (int j = from; j < to; ++j) 
                   		pixels[j] = ToShort(significant_bits);
       		}
        	dBitStart = (1 + (dBitStart >> 3)) << 3;
    	}

		// Display the image stack
        imageStack.show();
        IJ.run(imageStack, "Enhance Contrast", "saturated=0.35"); // Adjust saturation as needed
        imageStack.updateAndDraw();
	}

	private long dBitStart;
	private byte[] dTerseData;

    private short ToShort(int s) { 
		int indx = (int) dBitStart >> 3;          
		int trpx = ((dTerseData[indx] & 0xFF) +
                	((dTerseData[1 + indx] & 0xFF) << 8) +
                    ((dTerseData[2 + indx] & 0xFF) << 16));
        short result = (short)((trpx >>> (dBitStart & 7)) & ((1 << s) - 1));
        dBitStart += s;
        return result;
      }
 }
