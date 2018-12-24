#!/usr/bin/python

#*
#* This file is part of CPLEDBar.
#*                      ( The Crappy Pixel LED Bar )
#*
#* Copyright (C) 2018  D.Herrendoerfer
#*
#*   CPLEDBar is free software: you can redistribute it and/or modify
#*   it under the terms of the GNU General Public License as published by
#*   the Free Software Foundation, either version 2 of the License, or
#*   (at your option) any later version.
#*
#*   CPLEDBar is distributed in the hope that it will be useful,
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*   GNU General Public License for more details.
#*
#*   You should have received a copy of the GNU General Public License
#*   along with CPLEDBar.  If not, see <http://www.gnu.org/licenses/>.
#*
#
# This project is a realisation of the ideas presented by Ivan Miranda
# and Tom Stanton for a LED bar to project pictures and text into a long
# exposure photograph.
#

from PIL import Image
import os
import sys
from optparse import OptionParser

versionstring = "v0.0-alpha"

height = 252

options = {}

def main(argv):
    global versionstring
    global options
    # Main function everything starts here
    parser = OptionParser()
    parser.add_option("-i", "--imgfile", dest="imagefile",
                      help="read image from this FILE", metavar="FILE")

    (options, args) = parser.parse_args()

    if options.imagefile == None:
        print "You need to (at least) specify an image file."
        print "Try " +argv[0]+ " -h"
        sys.exit(1)

    imgfilename, imgfile_extension = os.path.splitext(options.imagefile)

    im = Image.open(options.imagefile)


    if im.mode != "RGBA":
        im = im.convert("RGBA")

    ximgsize,yimgsize = im.size

    px = im.load()

    f = open('out0.gb', 'wb')
    print "Writing to out0.gb"
    
    byte_arr = bytearray(height)

    for col in range(0,ximgsize):
        for row in range(0,height/3,3):
            red,green,blue,alpha = px[col,(row*3)+0]
            byte_arr[(row*3)+0] = green
            byte_arr[(row*3)+1] = red
            byte_arr[(row*3)+2] = blue

        binfmt = bytearray(byte_arr)
        f.write(binfmt)

        for row in range(1,height/3,3):
            red,green,blue,alpha = px[col,(row*3)+1]
            byte_arr[(row*3)+0] = green
            byte_arr[(row*3)+1] = red
            byte_arr[(row*3)+2] = blue

        binfmt = bytearray(byte_arr)
        f.write(binfmt)

        for row in range(2,height/3,3):
            red,green,blue,alpha = px[col,(row*3)+2]
            byte_arr[(row*3)+0] = green
            byte_arr[(row*3)+1] = red
            byte_arr[(row*3)+2] = blue

        binfmt = bytearray(byte_arr)
        f.write(binfmt)

    f.close()
    sys.exit(0)

if __name__ == "__main__":
    main(sys.argv)
 
