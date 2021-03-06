#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

int main(int argc,char* argv[])
{
	char cmd[2000] ;
	int  lcmd = lengthof(cmd)-100;
	int  l=0;

#ifdef __APPLE__
	for ( int i = 1 ; i < argc ; i++ ) {
		const char* filename = argv[i];
		if ( (strlen(filename)+l) < lcmd )
			l+=sprintf(cmd+l,"bbedit \"%s\"\n",filename) ;
	}
#else
	for ( int i = 1 ; i < argc ; i++ ) {
		const char* filename = argv[i];
		if ( (strlen(filename)+l) < lcmd )
			l+=sprintf(cmd+l,"/c/Program\\ Files/Sublime\\ Text\\ 2/sublime_text.exe  \"%s\" 2>&1 >/dev/null\n",filename) ;
	}
#endif
	if ( cmd[l] == '\n') l--;
	cmd[l]=0;
	puts(cmd);
	system(cmd) ;

	return 0 ;

    // unused
    argc ;
    argv ;
}
