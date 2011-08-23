#ifndef DIRENT_H
#define DIRENT_H

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdio.h>
	struct dirent
	{
	  char * d_name;
	  int d_type;
	};

	#define DIR_TYPE 1

	typedef struct DIR DIR;

	DIR * opendir ( const char * dirname );
	struct dirent * readdir ( DIR * dir );
	int closedir ( DIR * dir );
	void rewinddir ( DIR * dir );


#ifdef __cplusplus 
}
#endif 

#endif // DIRENT_H
