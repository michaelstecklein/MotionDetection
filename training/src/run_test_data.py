from os import listdir
import os.path
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import fsolve

# I understand this is ugly code. I wrote this quickly just to get the job done, and it does that so
# ugly it will remain!

# Needs to be run outside of src/ and data/ directories

PIXEL_VAR_THRESHOLD = 25 # greater than --> motion
SPATIAL_VAR_THRESHOLD = 425 # less than --> motion
point1 = (1227,159) # Manually enter points for the decision curve
point2 = (1400,886)

PRINT_ANNOTATIONS = True




def get_variances(path):
	temp_file = "TEMP.temp"
	vars_tuples = []
	first_images = []
	for f in listdir(path):
		if os.path.isfile(path+'/'+f) and f.endswith("_1.jpg"):
			first_images.append(f)
	for img1 in first_images:
		# run c++ bin on image pairs
		img1_str = str(img1)
		img2_str = img1_str.replace('_1.jpg','_2.jpg')
		ret = os.system("./src/printvariances.bin ./{0}/{1} ./{0}/{2}  >  {3}".format(path,img1_str,img2_str,temp_file))
		if ret != 0:
			sys.exit("Error running c++ bin: {}".format(ret))
		# move and rename diff image
		ret = os.system("mv diff_img.jpg ./{}/{}".format(path,img1_str.replace('_1.jpg','_diff.jpg')))
		ret += os.system("mv diff_img_com.jpg ./{}/{}".format(path,img1_str.replace('_1.jpg','_diff_com.jpg')))
		if ret != 0:
			sys.exit("Error running mv diff_img.jpg etc...")
		# read in output of c++ bin into array
		f = open(temp_file, 'r')
		printout = f.read()
		printout = printout.split("<vars>", 1)[-1]
		printout = printout.split("</vars>", 1)[0]
		tpl = printout.strip().split(',')
		vars_tuples.append((tpl,img1_str,img2_str))
		f.close()
	os.system("rm {}".format(temp_file))
	return vars_tuples


def print_list(name, lis):
	print name, ":::"
	for item in lis:
		print item

def reformat_tuple(tpl):
	list_a = [] # x's
	list_b = [] # y's
	list_c = [] # labels
	for item in tpl:
		list_a.append(item[0][0])
		list_b.append(item[0][1])
		list_c.append(item[1].replace("_1.jpg",""))
	return (list_a, list_b, list_c)



# start of main

motion_vars = get_variances("data/motion")
nomotion_vars = get_variances("data/no_motion")
maybemotion_vars = get_variances("data/maybe_motion")

print_list("MOTION", motion_vars)
print_list("NO MOTION", nomotion_vars)
print_list("MAYBE MOTION", maybemotion_vars)

motion_data = reformat_tuple(motion_vars)
nomotion_data = reformat_tuple(nomotion_vars)
maybemotion_data = reformat_tuple(maybemotion_vars)

# save results into variances-list.csv
fvarlist = open("data/results-variances-list.csv", 'w')
fvarlist.write("classification,img1,img2,spatial-variance,pixel-variance\n")
for entry in motion_vars:
	fvarlist.write("motion,\t{},\t{},\t{},\t{}\n".format(entry[1],entry[2],entry[0][0],entry[0][1]))
for entry in nomotion_vars:
	fvarlist.write("nomotion,\t{},\t{},\t{},\t{}\n".format(entry[1],entry[2],entry[0][0],entry[0][1]))
for entry in maybemotion_vars:
	fvarlist.write("maybemotion,\t{},\t{},\t{},\t{}\n".format(entry[1],entry[2],entry[0][0],entry[0][1]))
fvarlist.close()

# decision curve calculations
def equations(p):
	c,d = p
	def cost_func(x,y):
		return c*x**2 - d*y -1
	return (cost_func(point1[0],point1[1]), cost_func(point2[0],point2[1]))
c,d = fsolve(equations, (1,1))
a = c/d
b = 1/d
print "a:",a," b:",b," c:",c," d:",d


# PLOT
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1, axisbg="1.0")
data = (motion_data, nomotion_data, maybemotion_data)

# draw decision curve
max_x = max(max(data[0][0]),max(data[1][0]),max(data[2][0]))
x = np.linspace(0,int(float(max_x)),1000)
y = a*x**2 - b
'''
plt.plot(x,y)
plt.plot(point1[0],point1[1],'y^')
plt.plot(point2[0],point2[1],'y^')
'''

# draw thresholds lines
max_y = max(max(data[0][1]),max(data[1][1]),max(data[2][1]))
plt.plot([0,max_x],[PIXEL_VAR_THRESHOLD,PIXEL_VAR_THRESHOLD],'c:')
plt.plot([SPATIAL_VAR_THRESHOLD,SPATIAL_VAR_THRESHOLD],[0,max_y],'c:')
 
# scatter plot
# https://plot.ly/matplotlib/scatter/
colors = ("green", "red", "blue")
groups = ("motion", "no motion", "maybe motion") 
for data, color, group in zip(data, colors, groups):
	x, y, labels = data
	if PRINT_ANNOTATIONS: # point labels
		for i in range(len(x)):
			plt.annotate( labels[i], xy = (x[i],y[i]), xytext = (5,5), textcoords = 'offset points')
	ax.scatter(x, y, alpha=0.8, c=color, edgecolors='none', s=30, label=group)
 
# label plot
plt.title('Training Data Plot')
plt.xlabel('spatial variance')
plt.ylabel('pixel variance')
plt.legend(loc=2)
plt.show()
