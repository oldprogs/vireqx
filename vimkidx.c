#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "vireq.h"
DIR *dirp;
struct dirent *direntp;

#define _MAX_DRIVE 128
#define _MAX_DIR   512
#define _MAX_FNAME 512
#define _MAX_EXT   512


char dummy_string[512];
char ausgabe[512];
char eingabe[4096];
char dateiname[512];
FILE *config;
char configname[255];
char drive[_MAX_DRIVE];
char path[_MAX_DIR];
char file[_MAX_FNAME];
char ext[_MAX_EXT];

struct
{
    char drive[3];
    char unc[255];
} trans[30];
FILE *tmp;
char *p;
FILE *pfade;
FILE *pfade_c;
FILE *fbbs;
char pfadname[255];
char pfadcname[255];
struct
{
    char open;
    FILE *f;
    char name[255];
} idx[255];
struct
{
    char open;
    FILE *f;
    char name[255];
} idx_c[255];
char stattest[512];
struct stat buf;
char mmcopy=0;
char subdir[6000][255];
char start_dir[255];
int status=0;
char filesbbs[255];
unsigned long ii=0,jj=0,tempzaehl=0;
unsigned long i=0;
char ende=0;
char dirs_name[255];
unsigned char c;
void strlwr(char *);

void findfile(char *dir, char *name)
{
          char *p, *q;
          char entry[255];
	  #ifdef UNIX
	   ;
	  #else
	   strlwr(dir);
	  #endif
          dirp = opendir( dir );
          if( dirp == NULL ) {
	    printf(">>%s<<  \n",dir);
            perror( "Error " );
          } else {
              direntp = readdir( dirp );
          }
}
void findnext()
{
        direntp=readdir(dirp);
}
void findclose()
{
        closedir(dirp);
}
void strlwr(char *dummy)
{
          char *p;
          p=dummy;
          while(*p)
          {
             *p=tolower(*p);
             p++;
          }
}
void strupr(char *dummy)
{
          char *p;
          p=dummy;
          while(*p)
          {
             *p=toupper(*p);
             p++;
          }
}

int find_filesbbs(char *pfad)
{
	findfile(pfad,"*");
        if (!dirp)
        {
            findclose();
	   
            return(1);
        }
        while(direntp)
	{
		#ifdef UNIX
		 ;
		#else
		 strlwr(direntp->d_name);
		#endif
		if (strstr("files.bbx",direntp->d_name))
		{
			findclose();
			return(0);
		}
		findnext();
	}
	findclose();
	return(1);
}
void oeffne_idx(char c)
{

    sprintf(idx[c].name,"%svifile%c.idx",path,c);
    idx[c].f=fopen(idx[c].name,"wt");
    if (idx[c].f)
        idx[c].open=1;
    else
    {
        printf("Fehler beim Oeffnen von ");
        printf("%s",idx[c].name);
        perror("\nFehler ");
    }
}
void oeffne_idx_cdrom(char c)
{

    sprintf(idx_c[c].name,"%s%svifile%c.idc",drive,path,c);
    idx_c[c].f=fopen(idx_c[c].name,"wt");
    if (idx_c[c].f)
        idx_c[c].open=1;
    else
    {
        printf("Fehler beim Oeffnen von ");
        printf("%s",idx_c[c].name);
        perror("\nFehler ");
    }
}

void make_dirlist(char *dir)
{

    FILE *dirs;
    char *p;

    p=strchr(dir,0x0d);
    if (p)
       *p=0x00;
    p=strchr(dir,0x0a);
    if (p)
       *p=0x00;


    sprintf(dirs_name,"%sdirlist.dat",drive,path);
    dirs=fopen(dirs_name,"wt");

    sprintf(subdir[0],"%s",dir);
    sprintf(start_dir,"%s",subdir[0]);
    ii=1;
    jj=1;
    while(!ende)
    {
        findfile(start_dir,"*");
        if (direntp)
        {
            while(direntp)
            {
                if (ii==5990)
                   break;
		sprintf(stattest,"%s/%s",start_dir,direntp->d_name);
		stat(stattest,&buf);
		if (!S_ISDIR(buf.st_mode))
		{
			findnext();
			continue;
		}
                if (direntp->d_name[0]!='.')
		{
		    tempzaehl=jj;
		    tempzaehl--; 
                    sprintf(subdir[ii],"%s/%s",subdir[tempzaehl],direntp->d_name);
		    ii++;
		    printf("%u : %u --> %s\r",ii,jj,direntp->d_name);
		}
                findnext();
            }
	    findclose();
        }
        sprintf(start_dir,"%s",subdir[jj++]);
        if (jj > ii)
            break;
    }
    for (jj=0;jj<ii ;jj++ ) {
        fputs(subdir[jj],dirs);
        fputs("\n",dirs);
    }
    fclose(dirs);
}

void _outtext(char *text)
{
	printf(text);
	printf("\n");
}
int main(int argc, char *argv[])
{
    char *q;
    char mmcd=0, force=0;
    char param=0;
	char pfad[512];
    unsigned int nhandles;
    long zaehler=0;

    for (param=0;param<argc ; param++)
    {
        #ifdef UNIX
	 ;
	#else
	 strlwr(argv[param]);
	#endif
        if (!strncmp("-force",argv[param],strlen("-force")))
            force=1;
    }

    _outtext("VIMKIDX/Linux V0.07.2(vk)");
    _outtext("Creates all needed indexfiles for VIREQ/Linux");
    _outtext("Use -force to recreate the cdrom-indices");


    _outtext("Reading Paths.. ");
    _outtext("Path: ");
/*
    nhandles=_grow_handles(300);
    if (nhandles < 280)
    {
        _outtext("Ungenugende Anzahl Filehandles, Programmabbruch!");
        exit(200);
    }
*/


    sprintf(path,"/usr/bin/vireq/");

    sprintf(pfadname,"%svipath.idx",path);
    sprintf(pfadcname,"%svipath.idc",path);

    sprintf(configname,"%svireq.cfg",path);
    config=fopen(configname,"r");
    if (!config)
       exit(99);
    pfade=fopen(pfadname,"wt");
    if (!pfade) {
       exit(98);
    }
    if (force)
    {
        pfade_c=fopen(pfadcname,"wt");
        if (!pfade_c) {
           exit(96);
        }
    }
    while(!feof(config))
    {
        fgets(eingabe,(int)255,config);
        if (feof(config))
           break;
        #ifdef UNIX
	 ;
	#else
	    strlwr(eingabe);
	#endif
        p=strchr(eingabe,0x0d);
        if (p) *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p) *p=0x00;
        mmcd=0;
        mmcopy=0;
        if (!strncmp(eingabe,"include-file",strlen("include-file")))
        {
            p=strstr(eingabe," -cd");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcd=1;
            }
            if (mmcd && !force)
                continue;
            p=strstr(eingabe," -local");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcopy=1;
            }
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            q=strchr(p,0x20);
            if (q)
                *q=0x00;
            tmp=fopen(p,"rt");
            if (!tmp)
               continue;
            while(!feof(tmp))
            {
                fgets(eingabe,(int)255,tmp);
                if (feof(tmp))
                   break;
                p=strchr(eingabe,0x0d);
                if (p) *p=0x00;
                p=strchr(eingabe,0x0a);
                if (p) *p=0x00;
                p=strchr(eingabe,0x20);
                if (p) *p=0x00;
                if (!strlen(eingabe))
                    continue;
                #ifdef UNIX
		    ;
		#else
		    strlwr(eingabe);
		#endif
                if (eingabe[strlen(eingabe)-1]==0x5c)
                    eingabe[strlen(eingabe)-1]=0x00;
                if (mmcopy)
                    strcat(eingabe," -local");
                if (mmcd)
                {
                    fputs(eingabe,pfade_c);
                    fputs("\n",pfade_c);
                }
                else
                {
                    fputs(eingabe,pfade);
                    fputs("\n",pfade);
                }
                _outtext(eingabe);
                _outtext("\n\n");
            }
            fclose(tmp);
            continue;
        }
        if (!strncmp(eingabe,"include-path",strlen("include-path")))
        {
            char mmsub=0;
            p=strstr(eingabe," -s");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmsub=1;
            }
            p=strstr(eingabe," -S");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmsub=1;
            }
            p=strstr(eingabe," -t");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmsub=2;
            }
            p=strstr(eingabe," -T");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmsub=2;
            }
            p=strstr(eingabe," -cd");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcd=1;
            }
            p=strstr(eingabe," -CD");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcd=1;
            }
            p=strstr(eingabe," -local");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcopy=1;
            }
            p=strstr(eingabe," -LOCAL");
            if (p) {
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                *p++=0x20;
                mmcopy=1;
            }
            if (mmcd && !force)
                continue;
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            q=strchr(p,0x20);
            if (q)
                *q=0x00;
            if (!mmsub) {
                char *q;
                q=strchr(p,0x20);
                if (q)
                   *q=0x00;
                if (mmcopy)
                    strcat(p," -local");
                if (mmcd)
                {
                    fputs(p,pfade_c);
                    fputs("\n",pfade_c);
                }
                else
                {
                    fputs(p,pfade);
                    fputs("\n",pfade);
                }
                _outtext(p);
                _outtext("\n\n");
                continue;
            }

            make_dirlist(p);
            tmp=fopen(dirs_name,"rt");
            if (!tmp)
               continue;
            while(!feof(tmp))
            {
                fgets(eingabe,(int)255,tmp);
                if (feof(tmp))
                   break;
                p=strchr(eingabe,0x0d);
                if (p) *p=0x00;
                p=strchr(eingabe,0x0a);
                if (p) *p=0x00;
                #ifdef UNIX
		    ;
		#else
		    strlwr(eingabe);
		#endif
                if (eingabe[strlen(eingabe)-1]==0x5c)
                    eingabe[strlen(eingabe)-1]=0x00;
		status=find_filesbbs(eingabe);
                if (status==0 || mmsub==2)
                {
                    char *q;
                    q=strchr(eingabe,0x20);
                    if (q)
                       *q=0x00;
                    if (mmcopy)
                        strcat(eingabe," -local");
                    if (mmcd)
                    {
                        fputs(eingabe,pfade_c);
                        fputs("\n",pfade_c);
                    }
                    else
                    {
                        fputs(eingabe,pfade);
                        fputs("\n",pfade);
                    }
                    printf("%s\r",eingabe);
               }
            }
            fclose(tmp);
            continue;
        }
    }
    fclose(config);
    fclose(pfade);
    if (force)
        fclose(pfade_c);
//  Erstellen der VIFILE<x>.IDX

    for (i=0;i<256;i++) {
        idx[i].open=0;
    } /* endfor */
    _outtext("Reading Files... ");
    _outtext("File: ");

    pfade=fopen(pfadname,"rt");
    if (!pfade) {
       exit(90);
    }
    while(!feof(pfade))
    {
        zaehler++;
        fgets(eingabe,(int)255,pfade);
        if (feof(pfade))
            break;
        p=strchr(eingabe,0x0d);
        if (p) *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p) *p=0x00;
        p=strstr(eingabe," -local");
        if (p) {
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
        }
        p=strchr(eingabe,0x20);
        if (p) *p=0x00;
        sprintf(filesbbs,"%s/files.bbx",eingabe);
        fbbs=fopen(filesbbs,"rt");
        if (!fbbs)
        {
            findfile(eingabe,"*");
            if (direntp)
            {
                while(direntp)
                {
	                sprintf(stattest,"%s/%s",eingabe,direntp->d_name);
        	        stat(stattest,&buf);
	                if (S_ISDIR(buf.st_mode))
        	        {
                	        findnext();
	                        continue;
        	        }

                    p=direntp->d_name;
                    #ifdef UNIX
			;
		    #else
			strlwr(p);
		    #endif
                    c=*p;
                    if (!(c=='.'))
                    {
                        if (!idx[c].open)
                           oeffne_idx(c);
                        if (idx[c].open)
                        {
                            fprintf(idx[c].f,"%s %lu\n",p,zaehler-1);
                        }
                        printf("%s\r",p);
                    }
                    findnext();
                }
                findclose();
            }
            continue;
        }
        strcpy(pfad,eingabe);
        while (!feof(fbbs))
        {
            fgets(eingabe,(int)4000,fbbs);
            if (feof(fbbs))
               break;
            p=strchr(eingabe,0x0d);
            if (p) *p=0x00;
            p=strchr(eingabe,0x0a);
            if (p) *p=0x00;
            c=toupper(eingabe[0]);
            if (c==0x20 || c==0x0a || c==0x0d || c=='/' || c=='&' || c=='.')
                continue;
            if (c=='+'  || c=='*' || c=='\\' || c==0x00 || c==':')
                continue;
	    c=tolower(c);
	    #ifdef UNIX
		;
            #else
		strlwr(eingabe);
	    #endif
            if (!idx[c].open)
                oeffne_idx(c);
            if (!idx[c].open )
                continue;
            p=strchr(eingabe,0x20);
            if (p)
               *p=0x00;
            sprintf(dummy_string,"%s/%s",pfad,eingabe);
 	    if (access(dummy_string,F_OK))
                continue;
            fprintf(idx[c].f,"%s %lu\n",eingabe,zaehler-1);
            printf("%s\r",eingabe);
        }
        fclose(fbbs);
    }
    fclose(pfade);
    for (i=0;i<256;i++) {
       if (idx[i].open==1) {
          fclose(idx[i].f);
       }
    } /* endfor */
    if (!force)
    {
        for (i=0;i<256;i++) {
           if (idx_c[i].open==1) {
              fclose(idx_c[i].f);
           }
        } /* endfor */

        _outtext("Finished!\n\n");
        return(0);
    }

    pfade_c=fopen(pfadcname,"rt");
    if (!pfade) {
       exit(90);
    }
    zaehler=0;
    while(!feof(pfade_c))
    {
        zaehler++;
        fgets(eingabe,(int)255,pfade_c);
        if (feof(pfade_c))
            break;
        p=strchr(eingabe,0x0d);
        if (p) *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p) *p=0x00;
        p=strstr(eingabe," -local");
        if (p) {
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
            *p++=0x20;
        }
        p=strchr(eingabe,0x20);
        if (p) *p=0x00;
        sprintf(filesbbs,"%s/files.bbx",eingabe);
        fbbs=fopen(filesbbs,"rt");
        if (!fbbs)
        {
            findfile(eingabe,"*");
            if (direntp)
            {
                while(direntp)
                {
                    p=direntp->d_name;
                    #ifdef UNIX
			;
		    #else
		    strlwr(p);
		    #endif
                    c=*p;
                    if (!(c=='.'))
                    {
                        if (!idx_c[c].open)
                           oeffne_idx_cdrom(c);
                        if (idx_c[c].open)
                        {
                            fprintf(idx_c[c].f,"%s %lu\n",p,zaehler-1);
                        }
                        _outtext(p);
                        _outtext("\n\n");
                    }
                    findnext();
                }
                findclose();
            }
            continue;
        }
        while (!feof(fbbs))
        {
            fgets(eingabe,(int)4000,fbbs);
            if (feof(fbbs))
               break;
            p=strchr(eingabe,0x0d);
            if (p) *p=0x00;
            p=strchr(eingabe,0x0a);
            if (p) *p=0x00;
            c=toupper(eingabe[0]);
            if (c==0x20 || c==0x0a || c==0x0d || c=='/' || c=='&' || c=='.')
                continue;
            if (c=='+'  || c=='*' || c=='\\' || c==0x00 || c==':')
                continue;
	    #ifdef UNIX
		;
	    #else
	    strlwr(eingabe);
	    #endif
	    c=tolower(c);
            if (!idx_c[c].open)
                oeffne_idx_cdrom(c);
            if (!idx_c[c].open )
                continue;
            p=strchr(eingabe,0x20);
            if (p)
               *p=0x00;
            fprintf(idx_c[c].f,"%s %lu\n",eingabe,zaehler-1);
            _outtext(eingabe);
            _outtext("\n\n");
        }
        fclose(fbbs);
    }
    fclose(pfade_c);

    for (i=0;i<256;i++) {
       if (idx_c[i].open==1) {
          fclose(idx_c[i].f);
       }
    } /* endfor */

    _outtext("Finished!\n\n");
    return(0);
}

