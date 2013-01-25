#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define _MAX_DRIVE 3
#define _MAX_DIR 255
#define _MAX_FNAME 128
#define _MAX_EXT 12
#define MAX_FILENAMELEN _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT
#include <dirent.h>
#include "vireq.h"
#include "version.h"

DIR *dirp;
struct dirent *direntp;
struct stat datstat;
struct
{
      char name[MAX_FILENAMELEN];
      char passwort[20];
      long area;
      char wildcard;
      char found;
      char mmcd;
} found[512];

char aliaslist[100][MAX_FILENAMELEN];
unsigned int alias_max=0;


struct
{
    char open;
    FILE *f;
    char name[MAX_FILENAMELEN];
} idx[255];

struct
{
    char open;
    FILE *f;
    char name[MAX_FILENAMELEN];
} idx_c[255];

FILE *pfade;
char pfadfile[MAX_FILENAMELEN];
FILE *pwfile;
char pwfilename[MAX_FILENAMELEN];

unsigned int i;
unsigned int j;
char c,pwok;
time_t aktuelles;
char statname[MAX_FILENAMELEN];
char mmnewest, mmexecute;
char eingabe[512];
char cdtemp[512];
char ausgabe[512];
char dateiname[255][MAX_FILENAMELEN];
unsigned long area[255];
char geoeffnet=0;
unsigned long max;
char systemname[512];

char srifname[MAX_FILENAMELEN];
FILE *srif;
char requestname[MAX_FILENAMELEN];
FILE *request;
char responsename[MAX_FILENAMELEN];
FILE *response;
char *p;
char drive[_MAX_DRIVE];
char path[_MAX_DIR];
char file[_MAX_FNAME];
char ext[_MAX_EXT];
char adrive[_MAX_DRIVE];
char apath[_MAX_DIR];
char afname[_MAX_FNAME];
char aext[_MAX_EXT];
unsigned short task;
char logname[MAX_FILENAMELEN];
char dlcname[MAX_FILENAMELEN];
char *taska;
char tempname[MAX_FILENAMELEN];

void write_log (char *);
unsigned int zone_to, net_to, node_to, point_to=0;
unsigned int zone_from=2, net_from=246, node_from=2098, point_from=0;
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
void ExpandPath(char *path)
{
	return;
}
char strnicmp(char *string1,char *string2, size_t laenge)
{
        char s1[2048];
        char s2[2048];
        char *p,*q;
        strcpy(s1,string1);
        strcpy(s2,string2);
        s1[laenge]=0x00;
        s2[laenge]=0x00;
        strlwr(s1);
        strlwr(s2);
        return(strcmp(s1,s2));
}

char pwcheck(char *filename, char *password)
{
    char *p, *q;
    char pwpfad[MAX_FILENAMELEN];
	char dummy1[512];
    pwok=0;
    strlwr(filename);
    strlwr(password);

    pwfile=fopen(pwfilename,"rt");
    if (!pwfile)
        return(1);
    while(1)
    {
        fgets(pwpfad,(int) MAX_FILENAMELEN - 2,pwfile);
        if (feof(pwfile))
        {
            pwok=1;
            break;
        }
        strlwr(pwpfad);
        p=strchr(pwpfad,0x0d);
        if (p)
            *p=0x00;
        p=strchr(pwpfad,0x0a);
        if (p)
            *p=0x00;
        q=strchr(pwpfad,0x20);
        if (q)
            *q=0x00;
        p=strstr(filename,pwpfad);
        if (q)
            *q=0x20;
        if (p)
        {
            p=strchr(pwpfad,0x20);
            if (p)
            {
                while(*p++==0x20);
                p--;
                if (!strcmp(p,password))
                {
                    pwok=1;         // passwort passt -> OK
                    break;
                }    
                else
                {
                    pwok=0;         // Passwort pass nicht -> NOK
		    sprintf(dummy1,"Password expected for %s : %s\n",filename,p);
		    write_log(dummy1);
		    sprintf(dummy1,"Password received : %s",password);
		    write_log(dummy1);
                    break;
                }
            }
            else
            {
                pwok=1;             // kein pw im Pw-File... (?)
            }
        } 
    }
    fclose(pwfile);
    return(pwok);
}

char copypuffer[65535];

char copy64k(char *quellpfad,char *zielpfad,char *dateiname)
{
    FILE *in,*out;
    unsigned long gelesen;
    char ein[512],aus[512];

    sprintf(ein,"%s/%s",quellpfad,dateiname);
    sprintf(aus,"%s/%s",zielpfad,dateiname);

    in=fopen(ein,"rb");
    if (!in) 
       return(1);
    out=fopen(aus,"wb");
    if (!out)
    {
        fclose(in);
        return(1);
    }
    while(1)
    {
        gelesen=fread(&copypuffer[0],(size_t)1, (size_t)65535,in);
        if (!gelesen) 
           break;
        fwrite(&copypuffer[0],(size_t) 1, (size_t) gelesen,out);
    }
    fclose(in);
    fclose(out);
    return(0);
}


char ungleich(char *q, char *z)
{
    char sichern=0;
    char mmaufsetz=0;
    char gesichert[60];
    char *aufsetz;
    char *p;
    char *r;
    char ungleich=0;
    r=q;
    strcpy(gesichert,q);
    while(1)
    {
        if (!*q)
        {
            if (mmaufsetz)
            {
                mmaufsetz=1;
            }
            else
            {
                if (*z)
                    ungleich=1;
                break;
            }
        }
        if (!*z)
        {
            if (!*q)
                break;
            if (*q!='*')
               ungleich=1;
            if (*q=='*')
                if (*(q+1))
                    ungleich=1;
            break;
        }
        if (*q==*z || *q=='?')
        {
            q++;
            z++;
            continue;
        }
        if (*q=='*')
        {
            q++;
        }
        if (*(q-1)=='*' || (*q != *z && mmaufsetz))
        {
            if (mmaufsetz && ! (*(q-1)=='*'))
                q=aufsetz;
            if (*(q-1)=='*')
            {
                if (!*q)
                    break;
            }
            p=q;
            while(*p)
            {
                if (*p=='*' || *p=='?')
                    break;
                p++;
            }
            if (*p=='*' || *p=='?')
            {
                sichern=*p;
                *p=0x00;
            }
            if (!strstr(z,q))
            {
                ungleich=1;
                break;
            }
            else
            {
                z=strstr(z,q);
                aufsetz=q;
                if (strstr(z+1,q))
                    mmaufsetz=1;
                if (sichern)
                {
                    *p=sichern;
                    sichern=0;
                }
                continue;
            }
        }
        if (*q != *z)
        {
            ungleich=1;
            break;
        }
    }
    strcpy(r,gesichert);
    return(ungleich);
}

void write_temp(char *log_zeile)
{
    FILE *tmp;
    tmp=fopen(tempname,"at");
    if (!tmp)
        return;
    fputs(log_zeile,tmp);
    fputs("\x0d",tmp);
    fclose(tmp);
}

void write_dlc(char *log_zeile)
{
    FILE *tmp;
    if (!strlen(dlcname)) 
        return;
    tmp=fopen(dlcname,"at");
    if (!tmp)
        return;
    fputs(log_zeile,tmp);
    fputs("\n",tmp);
    fclose(tmp);
}

int status;

#define uint unsigned short
#define uchar unsigned char

struct pkt_1
{
    uint onode,
         dnode;
    uint jahr,
        monat,
        tag,
        stunde,
        minute,
        sekunde;
    uint baud;
    uint fix;
    uint onet,
         dnet;
    uchar pcode;
    uchar rev_major;
    uchar pw[8];
    uint ozone,
         dzone;
    uint auxnet;
    uint cw1;
    uchar pcode2,
          revision;
    uint cw2;
    uint ozone2,
         dzone2;
    uint opoint,
         dpoint;
    uchar pdata[4];
} packet;
        char dirname[255];
        char filename[255];

struct d
{
unsigned short node_from;
unsigned short node_to;
unsigned short net_from;
unsigned short net_to;
unsigned short attribute;
unsigned short cost;
} pp;


char datum[20];

#define NM_PRIVATE  1
#define NM_CRASH    2
#define NM_KILLSENT 128
#define NM_LOCAL    256
#define NM_HOLD     512
#define NM_DIRECT   1024

char pfad2[255];

char dummy_string2[80];
int write_pkt(char *pfad, char *sysop_from, char* sysop_to,
            uint zone_from, uint net_from,
            uint node_from, uint point_from,
            uint zone_to, uint net_to,
            uint node_to, uint point_to,
            char *call_subject, char *filename)
{
    FILE *pkt,
         *tmp;
    char zeit[10];
    uint fix2=2;
    int day;
    char dummy_string[100];
    unsigned long anz;
    struct
    {
        char Day[4];
        char Month[4];
        char DayNr[3];
        char filler[11];
        char Year[2];
    } zeit1;
    char *p1;
    time_t ltime;
    struct tm *gmt;

    time (&ltime);
    gmt = localtime (&ltime);
/*
    _strtime(zeit);
    _strdate(datum);
*/
    strftime(zeit,10,"%H:%M:%S",gmt);

    day=gmt->tm_wday;

    p1=asctime(gmt);
    strncpy(zeit1.Day,p1,(int)4);
    strncpy(zeit1.Month,p1+4,(int) 4);
    strncpy(zeit1.DayNr,p1+8,(int) 3);
    strncpy(zeit1.Year,p1+22,(int) 2);

    if (!strnicmp(zeit1.Month,"Dez",3))
        strcpy(zeit1.Month,"Dec");

    if (!strnicmp(zeit1.Month,"Mae",3))
        strcpy(zeit1.Month,"Mar");

    if (!strnicmp(zeit1.Month,"Mai",3))
        strcpy(zeit1.Month,"May");

    if (!strnicmp(zeit1.Month,"Okt",3))
        strcpy(zeit1.Month,"Oct");

loop:
    time(&ltime);
    if (pfad[strlen(pfad)-1] != '/')
        strcat(pfad,"/");
    strcpy(pfad2,pfad);
    sprintf(dummy_string,"%8lx",ltime);
    strcat(pfad2,dummy_string);
    strcat(pfad2,".pkt");
    if (!access(pfad2,(int)0x00))
        goto loop;

    pkt=fopen(pfad2,"wb");

    if (!pkt)
        return(21);

    packet.onode=node_from;
    packet.dnode=node_to;
    packet.jahr=gmt->tm_year;
    packet.monat=gmt->tm_mon;
    packet.tag=gmt->tm_mday;
    packet.stunde=gmt->tm_hour;
    packet.minute=gmt->tm_min;
    packet.sekunde=gmt->tm_sec;
    packet.baud=0x02;
    packet.fix=0x02;
    packet.auxnet=0x0000;
    if (!point_from)
        packet.onet=net_to;
    else
    {
        packet.onet=0xffff;
        packet.auxnet=net_to;
    }
    packet.dnet=net_to;
    packet.pcode=0x00;
    memset(packet.pw,0x00,(size_t)8);
    packet.ozone=zone_from;
    packet.dzone=zone_to;
    packet.cw1=0x0100;
    packet.pcode=0x00;
    packet.rev_major=0x01;
    packet.revision=0x30;
    packet.cw2=0x0001;
    packet.ozone2=zone_from;
    packet.dzone2=zone_to;
    packet.dpoint=point_to;
    packet.opoint=point_from;
    fwrite(&packet,(size_t) sizeof(packet), (size_t) 1, pkt);
    fwrite(&fix2,(size_t) sizeof(fix2),(size_t) 1, pkt);

    pp.net_to=net_to;
    pp.net_from=net_from;
    pp.node_from=node_from;
    pp.node_to=node_to;
    pp.attribute = NM_PRIVATE;
    pp.cost=0;
    fwrite(&pp,(size_t) sizeof(pp), (size_t) 1, pkt);


    for (anz = 0; anz < sizeof(datum); anz++)
        datum[anz]=0x00;
    strncpy(datum,zeit1.DayNr, (size_t) 3);
    strncat(datum,zeit1.Month, (size_t) 3);
    strcat(datum," ");
    strncat(datum,zeit1.Year, (size_t) 2);
    strcat(datum,"  ");
    strncat(datum,zeit,(int) 2);
    strcat(datum,":");
    strncat(datum,zeit+3,(int) 2);
    strcat(datum,":");
    strncat(datum,zeit+6,(int) 2);
    fwrite(datum,(size_t) strlen(datum), (size_t) 1, pkt);
    fputc(0x00,pkt);
    fwrite(sysop_to,(size_t) strlen(sysop_to),1,pkt);
    fputc(0x00,pkt);
    fwrite(sysop_from,(size_t) strlen(sysop_from),1,pkt);
    fputc(0x00,pkt);
    fwrite(call_subject,(size_t) strlen(call_subject),1,pkt);
    fputc(0x00,pkt);
/*
    fputc(0x01,pkt);
    fwrite("AREA:",(size_t) strlen("AREA:"),1,pkt);
    fwrite(area,(size_t) strlen(area),1,pkt);
    fputc(0x0d,pkt);
*/
    sprintf(dummy_string,"\x01INTL %d:%d/%d %d:%d/%d\x0d",zone_to, net_to,
            node_to, zone_from, net_from, node_from);
    fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);

    if (point_from)
    {
        sprintf(dummy_string,"%cFMPT %d\x0d",0x01, point_from);
        fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);
    }
    if (point_to)
    {
        sprintf(dummy_string,"\x01TOPT %d\x0d", point_to);
        fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);
    }


    sprintf(dummy_string,"\x01MSGID: %d:%d/%d.%d %8lx\x0d",zone_from,net_from,node_from,point_from,ltime);
    fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);

    sprintf(dummy_string,"\x01PID: VIREQ/Linux V%s\x0d",VERSION);
    fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);

    tmp=fopen(filename,"rb");

    if (!tmp)
        return(22);
    while(!feof(tmp))
    {
        anz=fread(dummy_string,(size_t) 1, (size_t) 80, tmp);
        if (anz)
            fwrite(dummy_string,(size_t) anz, (size_t) 1, pkt);
    }
    if (!point_from)
        sprintf(dummy_string,"\x0d--- VIREQ/Linux V%s by Volker Imre \x0d * Origin: - (%d:%d/%d)\x0d",VERSION,
        zone_from, net_from, node_from);
    else
        sprintf(dummy_string,"\x0d--- VIREQ/Linux V%s\x0d * Origin: (c) 1998-2001 by Volker Imre, Fidonet 2:246/2098 (%d:%d/%d.%d)\x0d",VERSION,
        zone_from, net_from, node_from, point_from);
    fwrite(dummy_string,(size_t) strlen(dummy_string), (size_t) 1, pkt);

    fputc(0x00,pkt);
    fputc(0x00,pkt);
    fputc(0x00,pkt);
    fclose(pkt);
    fclose(tmp);
    return(0);
}

void write_log(char *log_zeile)
{
    char errorlevel=0;
    FILE *logfile;

    char zeit[10];
    char logger[256];
    char *x;
    static is_open=0;
    char level[]={" !?+-~"};
    time_t akttime;
    struct tm *d2;
    char mon[][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    if (!strlen(logname))
        return;

    time(&akttime);
    d2=localtime(&akttime);

/*    _strtime(zeit);*/
    strftime(zeit,10,"%H:%M:%S",d2);
    x=strstr(log_zeile,"\n");
    if (x)
        *x=0x00;

    logfile=fopen(logname,"a");
    if (!is_open)
    {
        sprintf(logger,"\n %2.2d %s %s VIR  begin, VIREQ/Linux V%s\n",d2->tm_mday,mon[d2->tm_mon],zeit,VERSION);
        fputs(logger,logfile);
        is_open=1;
    }
    sprintf(logger," %2.2d %s %s VIR  %s\n",d2->tm_mday,mon[d2->tm_mon],zeit, log_zeile);

    fputs(logger,logfile);
    if (logfile) fclose(logfile);
    return;
}
void findnext()
{
        while(1)
        {
		if (!direntp)
			break;
                direntp=readdir(dirp);
		if (!direntp)
			break;
		if (!ungleich(filename,direntp->d_name))
			break;
        }
}
void findclose()
{
        closedir(dirp);
}

void findfile(char *name)
{
	char *p,*q;
	p=strrchr(name,'/');
	if (!p)
	{
		direntp=NULL;
		return;
	}
	q=p+1;
	*p=0x00;
	strcpy(dirname,name);
	strcpy(filename,q);
	p=strchr(filename,0x20);
	if (p)
		*p=0x00;
        dirp = opendir( dirname );
        if( dirp == NULL ) {
            perror( "Error " );
        } else {
            direntp = readdir( dirp );
        }
	if (ungleich(filename,direntp->d_name))
		findnext();
	if (!direntp)
		findclose();

}


int main(int argc, char *argv[])
{
    unsigned int nhandles;
    FILE *config;
    FILE *alias;
    char configname[255];
    char aka=0,go_on=0,mmdone=0,mmcopy=0;
    char eigen_aka=0;
    char sysopname[255];
    char akastring[255];
    char baud[255];
    char dummy_string[512];
	char pkt_pfad[255];
	unsigned int j;

    max=0;
    if (argc < 2) {
       exit(99);
    }
    taska=getenv("TASK");
    if (!taska) {
       task=99;
    } else {
       task=atoi(taska);
    }

/*    _splitpath(argv[0],drive,path,file,ext);*/
    sprintf(path,"/opt/vireqx/var/");
    sprintf(tempname,"%svireq%u.tmp",path,task);
    unlink(tempname);
    for (i=0;i<510 ; i++)
    {
        found[i].area=-1;
        found[i].mmcd=0;
        found[i].found=0;
    }
    memset(cdtemp,(int)0x00,(size_t)sizeof(cdtemp));

    sprintf(configname,"%svireq.cfg",path);
    sprintf(pwfilename,"%svireq.pwd",path);
    config=fopen(configname,"rt");
    if (!config)
       exit(99);
    while(!feof(config))
    {
        fgets(eingabe,(int)255,config);
        if (feof(config))
           break;
/*	  strlwr(eingabe);*/
        p=strchr(eingabe,0x0d);
        if (p) *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p) *p=0x00;
        if (!strnicmp(eingabe,"LOGFILE",strlen("LOGFILE")))
        {
            strlwr(eingabe);
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            strcpy(dummy_string,p);
            ExpandPath(dummy_string);
            sprintf(logname,dummy_string,task);
        }
        if (!strnicmp(eingabe,"DLC-LOG",strlen("DLC-LOG")))
        {
            strlwr(eingabe);
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            strcpy(dummy_string,p);
            ExpandPath(dummy_string);
            sprintf(dlcname,dummy_string,task);
        }
        if (!strnicmp(eingabe,"CDTEMP",strlen("CDTEMP")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            strcpy(cdtemp,p);
            ExpandPath(cdtemp);
        }
        if (!strnicmp(eingabe,"AKA",strlen("AKA")) && !eigen_aka)
        {
            eigen_aka=1;
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(akastring,p);
            p=akastring;
            zone_from=(unsigned)atoi(p);
            p=strchr(akastring,':');
            p++;
            net_from=(unsigned)atoi(p);
            p=strchr(akastring,'/');
            p++;
            node_from=(unsigned)atoi(p);
            p=strchr(akastring,'.');
            if (p) {
               p++;
               point_from=(unsigned)atoi(p);
            }
        }
        if (!strnicmp(eingabe,"ALIASLIST",strlen("ALIASLIST")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            strcpy(aliaslist[alias_max],p);
            ExpandPath(aliaslist[alias_max++]);
        }
        if (!strnicmp(eingabe,"SYSTEMNAME",strlen("SYSTEMNAME")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p++==0x20);
            p--;
            sprintf(systemname,p);
        }
    }
    fclose(config);
    sprintf(srifname,"%s",argv[1]);
    srif=fopen(srifname,"rt");
    if (!srif) {
       exit(98);
    }
    while (!feof(srif)) {
        fgets(eingabe,(int)255,srif);
        p=strchr(eingabe,0x0d);
        if (p)
           *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p)
           *p=0x00;
        if (!strnicmp(eingabe,"Sysop",strlen("Sysop")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(sysopname,p);
        }
        if (!strnicmp(eingabe,"AKA",strlen("AKA")) && !aka)
        {
            aka=1;
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(akastring,p);
            p=akastring;
            zone_to=(unsigned)atoi(p);
            p=strchr(akastring,':');
            p++;
            net_to=(unsigned)atoi(p);
            p=strchr(akastring,'/');
            p++;
            node_to=(unsigned)atoi(p);
            p=strchr(akastring,'.');
            if (p) {
               p++;
               point_to=(unsigned)atoi(p);
            }
        }
        if (!strnicmp(eingabe,"Baud",strlen("Baud")))
        {
            aka=1;
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(baud,p);
        }
        if (!strnicmp(eingabe,"RequestList",strlen("RequestList")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(requestname,p);
        }
        if (!strnicmp(eingabe,"ResponseList",strlen("ResponseList")))
        {
            p=strchr(eingabe,0x20);
            if (!p)
               break;
            while(*p==0x20)
               p++;
            sprintf(responsename,p);
        }
    } /* endwhile */
    fclose(srif);
    sprintf(dummy_string,"Hi %s,\x0d\x0dhere is the result of your filerequest:\x0d\x0d",sysopname);
    write_temp(dummy_string);
    sprintf(dummy_string,"Requester is %s, %s at %s bps",sysopname,akastring,baud);
    write_log(dummy_string);
    write_log("Wanted Files:");
    write_temp("Wanted Files:");
    if (!strlen(requestname) || !strlen(responsename)) {
       exit(97);
    }
    request=fopen(requestname,"rt");
    if (!request) {
       exit(96);
    }
    while(!feof(request))
    {
        fgets(eingabe,(int)200,request);
        if (feof(request)) {
           break;
        }
        p=strchr(eingabe,0x0d);
        if (p)
           *p=0x00;
        p=strchr(eingabe,0x0a);
        if (p)
           *p=0x00;
        p=strstr(eingabe," !");      // Suche nach Passwort
        if (p)
        {
            p+=2;
            strcpy(found[max].passwort,p);
            p=strchr(found[max].passwort,0x20);   // Leerzeichen am PW-Ende filtern
            if (p)
                *p=0x00;
        }
        else
            found[max].passwort[0]=0x00;
        p=strchr(eingabe,0x20);   // Leerzeichen am Ende filtern
        if (p)
           *p=0x00;
        #ifdef UNIX
	    ;
	#else
	    strlwr(eingabe);
	#endif
        if (strchr(eingabe,'*') || strchr(eingabe,'?'))
        {
            found[max].area=-3;         // Wildcard, area = -3
            found[max].wildcard=1;
        }
        else
            found[max].wildcard=0;
        strcpy(found[max++].name,eingabe);
 
        sprintf(dummy_string,"      %s",eingabe);
        write_log(dummy_string);
        write_temp(dummy_string);
        if (max==500)
        {
           write_log("Maximum number of files for request (500) reached!");
           break;
        }
    }
    fclose(request);
    unlink(requestname);
    if (alias_max)
    {
        for (i=0; i<alias_max; i++)
        {
            alias=fopen(aliaslist[i],"rt");
            if (!alias)
               continue;
            while(!feof(alias))
            {
                fgets(eingabe,(int)255,alias);
                if (feof(alias))
                    break;
                mmnewest=mmexecute=0;
                p=strchr(eingabe,0x0d);
                if (p)
                   *p=0x00;
                p=strchr(eingabe,0x0a);
                if (p)
                   *p=0x00;
                // strlwr(eingabe);
                p=strstr(eingabe," -newest");
                if (p)
                {
                    *p=0x00;
                    mmnewest=1;
                }
                p=strstr(eingabe," -execute");
                if (p)
                {
                    *p=0x00;
                    mmexecute=1;
                }
                p=strchr(eingabe,0x20);
                if (!p)
                   continue;
                *p=0x00;
                j=0;
                while(j < max)
                {
			printf("%s (%lu) -> %s (%lu)\n",eingabe,strlen(eingabe),found[j].name,strlen(found[j].name));
                    if (!strncmp(eingabe,found[j].name,strlen(found[j].name)) && strlen(eingabe)==strlen(found[j].name))
                    {
                        found[j].area=-2;
                        *p=0x20;
                        while(*p==0x20)
                           p++;
                        strcpy(found[j].name,p);
                        if (mmexecute)
                        {
                            found[j].area=-5;
                        } 

                        if (strchr(found[j].name,'*') || strchr(found[j].name,'?'))
                        {
                            aktuelles=0;
                            findfile(found[j].name);
                            if (direntp && !mmnewest)
                                found[j].area=-4;
                            while(direntp)
                            {
                                if (mmnewest)
                                {
                                    sprintf(statname,"%s/%s",dirname,direntp->d_name);
                                    stat(statname,&datstat);
                                    if (datstat.st_mtime > aktuelles)
                                    {
                                        sprintf(found[j].name,"%s/%s",dirname,direntp->d_name);
                                        aktuelles=datstat.st_mtime;
                                    }
                                }
                                else
                                {
                                    found[max].area=-2;
                                    sprintf(found[max++].name,"%s/%s",dirname,direntp->d_name);
                                }
                                findnext();
                            }
                            findclose();
                        }

                    }
                    j++;
                }
            }
            fclose(alias);
        } /* endfor */
    }
    i=0;
    while (i < max) {
        if (found[i].area==-2 || found[i].area==-5)      // ALIAS, bereits gefunden!
        {
            i++;
            continue;
        }
        #ifdef UNIX
	 ;
	#else
	 c=tolower(found[i++].name[0]);
	#endif
        c=(found[i++].name[0]);
        if (!idx[c].open==1) 
        {
            sprintf(idx[c].name,"%svifile%c.idx",path,c);
            idx[c].f=fopen(idx[c].name,"rt");
            if (idx[c].f)
            {
                idx[c].open=1;
                geoeffnet++;
            }
        }
    } /* endwhile */
    i=0;
    while (1) {
        for (j=0;j<250;j++ ) {
           if (idx[j].open==1) {
              fscanf(idx[j].f,"%s %lu",dateiname[j],&area[j]);
              if (feof(idx[j].f)) {
                 fclose(idx[j].f);
                 idx[j].open=0;
                 geoeffnet--;
              }
           }
        }
        if (!geoeffnet) {
           break;
        }
        i=0;
        while (i < max && max < 500)
        {
            if (found[i].area!=-1 && found[i].area!=-3 && found[i].area!=-4)
            {
                i++;
                continue;
            }
            #ifdef UNIX
	     ;
	    #else
	     c=tolower(found[i].name[0]);
	    #endif
            c=(found[i].name[0]);
            if (!idx[c].open)
            {
                i++;
                continue;
            }
            if (found[i].wildcard)
            {
                if (!ungleich(found[i].name,dateiname[c]))
                {
                    found[i].found=1;
                    strcpy(found[max].name,dateiname[c]);
                    strcpy(found[max].passwort,found[i].passwort);
                    found[max++].area=area[c];
                    if (max==500)
                    {
                       write_log("Maximum number of files for request (500) reached!");
                       write_temp("Maximum number of files for request (500) reached!");
                       break;
                    }
                }
            }
            else
            {
                if (!strncmp(dateiname[c],found[i].name,strlen(found[i].name)) && strlen(dateiname[c])==strlen(found[i].name))
                {
                    found[i].area=area[c];
                }
            }
            i++;
        }
    }
    for (i=0;i<250;i++) {
       if (idx[i].open==1) {
          fclose(idx[i].f);
       }
    } /* endfor */


    // Nur auf CD suchen, wenn ein/mehrere Files nicht gefunden wurden!

    for (i=0;i<500 ; i++)
    {
        if ((found[i].area==-1 || found[i].area == -3) && strlen(found[i].name))
        {
            go_on=1;
            break;
        }
    }
    if (go_on)
    {
        geoeffnet=0;
        i=0;
        while (i < max) {
            if (found[i].area==-2)      // ALIAS, bereits gefunden!
            {
                i++;
                continue;
            }
            c=found[i++].name[0];
            if (!idx_c[c].open==1)
            {
                sprintf(idx_c[c].name,"%svifile%c.idc",path,c);
                idx_c[c].f=fopen(idx_c[c].name,"rt");
                if (idx_c[c].f)
                {
                    idx_c[c].open=1;
                    geoeffnet++;
                }
            }
        } /* endwhile */
 
        i=0;
        while (1) {
            for (j=0;j<256;j++ ) {
               if (idx_c[j].open==1) {
                  fscanf(idx_c[j].f,"%s %lu",dateiname[j],&area[j]);
                  if (feof(idx_c[j].f)) {
                     fclose(idx_c[j].f);
                     idx_c[j].open=0;
                     geoeffnet--;
                  }
               }
            }
            if (!geoeffnet) {
               break;
            }
            i=0;
            while (i < max && max < 500)
            {
                if (found[i].area!=-1 && found[i].area!=-3 && found[i].area!=-4)
                {
                    i++;
                    continue;
                }
                c=found[i].name[0];
                if (!idx_c[c].open)
                {
                    i++;
                    continue;
                }
                if (found[i].wildcard)
                {
                    if (!ungleich(found[i].name,dateiname[c]))
                    {
                        strcpy(found[max].name,dateiname[c]);
                        strcpy(found[max].passwort,found[i].passwort);
                        found[i].found=1;
                        found[max].mmcd=1;
                        found[max++].area=area[c];
                        if (max==500)
                        {
                           write_log("Maximum number of files for request (500) reached!");
                           write_temp("Maximum number of files for request (500) reached!");
                           break;
                        }
                    }
                }
                else
                {
                    if (!strncmp(dateiname[c],found[i].name,strlen(found[i].name)) && strlen(dateiname[c])==strlen(found[i].name))
                    {
                        found[i].area=area[c];
                        found[i].mmcd=1;
                    }
                }
                i++;
            }
        }
        for (i=0;i<256;i++) {
           if (idx_c[i].open==1) {
              fclose(idx_c[i].f);
           }
        } /* endfor */
    }

    response=fopen(responsename,"wt");

    sprintf(pfadfile,"%svipath.idx",path);
    pfade=fopen(pfadfile,"rt");
    if (pfade)
    {
        unsigned long zaehler=0;
        while(!feof(pfade))
        {
            fgets(eingabe,(int)200,pfade);
            if (feof(pfade)) {
               break;
            }
            p=strchr(eingabe,0x0d);
            if (p)
               *p=0x00;
            p=strchr(eingabe,0x0a);
            if (p)
               *p=0x00;
            for (i=0;i<max ;i++ )
            {
                if (found[i].area==zaehler && !found[i].mmcd)
                {
                    sprintf(dummy_string,"%s/%s",eingabe,found[i].name);
                    pwok=pwcheck(dummy_string,found[i].passwort);
                    if (pwok)
                    {
                        if (!mmdone)
                        {
                            write_log("Files found:");
                            write_temp("Files found:");
                            mmdone=1;
                        }
                        ExpandPath(eingabe);
                        sprintf(ausgabe,"+%s/%s\n",eingabe,found[i].name);
                        fputs(ausgabe,response);
                        sprintf(dummy_string,"      %s/%s\n",eingabe,found[i].name);
                        write_log(dummy_string);
                        sprintf(dummy_string,"%s/%s",eingabe,found[i].name);
                        write_dlc(dummy_string);
                        sprintf(dummy_string,"      %s",found[i].name);
                        write_temp(dummy_string);
                    }
                    else
                    {
                        found[i].area=-6;
                    }
                }
            }
            zaehler++;
        }
        fclose(pfade);
    }

    sprintf(pfadfile,"%svipath.idc",path);
    pfade=fopen(pfadfile,"rt");
    if (pfade)
    {
        unsigned long zaehler=0;
        while(!feof(pfade))
        {
            mmcopy=0;
            fgets(eingabe,(int)200,pfade);
            if (feof(pfade)) {
               break;
            }
            p=strchr(eingabe,0x0d);
            if (p)
               *p=0x00;
            p=strchr(eingabe,0x0a);
            if (p)
               *p=0x00;
            for (i=0;i<max ;i++ )
            {
                if (found[i].area==zaehler && found[i].mmcd)
                {
                    strupr(eingabe);
                    p=strstr(eingabe," -LOCAL");
                    if (p)
                    {
                        *p++=0x20;
                        *p++=0x20;
                        *p++=0x20;
                        *p++=0x20;
                        *p++=0x20;
                        *p++=0x20;
                        *p++=0x20;
                        if (strlen(cdtemp))
                            mmcopy=1;
                    }
                    p=strchr(eingabe,0x20);   // Leerzeichen am Ende filtern
                    if (p)
                       *p=0x00;
                    ExpandPath(eingabe);
                    sprintf(dummy_string,"%s/%s",eingabe,found[i].name);
                    pwok=pwcheck(dummy_string,found[i].passwort);
                    if (pwok)
                    {
                        if (!mmdone)
                        {
                            write_log("Files found:");
                            write_temp("Files found:");
                            mmdone=1;
                        }
                        if (!mmcopy)
                        {
                            sprintf(ausgabe,"+%s/%s\n",eingabe,found[i].name);
                            fputs(ausgabe,response);
                        }
                        else
                        {
                            if (!copy64k(eingabe,cdtemp,found[i].name))
                            {
                                sprintf(ausgabe,"-%s/%s\n",cdtemp,found[i].name);
                                fputs(ausgabe,response);
                            }
                            else
                            {
                                sprintf(ausgabe,"+%s/%s\n",eingabe,found[i].name);
                                fputs(ausgabe,response);
                            }
                        }    
                        sprintf(dummy_string,"      %s/%s\n",eingabe,found[i].name);
                        write_log(dummy_string);
                        sprintf(dummy_string,"%s/%s",eingabe,found[i].name);
                        write_dlc(dummy_string);
                        sprintf(dummy_string,"      %s",found[i].name);
                        write_temp(dummy_string);
                    }
                    else
                    {
                        found[i].area=-6;
                    }
                }
            }
            zaehler++;
        }
        fclose(pfade);
    }

    // Auswertung der Aliase (areacode = -2)
    for (i=0;i<max ;i++ )
    {
        if (found[i].area==-2) 
        {
            ExpandPath(found[i].name);
            pwok=pwcheck(found[i].name,found[i].passwort);
            if (pwok)
            {
                if (!mmdone)
                {
                    write_log("Files found:");
                    write_temp("Files found:");
                    mmdone=1;
                }
                sprintf(ausgabe,"+%s\n",found[i].name);
                fputs(ausgabe,response);
/*                _splitpath(found[i].name,adrive,apath,afname,aext);*/
                sprintf(dummy_string,"      %s      (Alias)",found[i].name);
                write_log(dummy_string);
                write_dlc(found[i].name);
                sprintf(dummy_string,"      %s",found[i].name);
                write_temp(dummy_string);
            }
            else
            {
                found[i].area=-6;
            }
        }
/*
        if (found[i].area==-5)
        {
            FILE *execute;
            char executename[255];
            sprintf(executename,"%svireq.execute",path);
            execute=fopen(executename,"w+t");
            fputs(sysopname,execute);
            fputs("\n\n\n",execute);
            fclose(execute);
            write_log("Servicerequest:");
            write_temp("Servicerequest:");
            mmdone=0;
            ExpandPath(found[i].name);
            _splitpath(found[i].name,adrive,apath,afname,aext);
            sprintf(ausgabe,"-%s%s%s%d\n",drive,path,"vireqsvc.",task);
            fputs(ausgabe,response);
            sprintf(dummy_string,"      vireqsvc.%d      (Service)",task);
            write_log(dummy_string);
            sprintf(dummy_string,"      vireqsvc.%d",task);
            write_temp(dummy_string);
            system(found[i].name);
        } 
*/
    }

    mmdone=0;
    for (i=0;i<500 ; i++)
    {
        if ((found[i].area==-1 && strlen(found[i].name)) || (found[i].wildcard && !found[i].found))
        {
            if (!mmdone)
            {
                write_log("Files not found:");
                write_temp("Files not found:");
                mmdone=1;
            }
            sprintf(dummy_string,"      %s",found[i].name);
            write_log(dummy_string);
            write_temp(dummy_string);
        }
    }
    mmdone=0;
    for (i=0;i<500 ; i++)
    {
        if (found[i].area==-6)
        {
            sprintf(dummy_string,"Wrong password for %s specified!",found[i].name);
	    write_log(dummy_string);
	    write_temp(dummy_string);
        }
    }
    sprintf(dummy_string,"\x0d\x0dYours sincerely,\x0dSysOp\x0d\x0d");
    write_temp(dummy_string);
    sprintf(pkt_pfad,"%s%s",drive,path);

    sprintf(dummy_string,"Your request at %s",systemname);
    write_pkt(pkt_pfad,
             "VIREQ/Linux",
             sysopname,
            (uint) zone_from,
            (uint) net_from,
            (uint) node_from,
            (uint) point_from,
            (uint) zone_to,
            (uint) net_to,
            (uint) node_to,
            (uint) point_to,
            dummy_string,
            tempname);
    unlink(tempname);
    sprintf(ausgabe,"-%s\n",pfad2);
    fputs(ausgabe,response);
    fclose(response);
}
