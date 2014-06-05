from math import *
def direct(az, elev):
	az, elev = az/180.0*pi, elev/180.0*pi
	#r is 1.
	#the y component depends only on elev:
	y = sin(elev)
	#the x and z components are complex.
	#xz is the length of the hypotenuce of a right triangle such that the x-axis forms one of its sides.
	#the other is an imaginary line parallel to the z-axis.
	#we have the angle between the x axis and the point xz.
	xz = cos(elev)
	x = xz*cos(az)
	z = xz*sin(az)
	return (x, y, z)
