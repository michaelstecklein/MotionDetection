from os import listdir
import os.path
import sys

# Needs to be run outside of src/ and data/ directories






def get_variances(path):
	temp_file = "TEMP.temp"
	first_images = []
	for f in listdir(path):
		if os.path.isfile(path+'/'+f) and f.endswith("_1.jpg"):
			first_images.append(f)
	for img1 in first_images:
		img1_str = str(img1)
		img2_str = img1_str.replace('_1.jpg','_2.jpg')
		ret = os.system("./src/printvariances.bin ./{0}/{1} ./{0}/{2}  >  {3}".format(path,img1_str,img2_str,temp_file))
		if ret != 0:
			sys.exit("Error running c++ bin: {}".format(ret))
		# TODO read out of temp file
	os.system("rm {}".format(temp_file))







motion_vars = get_variances("data/motion")
nomotion_vars = get_variances("data/no_motion")
maybemotion_vars = get_variances("data/maybe_motion")
