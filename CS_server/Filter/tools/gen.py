axes=["x","y","z"]
def ran(grid,axis):
	print axes[axis] +":"
	for x in range ((grid-1)*10, (grid-1)*10 +4):
		for y in range ((grid-1)*10, (grid-1)*10 +4): 
			for z in range  ((grid-1)*10,  (grid-1)*10 +4):
				coord = [x*1.0,y*1.0,z*1.0] 
				print coord[axis],
			print
			
			
for grid in range(1,5):
	print "grid:",grid
	for axis in range(0,3):
		ran(grid,axis)
		print
