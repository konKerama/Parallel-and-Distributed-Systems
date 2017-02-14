# Script Used to test index values. 
# If any index breaks matrix limits, print informative message accordingly.

padSize = 2
(m,n) = (68,68)
(sizeX,sizeY) = (32,32)
matrixLimits = range(0,69)
# For each thread
#for i in range(2,66):
#	for j in range(2,66):
(i,j) = (60,60)
if (i < 32):
	if (j<32):
		(currentBlockX,currentBlockY) = (0,0)
	elif (j>=32 and j<64):
		(currentBlockX,currentBlockY) = (0,1)
	else:
		(currentBlockX,currentBlockY) = (0,2)
elif (i >= 32 and i <64):
	if (j<32):
		(currentBlockX,currentBlockY) = (1,0)
	elif (j>=32 and j<64):
		(currentBlockX,currentBlockY) = (1,1)
	else:
		(currentBlockX,currentBlockY) = (1,2)
else:
	if (j<32):
		(currentBlockX,currentBlockY) = (2,0)
	elif (j>=32 and j<64):
		(currentBlockX,currentBlockY) = (2,1)
	else:
		(currentBlockX,currentBlockY) = (2,2)
if (i>60):
	print "[+] In block (%d,%d)" % (currentBlockX,currentBlockY)
#for each neighbour
#(i,j) = (66,66)
for k in range(i-padSize,i+padSize+1):
	if k not in matrixLimits:
		print "[*] DEBUG k = %d ERROR OUT OF MATRIX X" % k
	for l in range(j-padSize, j+padSize+1):
		if k not in matrixLimits:
			print "[*] DEBUG k = %d ERROR OUT OF MATRIX X" % k
		a = k - currentBlockX * sizeX
		b = l - currentBlockY * sizeY
		#print "[+] a = %d | b = %d" %(a,b)

for row in range(currentBlockX*sizeX, (currentBlockX+1)*sizeX+1):
		for col in range(currentBlockY*sizeY, (currentBlockY+1)*sizeY+1):
			if not (row > (m-padSize) or col > (n-padSize)): # ?
			#	print "[-] DEBUG (row,col) = (%d, %d) out of bounds" % (row,col)
			#else:				
				for k in range(i-padSize,i+padSize+1):
					if k not in matrixLimits:
						print "[*] DEBUG k = %d ERROR OUT OF MATRIX X" % k
					for l in range(j-padSize, j+padSize+1):
						if k not in matrixLimits:
							print "[*] DEBUG k = %d ERROR OUT OF MATRIX X" % k
						a = k - currentBlockX * sizeX
						b = l - currentBlockY * sizeY
						print("[+] (a,b) = (%d,%d) for Block number (%d,%d)") % (a,b,currentBlockX, currentBlockY)



