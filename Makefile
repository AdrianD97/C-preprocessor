CC = cl
LINK = link
CFLAGS = /MD

build: so-cpp.exe

so-cpp.exe: preprocessor.obj
	$(LINK) /OUT:so-cpp.exe preprocessor.obj

preprocessor.obj: preprocessor.c
	$(CC) $(CFLAGS) preprocessor.c /Fopreprocessor.obj

clean : exe_clean obj_clean
 
obj_clean :
  del *.obj
 
exe_clean :
  del so-cpp.exe