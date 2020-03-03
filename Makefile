build: so-cpp.exe

so-cpp.exe: preprocessor.c
	cl /MD preprocessor.c 
	
clean : exe_clean obj_clean
 
obj_clean :
  del *.obj
 
exe_clean :
  del so-cpp.exe