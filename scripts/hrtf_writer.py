"""This is a helper class for scripts to ipmport from various HRTF sources."""

import numpy
import numpy.fft as fft
import struct
import enum
import itertools
import uuid


EndiannessTypes=enum.Enum("EndiannessTypes", "big little")

class HrtfWriter(object):
    endianness_marker = 1
    #The odd syntax here lets us put comments in.
    format_template="".join([
    "{}", #endianness and size indicator. This is platform-dependent and doesn't add anything to the string.
    "16B", #The uuid.
    "2i", #Endianness check marker, samplerate.
    "4i", #Response count, number of elevations, min elevation, max elevation.
    "{}", #Hole for the azimuth counts.
    "i", #Length of each response in samples.
    "{}", #hole for the responses.
    ])

    def __init__(self, samplerate, min_elevation, max_elevation, responses, endianness=EndiannessTypes.little, print_progress=True):
        """Parameters should all be integers:
        samplerate: obvious.
        min_elevation: Lowest elevation in degrees.
        max_elevation: Highest elevation in degrees.
        responses: List of lists of Numpy arrays in any format.
        Each sublist is one elevation, and they should be stored in ascending order (lowest elevation first).
        endianness: Endianness of the target CPU.
        print_progress: If true, using this class prints progress information to stdout.
        """
        self.samplerate=int(samplerate)
        self.elevation_count = len(responses)
        self.min_elevation = int(min_elevation)
        self.max_elevation = int(max_elevation)
        self.responses=responses
        self.endianness = endianness
        self.print_progress=print_progress
        #Some sanity checks.
        if self.elevation_count == 0:
            raise ValueError("No elevations!")
        self.azimuth_counts = []
        for i in responses:
            if len(i) ==0:
                raise ValueError("Elevation {} is empty.".format(i))
            self.azimuth_counts.append(len(i))
        response_lengths = [len(response) for elevation in self.responses for response in elevation]
        for i in response_lengths:
            if i != response_lengths[0]:
                raise valueError("Responses must all have the same length.")
        self.response_length = response_lengths[0]
        self.response_count=sum((len(elev) for elev in self.responses))
        if print_progress:
            print("basic sanity checks passed and HRTF Writer initialized.")
            print("Dataset has {} responses and {} elevations".format(self.response_length, self.elevation_count))
            print("sr =", self.samplerate)
            print(self.elevation_count, "elevations.")
            print("Min elevation =", self.min_elevation, "max elevation = ", self.max_elevation)
            print("Azimuth counts: {}".format(self.azimuth_counts))

    def make_format_string(self):
        endianness_token= "<" if self.endianness == EndiannessTypes.little else ">"
        self.format_string=self.format_template.format(endianness_token, str(len(self.azimuth_counts))+"i", str(self.response_count*self.response_length)+"f").encode('ascii')
        if self.print_progress:
            print("Format string:", self.format_string)

    def pack_data(self):
        self.make_format_string()
        iter = itertools.chain(
        uuid.uuid4().bytes, #Generate a 16-byte uuid.
        [self.endianness_marker, self.samplerate, self.response_count,
        self.elevation_count, self.min_elevation, self.max_elevation],
        self.azimuth_counts,
        [self.response_length],
        *[list(response) for elevation in self.responses for response in  elevation])
        data=list(iter)
        self.packed_data = struct.pack(self.format_string, *data)
        if self.print_progress:
            print("Data packed. Total size is {}.".format(len(self.packed_data)))

    def write_file(self, path):
        if not hasattr(self, 'packed_data'):
            raise ValueError("Must pack data first.")
        with open(path, "wb") as f:
            f.write(self.packed_data)
        if self.print_progress:
            print("Data written to {}".format(path))

    def data_to_float64(self):
        if self.print_progress:
            print("Converting data to float.")
        new_responses = []
        for elev in self.responses:
            new_elev = []
            for response in elev:
                if numpy.issubdtype(response.dtype, int):
                    minimum = numpy.iinfo(response.dtype).min
                    maximum = numpy.iinfo(response.dtype).max
                    subtract=0
                    if minimum == 0: #if the type is unsigned.
                        subtract = maximum/2
                    new_response = response.astype(numpy.int64)-subtract
                    new_response = new_response/float(maximum+1) #+1 guarantees that we have no values below -1
                    new_response = new_response.astype(numpy.float64)
                else:
                    new_response = response.astype(numpy.float64) #it's already a floating point type.
                new_elev.append(new_response)
            new_responses.append(new_elev)
        self.responses = new_responses

    def linear_phase(self):
        """Convert all the data to linear phase."""
        if self.print_progress:
            print("Converting data to linear phase...")
        new_responses=[]
        for elev in self.responses:
            new_elev=[]
            for response in elev:
                #ifft of the abs of the fft throws out all phase.
                #Because libaudioverse doesn't do minimum phase, it's automatically delayed and all is (theoretically) happy.
                #We overdo the fft to minimize any possible error, and then truncate.
                response = fft.irfft(numpy.abs(fft.rfft(response, max(self.response_length, 512)))).astype(numpy.float64)
                response = response[:self.response_length]
                new_elev.append(response)
            new_responses.append(new_elev)
        self.responses=new_responses


    def standard_build(self, path):
        """Does a standard build, that is the transformations that should be made on most HRIRs."""
        if self.print_progress:
            print("Standard build requested.")
        self.data_to_float64()
        self.pack_data()
        self.write_file(path)
