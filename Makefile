# Makefile for nmake
# `Microsoft Visual Studio Express 2013 for Windows Desktop'
# etc.

CC=cl
CFLAGS=

extract-authenticode.exe: extract-authenticode.c
	$(CC) $(CFLAGS) /Fe$@ $**
clean:
	-del *~
	-del *.obj
