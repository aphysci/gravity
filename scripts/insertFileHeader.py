"""
insertFileHeader.py

This routine is used to insert the Lesser General Public License header
statement in all of the Gravity source files.


@author: dmuench
"""

import os

# User Parameters
gravityDir = '/home/dmuench/gravity'

LGPLTextFiles = ['GravityLGPLHeader_cpp_java.txt','GravityLGPLHeader_shell.txt','GravityLGPLHeader_bat.txt']

fileExtensions_cpp_and_java = ('.cc', '.cpp', '.h', '.hh', '.java', '.proto', '.i')
fileExtensions_shell = ('akefile', '.in', '.mak', '.sh', '.config')
fileExtensions_bat = ('.bat')

APSstring = 'Applied Physical Sciences Corp.'

beginBashstring = 0
endBashstring = 11
bashString = '#!/bin/bash'

beginShstring = 0
endShstring = 9
shString = '#!/bin/sh'

fileMatches = []

for i in xrange(0,len(LGPLTextFiles)):
    # Read in the Gravity Lesser General Public License header statement
    LGPLTextFile = LGPLTextFiles[i]
    try:
        with open(LGPLTextFile, 'r') as LGPLFile:
            LGPLData = LGPLFile.read()
            LGPLFile.close()
            
    except IOError as err:
        print('Error while reading ' +LGPLTextFile+': ' + str(err))
                
    # Coordinate the file extensions to the current LGPL header file
    if(i==0):
        fileExtensions = fileExtensions_cpp_and_java
    elif(i==1):
        fileExtensions = fileExtensions_shell
    else:
        fileExtensions = fileExtensions_bat
        
        
    # Generate the file names in a directory tree by walking the tree top-down, based on the fileExtensions list
    for dirPath, dirNames, fileNames in os.walk(gravityDir, True):
        fileMatches.extend(os.path.join(dirPath, fileName) 
            for fileName in fileNames if fileName.lower().endswith(fileExtensions))

    # Loop through the file matches and determine which files need the LGPL header 
    for x in xrange(0,len(fileMatches)):   
       try:
           with open(fileMatches[x], 'r+') as outFile:
               sourceData = outFile.read()
               outFile.seek(0)

               # Only write the LGPL header to files that do not currently have it
               if(sourceData.find(APSstring) == -1):
                   print('Appending File #'+str(x)+': ' +fileMatches[x])
                       
                   # Look for the Shebang bash and sh interpreter. If found, put the LGPL header after it
                   if(sourceData[beginBashstring:endBashstring] == bashString):
                       outFile.write(sourceData[beginBashstring:endBashstring]+'\n'+LGPLData+'\n'+sourceData[endBashstring+1:])
                   elif(sourceData[beginShstring:endShstring] == shString):
                       outFile.write(sourceData[beginShstring:endShstring]+'\n'+LGPLData+'\n'+sourceData[endShstring+1:])
                   else:
                       outFile.write(LGPLData+'\n'+sourceData)

               outFile.close()
       except IOError as err:
           print('Error while appending LPGL header: ' + str(err))   

