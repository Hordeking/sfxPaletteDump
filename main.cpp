// sfxObjdump. Driver program to test sfxObj object and dump
// the contained model to wavefront obj. Copyright (C) 2018 JD Fenech
// (hordeking@users.noreply.github.com) based on sfxObjReader by
// Stéphane Dallongeville.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <getopt.h>


#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdint>

//#include "SFXObject.h"

using namespace std;

void print_usage(void) {
    cerr << "Usage: sfxpaldump <romfile> <texture data address> <texture data length> <output file>" << endl;
    cerr << endl;
    //cerr << "	-f <n>	Frame n only" << endl;
    //cerr << "	-b <basename>	Set base filename.\n\t\t\tOutput filenames will be be in format \"Basename nn\"" << endl;
    //cerr << "	-v	Verbose output" << endl;
    //cerr << "	-t	Validate only." << endl;
    //cerr << "\n\nsfxobjdump Copyright (C) 2018  JD Fenech\nThis is free software, and you are welcome to redistribute it under certain conditions;\nThis program comes with ABSOLUTELY NO WARRANTY; for details see COPYING file." << endl;
    //cerr << endl;
}


int main(int argc, char * argv[])
{

	int option = 0;
	size_t n = 0;
	bool verbose = false;
	bool testonly = false;
	bool singleframe = false;
	string base_filename("Model");

	uint32_t palette[16] = {
		0x000000,	//Transparent color.
		0xAA0841,
		0xF35969,
		0xFBAA69,
		0xFBD392,
		0x0051DB,
		0x618AFB,
		0x69DBFB,
		0xAAFBFB,
		0x183049,
		0x496171,
		0x71829A,
		0x9AB2C3,
		0xC3DBEB,
		0xEBFBFB,
		0x008A00,
	};

	cerr << argc << endl;

	if (argc<4) { print_usage(); return -1;}

	while ((option = getopt(argc, argv,"vtf:b:")) != -1) {
		switch (option) {

			case 'v' : //Verbose mode
				verbose = true;
				break;

			case 't' : //Validate object mode
				testonly = true;
				break;

			case 'b' : //List only
				base_filename = optarg;
				break;

			case 'f' :
				n = atoi(optarg);
				singleframe = true;
				break;

			default: print_usage();
				exit(EXIT_FAILURE);
		}
	}

	// TODO: Check that the user actually provided a filename for this.
	ifstream romfile(string(argv[optind]), ifstream::binary);
	if (!romfile.good()) { cerr << (string(argv[1]) + ": File not found, or unable to be opened.") << endl << endl; print_usage(); return -1;}

	//	outfile << stoull(string(argv[1]), nullptr, 0) << endl;
	cerr << argv[optind] << endl;
	cerr << stoull(string(argv[optind+1]), nullptr, 0) << endl;
	cerr << string(argv[optind+2]) << endl;

    size_t texdataoffset = stoull(string(argv[optind+1]), nullptr, 0); //= 0x090200;

    size_t texdatalength = stoull(string(argv[optind+2]), nullptr, 0);

    ofstream outfile(string(argv[optind+3]));
    if (!outfile.good()) {cerr << "Bad Output!" << endl; return -1;}

	//Since we're outputting a windows BMP, we need to set up the header for it.
	//It's a bit of a pain in the ass, but simpler than outputting png or gif.
	//At least we can output the basic stuff with a few lines of code.
    int iDummy = 0;

	const size_t numlines = texdatalength/512*2;

    //File Header (14 Bytes)
    outfile << "BM";		// Magic number, required
    iDummy = 14+40+1024+texdatalength; outfile.write( (char *) &iDummy, 4);	// The total file size. Some of these are a bit barmy, according to imagemagic.
    iDummy = 0; outfile.write( (char *) &iDummy, 4);
    iDummy = 14+40+16*4; outfile.write( (char *) &iDummy, 4);

    //BMP INFO Header (40 Bytes)
	iDummy = 40; outfile.write( (char *) &iDummy, 4);	//Info size
	iDummy = 512; outfile.write( (char *) &iDummy, 4);	//Image width
	iDummy = -numlines; outfile.write( (char *) &iDummy, 4);	//Image height
	iDummy = 1; outfile.write( (char *) &iDummy, 2); //Writing 01 00 08 00 to the file
	iDummy = 4; outfile.write( (char *) &iDummy, 2); //Bits per pixel.
	iDummy = 0; outfile.write( (char *) &iDummy, 4);	//Compression
	iDummy = texdatalength; outfile.write( (char *) &iDummy, 4);	//Image size, 0 if uncompressed
	iDummy = 3780; outfile.write( (char *) &iDummy, 4);	//XRes Pix Per Meter
	iDummy = 3780; outfile.write( (char *) &iDummy, 4);	//YRes Pix Per Meter
	iDummy = 16; outfile.write( (char *) &iDummy, 4);	//ncolors
	iDummy = 16; outfile.write( (char *) &iDummy, 4);	//important colors


    for(int i = 0; i<16; ++i){
		outfile.write( (char *)&(palette[i]),4);
    }

/*    for(int i = 16; i<256; ++i){
		iDummy = 0; outfile.write( (char *)&iDummy,4);
    }*/

    romfile.seekg(texdataoffset, ifstream::beg);
    cerr << romfile.tellg() << endl;

    for(size_t i = 0; i<numlines; ++i){
    	for(size_t j = 0; j<2; ++j){
			for(int k = 0; k<0x80; ++k){
				char buf = 0, outbuf = 0;

				// Pulls the odd byte into the upper nibble
				romfile.read(&buf, 1);
				(0==j)?(buf&=0x0f):buf=(buf>>4)&0x0f;
				outbuf = buf<<4;

				// Pulls the even byte into the lower nibble.
				romfile.read(&buf, 1);
				(0==j)?(buf&=0x0f):buf=(buf>>4)&0x0f;
				outbuf |= buf;

				// Spit out the completed 4bpp pixel pair to the file.
				outfile.write( (char *)&outbuf,1);
			}

			// This is responsible for de-interleaving the image data.
			// When j == 0, the code above is working on texture page 1.
			// When j == 1, the code above is working on texture page 2.
			// The line below determines if we've just finished texture pg 1.
			// If we have, we need to reset the one line so that we can run
			// it again to get page 2.
			(0==j)?romfile.seekg(-0x100, ifstream::cur):(romfile.seekg(0x00, ifstream::cur));
		}
    }

    outfile.close();
    romfile.close();

    return 0;
}
