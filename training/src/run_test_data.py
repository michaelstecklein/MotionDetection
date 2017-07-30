from os import listdir
import os.path
import sys
import numpy as np
import matplotlib.pyplot as plt

# Needs to be run outside of src/ and data/ directories






def get_variances(path):
	temp_file = "TEMP.temp"
	vars_tuples = []
	first_images = []
	ftmp = open ("{}/output".format(path),'w') # TODO remove
	for f in listdir(path):
		if os.path.isfile(path+'/'+f) and f.endswith("_1.jpg"):
			first_images.append(f)
	for img1 in first_images:
		img1_str = str(img1)
		img2_str = img1_str.replace('_1.jpg','_2.jpg')
		ret = os.system("./src/printvariances.bin ./{0}/{1} ./{0}/{2}  >  {3}".format(path,img1_str,img2_str,temp_file))
		if ret != 0:
			sys.exit("Error running c++ bin: {}".format(ret))
		f = open(temp_file, 'r')
		printout = f.read()
		print "IMAGES   ", img1_str, img2_str
		print "PRINTOUT  ", printout
		printout = printout.split("<vars>", 1)[-1]
		printout = printout.split("</vars>", 1)[0]
		tpl = printout.strip().split(',')
		vars_tuples.append(tpl)
		ftmp.write("{} {}    {}\n".format(img1_str,img2_str,tpl)) # TODO remove
		f.close()
		break # TODO remove
	os.system("rm {}".format(temp_file))
	ftmp.close() # TODO remove
	return vars_tuples


def print_list(name, lis):
	print name, ":::"
	for item in lis:
		print item

def reformat_tuple(tpl):
	list_a = []
	list_b = []
	for item in tpl:
		list_a.append(item[0])
		list_b.append(item[1])
	return (list_a, list_b)



get_variances("data/TEMP") # TODO remove
sys.exit("") # TODO remove
motion_vars = get_variances("data/motion")
nomotion_vars = get_variances("data/no_motion")
maybemotion_vars = get_variances("data/maybe_motion")

print_list("MOTION", motion_vars)
print_list("NO MOTION", nomotion_vars)
print_list("MAYBE MOTION", maybemotion_vars)

motion_data = reformat_tuple(motion_vars)
nomotion_data = reformat_tuple(nomotion_vars)
maybemotion_data = reformat_tuple(maybemotion_vars)


# scatter plot

data = (motion_data, nomotion_data, maybemotion_data)
colors = ("green", "red", "blue")
groups = ("motion", "no motion", "maybe motion") 
 
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1, axisbg="1.0")
 
for data, color, group in zip(data, colors, groups):
    x, y = data
    ax.scatter(x, y, alpha=0.8, c=color, edgecolors='none', s=30, label=group)
 
plt.title('Matplot scatter plot')
plt.xlabel('spatial variance')
plt.ylabel('pixel variance')
plt.legend(loc=2)
plt.show()
