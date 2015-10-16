"""Loads the Diffused MIT KEMAR HRTF dataset included with this repository and converts it to a .hrtf file.  Pass the path to the data as the parameter to this script."""
import numpy
import scipy.io.wavfile as wavfile
from glob import glob
import sys
import os.path
import re
import itertools
import struct
import hrtf_writer

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: mit_hrtf.py <directory> <output_file>")
        exit()

    root_path = sys.argv[1]
    output_file = sys.argv[2]
    all_wavefiles = glob(root_path + '/*/*.wav')
    print("Enumerated wave files:", len(all_wavefiles), "found.")
    read_wavefiles = dict()

    print("Reading files.")
    for i in all_wavefiles:
        name = os.path.split(i)[1]
        read_wavefiles[name] = wavfile.read(i)

    print("Separating channels.")

    #we can discard samplerates at this point.
    for i, j in dict(read_wavefiles).items():
        arr = j[1]
        #Note: this is numpy slicing syntax.
        read_wavefiles[i] = (arr[:, 0], arr[:, 1])

    print("Extracting angles.")

    hrirs = dict()
    pattern = re.compile(r"H(-{0,1}\d+)e(\d+)a\.wav")

    for i, j in read_wavefiles.items():
        extracted = pattern.match(i)
        elevation = extracted.group(1)
        azimuth = extracted.group(2)
        elevation = int(elevation)
        azimuth = int(azimuth)
        hrirs[(elevation, azimuth)] = j

    print("Expanding 180-degree stereo responses to 360-degree mono responses.")
    for i, j in dict(hrirs).items():
        #0 and 180 are special: we average these instead.
        if i[1] in set([0, 180]):
            #The int16 is important here, as HrtfWriter tries to convert data for us.
            response = numpy.array([(a+b)/2.0 for a, b in zip(j[0], j[1])], dtype = numpy.int16)
            hrirs[i] = response
            continue
        #otherwise, we have two responses.  The first is at the current angle, and is taken as the right channel.
        #the second is 360 minus the current angle, and is the left channel from this file.
        hrirs[i] = j[1] #first of the pair.
        hrirs[(i[0], 360-i[1])] = j[0] #the second response.

    print("Computing data needed by HrtfWriter.")
    #compute the number of elevations:
    elev_list = list(set([i[0] for i in hrirs.keys()]))
    number_of_elevations = len(elev_list)
    elev_list.sort() #from least to greatest, per the hrir datafile format.

    #get min and max elevation.
    minimum_elevation = min(elev_list)
    maximum_elevation = max(elev_list)

    #now, compute the azimuths per elevation, and create a list of them.
    azimuth_list = []
    for elev in elev_list:
        azimuth_list.append(len([key for key in hrirs if key[0] == elev]))

    print("Preparing list of responses...")
    responses = []
    for ind, elev in enumerate(elev_list):
        new_elev = []
        azimuths = [i[1] for i in hrirs.keys() if i[0] == elev]
        azimuths.sort()
        for az in azimuths:
            new_elev.append(hrirs[(elev, az)])
        responses.append(new_elev)

    #At this point, we slot everything into an hrtfWriter.
    writer = hrtf_writer.HrtfWriter(samplerate= 44100, min_elevation = minimum_elevation, max_elevation = maximum_elevation, responses= responses)
    writer.standard_build(output_file)