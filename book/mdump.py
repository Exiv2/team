#!/usr/bin/env python3

import Tyf
import os
import sys

import urllib.request
from io  import BytesIO
from PIL import Image

##
#
def dumpTags(ifd):
	bDumpTags    = True
	bGenerateMap = False

	if bDumpTags:
		for tag in ifd:
			V=tag[1]
			v=str(V)
			if type(V)==type(''):
				v='"'+v+'"'
			if len(v) > 30:
			   v = v[0:26] + '.. '
			t=str(type(V))
			t=t[8:len(t)-2]
			if t == 'bytes':
				t=str(len(V)) + ' ' + t
			elif t == 'str':
				t=str(len(V))
			if len(t) > 30:
				t = t[0:26]+'.. '
				
			t='('+t+')'

			print('%s -> %s %s' % ( tag[0], v , t)	)

	if bGenerateMap:
		# MacOS Read this: https://stackoverflow.com/questions/50236117/scraping-ssl-certificate-verify-failed-error-for-http-en-wikipedia-org
		#
		# Maps Doc: https://developers.google.com/maps/documentation/maps-static/intro
		#
		# https://maps.googleapis.com/maps/api/staticmap?center=Brooklyn+Bridge,New+York,NY&zoom=13&size=600x300&maptype=roadmap
		# &markers=color:blue%7Clabel:S%7C40.702147,-74.015794&markers=color:green%7Clabel:G%7C40.711614,-74.012318
		# &markers=color:red%7Clabel:C%7C40.718217,-73.998284
		# &key=YOUR_API_KEY
		latitude  = ifd.gps['GPSLatitudeRef']  * ifd.gps['GPSLatitude']
		longitude = ifd.gps['GPSLongitudeRef'] * ifd.gps['GPSLongitude']
		format    = 'jpg' # png png8 png32 gif jpg jpg-baseline
		scale     = '1.0'
		size      = '256x256'
		mcolor    = 'blue'
		zoom      = '12'  # 1 = Globe, 14 = Streets
		maptype   = 'roadmap' # roadmap satellite hybrid terrain
		url       = """https://maps.googleapis.com/maps/api/staticmap?
		key=%s&center=%s,%s&zoom=%s&size=%s&markers=color:%s%%7C%s,%s&format=%s&scale=%s&maptype=%s
		""" % (
			os.environ.get('GOOGLE_MAPS_KEY'),
			latitude, longitude,
			zoom, size, mcolor,
			latitude, longitude,
			format, scale,maptype
		)
		# remove white space
		url = url.replace('\n', '').replace('\r', '').replace(' ','').replace('\t','')

		print("opening " + url);
		opener     = urllib.request.urlopen(url)
		image      = Image.open(BytesIO(opener.read()))
		image_path = "mdump"  +'.'+format;
		image.save(image_path)
		print("saved: "+image_path)
		os.system("open  "+image_path)

##
#
def mdump(argv):
	"""mdump - main program of course"""

	image = Tyf.open(argv[1])
	# help(jpg)

	if str(type(image)) == "<class 'Tyf.TiffFile'>":
		dumpTags(image[0])
	elif str(type(image)) == "<class 'Tyf.JpegFile'>":
		dumpTags(image.ifd0)
	else:
		print("unknown image type " + str(type(image)))

if __name__ == '__main__':
	mdump(sys.argv)

# That's all Folks
##
