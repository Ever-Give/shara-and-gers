#include "windows.h"
#include <stdio.h>
#include <vector>

#pragma hdrstop

//#define PUT_LENGTH
//#define CHECK_LEN
//#define CHECK_DATE
//#define CHECK_DATE_RUS
// #define CHECK_PLAGIAT
//#define CHECK_SPACES
#define CHECK_HASH
//#define CHECK_NORUS
#define CHECK_VANN

const byte utf8sign[3]={0xef,0xbb,0xbf},eoln[]={13,10};
int mycount=0, scount=0, errcount=0;
const char months[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const byte vann[]={0x20,0xd0,0xb2,0xd0,0xb0,0xd0,0xbd,0xd0,0xbd,0};
#define M100T 80000
byte array100[M100T];
byte iss[M100T];

int Put3Symbol(byte *ptr,int cp) //https://github.com/nemtrif/utfcpp/blob/master/source/utf8/checked.h
{
 if(cp<128){ *ptr++=(byte)cp; return 1; }
 else if (cp<0x800){ *ptr++ = ((cp>> 6)|0xc0); *ptr++ = ((cp&0x3f)|0x80); return 2; }
 *ptr++ = ((cp>>12)|0xe0);
 *ptr++ = (((cp>>6)&0x3f)|0x80);
 *ptr++ = ((cp&0x3f)|0x80);
 return 3;
}

int CalcUtf8Len(byte *s)
{
int sum=0;
 for(int i=0;s[i];i++)
 {
  sum++;
  int c1=s[i];
  if ((( c1>> 6) & 3) == 3) { // at least 2 bytes
        int c2 = s[++i];
        if ((c1 >> 5) & 1) i++;  // at least 3 bytes
  }
 }
 return sum;
}

int NextUTF8Char(const char* str, int idx)
{
    // https://en.wikipedia.org/wiki/UTF-8
    int c1 = str[idx++];
    int utf8c;
    if (((c1 >> 6) & 3) == 3)
    { // at least 2 bytes
        int c2 = str[idx++];
        if ((c1 >> 5) & 1)
        {  // at least 3 bytes
            int c3 = str[idx++];
            if ((c1 >> 4) & 1) { // 4 bytes
                int c4 = str[idx++];
                utf8c = ((c4 & 7) << 18) | ((c3 & 0x3f) << 12) |((c2 & 0x3f) << 6) | (c1 & 0x3f);
            } else utf8c = ((c3 & 15) << 12) | ((c2 & 0x3f) << 6) | (c1 & 0x3f);
        } else utf8c = ((c1 & 0x1f) << 6) | (c2 & 0x3f);
    } else utf8c = c1 & 0x7f;
    return utf8c;
}
void Pass1()
{
char path0[]="c:/projects/wetsettxt/s"; int xxxcount=0;
//char path0[]="c:/projects/wetsettxt/patches";
//char path0[]="d:/omo/restored";
//char path0[]="c:/projects/wetsettxt/x/de"; int xxxcount=4100;
//char path0[]="c:/projects/wetsettxt/x/es"; int xxxcount=4200;
//char path0[]="c:/projects/wetsettxt/x/fr"; int xxxcount=4300;
//char path0[]="c:/projects/wetsettxt/x/it"; int xxxcount=4400;
//char path0[]="c:/projects/wetsettxt/x/nl2"; int xxxcount=4500;
//char path0[]="c:/users/1/downloads/new"; int xxxcount=4600;
char fname2[]="errs1.txt";
char path[128],shablon[128],st[4096],sdate[132];
int is8=0,started=0,sum=0,fnum,out=1;
FILE *ff=NULL;
int filecount=0;

  WIN32_FIND_DATA found;
  strcat(strcpy(shablon,path0),"/*.txt");
  HANDLE hFind = FindFirstFile(shablon, &found);

  FILE *f2=fopen(fname2,"w");
  FILE *fx=fopen("xlist.txt","wb");
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,shablon,0,0);
  else do
  {
   int errcount=0,startcount=0,linecount=0;
   filecount++;
//if(filecount>10) break;
   fnum=atoi(found.cFileName);
   sprintf(path,"%s/%s",path0,found.cFileName);
   FILE *f=fopen(path,"rb");
   sprintf(path,"out/%s",found.cFileName);
   int numm=atoi(found.cFileName);
   FILE *f3=NULL;
   if(out) f3=fopen(path,"w");
   if(f)
   {
    if(started){ fprintf(ff,"\r\n@%05d\r\n",fnum); sum+=10; }
    for(;;)
    {
     fgets(st,4096,f);
     if(feof(f)) break;
     strtok(st,"\r\n");
     if(!linecount++)
     {
      if((byte)st[0]==0xef)
      {
       strcpy(st,st+3);
       if(!is8) MessageBox(0,path,"Set utf-8",0);
       is8=1;
      }
      else if(is8){ MessageBox(0,path,"Not utf-8",0); return;}

      strtok(st,"\r\n"); fprintf(fx,"%s",st);

      if(!started++) {
        sprintf(path,"xxx/out%05d.txt",++xxxcount);
        ff=fopen(path,"wb");
        if(ff && is8) fwrite(utf8sign,3,1,ff);
        fprintf(ff,"@%05d\r\n",fnum); sum+=8;
        //fprintf(ff,"@=%05d\r\n",fnum); sum+=9;
      }
     }
//     if(linecount==2) sprintf(st,"\t#%d",numm);

     if(linecount==3) if(strlen(st)>10) {

      //if(!strncmp(st,"Posted by ",10)){ strcpy(shablon, st+10); strcat(strcpy(st,"Author: "),shablon); }
      char *p1=strstr(st," on ");
      if(p1) *p1=0; else p1=st;
      char *p2=strchr(p1+1,',');
      int year=0;
      if(p2){ *p2=0; year=atoi(p2+2); }
      char *pday=strchr(p1+4,' ');
      if(pday) {
//      MessageBox(0,p1,pday,0);
       int month=0;
       for(int m=0;m<12;m++) if(!strncmp(p1+4,months[m],3)){ month=m+1; break; }
       sprintf(sdate,"%d-%02d-%02d%s",year,month,atoi(++pday),p2+6);
       fprintf(fx,"\t%d\t%s\t%s\t%s\n",found.nFileSizeLow,found.cFileName,st+8,sdate);
       sprintf(p1," (%s) #%d",sdate, numm);
      }
     }
     else MessageBox(0,path,st,0);

     if(st[0]==13) continue;
//     if(strchr(st,0xb4))
     int len=strlen(st);
     if(len && st[len-1]==' ') st[--len]=0;

     int tsum=0;
     for(int t=0;t<len;t++) if((byte)st[t]==0x93) tsum++; else if((byte)st[t]==0x94) tsum--;

//     if(st[0]=='"' && st[1]>='A' && st[1]<='Z') st[0]=147;
//     if(tsum && !(tsum&1))//||
	if(len>1000 || st[0]==' ' || (st[0]>='a' && st[0]<='z'))
     {
      errcount++;
//      if(errcount>3)
      {
       if(!startcount++) fprintf(f2,"\n@%d\n",fnum);//path);
       fprintf(f2,"%s\n",st);
      }
      if(out) fprintf(f3,"@@");
     }
     if(len+sum+4>5000) {
      fclose(ff);
      sprintf(path,"xxx/out%05d.txt",++xxxcount);
      ff=fopen(path,"wb");
      if(ff && is8) fwrite(utf8sign,3,1,ff);
//      fprintf(ff,"@%05d\r\n",fnum); sum+=8;
      sum=0;
     }
     if(sum){ fwrite(eoln,2,1,ff); sum+=2;}
     fwrite(st,len,1,ff); sum+=len;
     fwrite(eoln,2,1,ff); sum+=2;
     if(out) fprintf(f3,"%s\n\n",st);
    }
   }
   else fprintf(f2,"\nERROR %s\n",path);
   fclose(f);
   if(out) fclose(f3);
  }
  while(FindNextFile(hFind, &found));
  FindClose(hFind);
  fclose(f2);
  fclose(fx);
  MessageBox(0,fname2,filecount?"Ok":"Empty",0);
}

void Pass2()
{
int filecount=0,errcount=0,previ=0;
char fname[128], st[2048];
FILE *f2=fopen("errs.txt","w");
FILE *ff=NULL;
bool out=1;
int utf=0,maxlen=1000;

  for(int i=1;i<=55;i++)
  {
    if(out && i%100==1) {
     if(ff) fclose(ff);
     sprintf(fname,"def/wet%03d.txt",(i/100));
     ff=fopen(fname,"w");
     if(utf) fwrite(utf8sign,3,1,ff);
    }
    sprintf(fname,"xxx/r/out%04d.txt",i);
    FILE *f=fopen(fname,"r");
    if(!f){ MessageBox(0,fname,0,0); break;}
    filecount++;
    int linecount=0;
    for(;;)
    {
     fgets(st,2047,f);
     if(feof(f)) break;
     if(!linecount++ && (byte)st[0]==0xef) {
      if(!utf++){ fwrite(utf8sign,3,1,ff); maxlen=2000;}
      strcpy(st,st+3);
     }
     if(st[0]=='"') st[0]=0xab;
     if(out) fprintf(ff,"%s",st);
     if(strlen(st)>maxlen || ((byte)st[0]>223 && !utf)) {
      if(previ!=i){ fprintf(f2,"%s\n",fname); previ=i; }
      st[1000]=0;
      fprintf(f2,"%s\n",st);
      errcount++;
     }
    }
    fclose(f);
    if(out) fprintf(ff,"\n");
  }
  if(f2) fclose(f2);
  if(ff) fclose(ff);
  sprintf(st,"%d errors in %d files",errcount,filecount);
  MessageBox(0,st,0,0);
}

void SeparateRus()
{
char fname[128],st[2048];
FILE *f2=NULL;
int num=0,errs=0,filecount=0;

 FILE *ferr=fopen("ferr.txt","w");
 if(ferr) //for(int k=47;k<=47;k++)
 for(int k=1;k<=28;k++)
 {
  sprintf(fname,"defrus\\wet%03d.txt",k);
  FILE *f=fopen(fname,"r");
  if(!f){ MessageBox(0,fname,0,0); break; }
  fprintf(ferr,"#%s#\n",fname);
  for(int line=0;line<100000;line++) {
   fgets(st,2047,f);
   if(feof(f)) break;
   if(strlen(st)>=1000){ fprintf(ferr,"LONG %s",st); errs++; }
   if(line==2 && (byte)st[0]==0xc4 && num) {
    sprintf(fname,"c:/projects/wetsettxt/s/%d.txt",num);
    FILE *ff=fopen(fname,"r");
    if(ff) for(int r=0;r<3;r++) fgets(st,2047,ff);
     else MessageBox(0,fname,0,0);
    fclose(ff);
   }
   if(st[0]=='@') {
    num=atoi(st+1);
    fgets(st,2047,f);
    //sprintf(fname,"out/rus/r%05d.txt",num);
    sprintf(fname,"out/rus/%d.txt",num);
    if(f2) fclose(f2); //CLOSE PREV FILE
    f2=fopen(fname,"w");
    filecount++;
    if(strlen(st)>2) fprintf(ferr,"OTSTUP %s",fname);
   }
   else if(f2) {
    for(int j=0;st[j];j++) if((byte)st[j]==0xf1 && st[j+1]=='@') st[j+1]=0xe0;
    fprintf(f2,"%s",st);
   }
  }
  fclose(f);
 }
 fclose(ferr);
 if(f2) fclose(f2);
 sprintf(st,"fiels =%d, errs=%d",filecount,errs);
 MessageBox(0,st,"done",0);
}

//--------------------------------
class TElem{
public:
 char *s;
 std::vector<unsigned int>ints;
 TElem(char *,unsigned int);
 ~TElem(){ delete[] s;}
 int AddOcc(unsigned int);
};

TElem::TElem(char *str,unsigned int csum)
{
 int len=strlen(str);
 s=new char[len+1];
 strcpy(s,str);
 bool found=0;
 ints.push_back(csum);
}

int TElem::AddOcc(unsigned int csum)
{
 for(int i=i;i<ints.size();i++)
  if(ints[i]==csum) return 1;
 ints.push_back(csum);
 return 0;
}

std::vector<TElem*>vect[32];

int ProcessDir(FILE *f2, char *path0, int level, bool param, int line4)
{
char st[1024],fname[256];
  WIN32_FIND_DATA ffData;
  int sum=0,engcount=0,ruscount=0;

  strcpy(st,path0);
  strcat(st,"*.*");
  HANDLE hFind = FindFirstFile(st, &ffData);
  int filecount=0;
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,st,0,0);
  else do
  {
//   if(filecount++>40) break;
   strcat(strcpy(st,path0),ffData.cFileName);
   if(ffData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
    strcat(st,"/");
    if(ffData.cFileName[0]!='.') {
     int ret=ProcessDir(f2,st,level+1,param,line4);
#ifdef PUT_LENGTH
     if(level==0) fprintf(f2,"%d\t%s\n",ret,ffData.cFileName);
      else sum+=ret;
#endif
    }
   }
   else if(!strcmp(ffData.cFileName,"rus.textile")) {
      ruscount++;
      FILE *frus=fopen(st,"r");
      if(frus) {
	     for(int k=0;k<3;k++) fgets(st,1023,frus);
	     if(strchr(st,'[')) mycount++;
              else {
#ifdef CHECK_DATE_RUS
                 fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"RDATE: %s\n\n",st);
#endif
              }
#ifdef CHECK_VANN
   int vcnt=0;
   for(int l=0;l<1000;l++)
   {
     st[0]=0;
     fgets(st,1023,frus);
     int len=CalcUtf8Len(st);
     if(feof(frus) && !st[0]) break;
     if(strstr(st,(char*)vann)) vcnt++;
   }
   if(vcnt) fprintf(f2,"\n%d\t%s%s\n",vcnt, path0,ffData.cFileName);
#endif
      }
      fclose(frus);
   }
   else
   {
   engcount++;
   sum+=ffData.nFileSizeLow;
   FILE *f=fopen(st,"rb");
   strcpy(fname,st);
   unsigned int csum=0;
   int started=0,lastl=-1,repcount=0,tag=0;
   bool rewrite=0;
   if(f) for(int line=0;line<1000;line++)
   {
     st[0]=0;
     fgets(st,1023,f);
     int len=strlen(st);
     if(feof(f) && !st[0]) break;
#ifdef CHECK_DATE
     if(line==2)
     {
      char *ptr=strchr(st,'(');
      if(!ptr || !(atoi(ptr+1) || ptr[1]=='d')){ //not (date unknown)
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"DATE: %s\n\n",st);
      }
     }
#endif
#ifdef CHECK_HASH
	if(line==2) if(level<3)
        {
         char *ptrH=strchr(st,'#');
         if(!ptrH || *(ptrH-1)==' ') {
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"HASH: %s\n\n",st);
         }
         else if(*(ptrH-1)=='S') scount++;
        }
#endif
//    if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);

#ifdef CHECK_LEN
     if(st[0]>='a' && st[0]<='z'){
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"CAPS: %s\n\n",st);
     }
     if(CalcUtf8Len(st)>1000){
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"LONG: %s\n\n",st);
     }
     if(strstr(st,"\" \"")){
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"QUOT: %s\n\n",st);
     }
#endif

#ifdef CHECK_SPACES
//        if(len && st[len-1]==' ')
	if(line>2 && st[0]=='-' && st[1]!=' ')//strstr(st,"   "))
	{
                 if(!started++) fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
                 fprintf(f2,"SPACES: %s\n\n",st);
        }
#endif

#ifdef CHECK_PLAGIAT
     if(line>line4) {
      int isword=0, lastpos=0;
      char lastsym=0;
      for(int i=0;st[i];i++)
       if(st[i]=='{') tag=1;
       else if(st[i]=='}') tag=0;
       else if(!tag) if(st[i]>='A' && st[i]<='z')
       {
        if(!isword++){ lastsym=st[i]; lastpos=i; }
       }
       else if(isword) {
        isword=0;
        st[i]=0;
        int numm=(lastsym&31);
        char *ptr=st+lastpos;

        int  found=0;
        for(int j=0;j<vect[numm].size();j++)
        {
         TElem *elem=vect[numm][j];
         if(!strcmp(elem->s,ptr))
         {
           if(elem->AddOcc(csum) && repcount++>30)
           {
            if(!started++) {
             if(param) {
              if(f2) fclose(f2);
              sprintf(fname,"plagiat/%s",ffData.cFileName);
              f2=fopen(fname,"w");
             }
             else fprintf(f2,"\n%s%s\n",path0,ffData.cFileName);
            }
            if(line!=lastl){ lastl=line; fprintf(f2,"\n"); }
            fprintf(f2,"%s ",ptr);
           }
           found=1;
           break;
         }
        }//for
        csum=(csum<<4)+numm;
        if(!found) vect[numm].push_back(new TElem(ptr,csum));
        st[i]=' ';
       }//else
     }//line
#endif
   }//readline
   fclose(f);
   }//else
  }
  while(FindNextFile(hFind, &ffData));
  FindClose(hFind);
#ifdef CHECK_NORUS
  if(engcount && !ruscount) fprintf(f2,"\n(NORUS) %s%s\n",path0,ffData.cFileName);
#endif
  return sum;
}

void FindRepeats()
{
char fname2[100]="!reps!.txt",st[1024];
int filecount=0;

  FILE *f2=fopen(fname2,"w");
  fwrite(utf8sign,3,1,f2);
  filecount+=ProcessDir(f2,"c:/users/1/downloads/s/s/",0,false,4);
//  filecount+=ProcessDir(f2,"c:/projects/wetsettxt/patches/",1,false,4);
//  filecount+=ProcessDir(f2,"c:/projects/wetsettxt/s/",1,false,4);
////  filecount+=ProcessDir(f2,"c:/projects/list/ripple/!where/wetsettxt/",1,4);

//  filecount+=ProcessDir(f2,"d:/omo/txt/peestxt/",0,false,8);
//  filecount+=ProcessDir(f2,"d:/omo/txt/peefict/",0,false,4);
//  filecount+=ProcessDir(f2,"c:/users/1/downloads/s/s/",0,false,4);
//-  filecount+=ProcessDir(f2,"d:/omo/txt/omofict0/",0,true,4);
  fclose(f2);
  sprintf(fname2+strlen(fname2)," my%d (%d)",mycount,scount);
  MessageBox(0,fname2,filecount?"Ok":"Empty",0);
}


int ScanOnePage(int pg, char *fname, FILE *f2, FILE *ff, int what, int depth)
{
char topic[128],temp[256];
 if(what<=1) strcpy(topic,"https://peefans.com/topic/");
  else if(what<=3) strcpy(topic,"https://www.omorashi.org/topic/");
   else strcpy(topic,"http://www.peesearch.net/community/forums/showthread/");

 int len40=strlen(topic);
 int errs=0, errc=0;

 FILE *f=fopen(fname,"rb");
 if(!f){ MessageBox(0,fname,0,0); return 1; }
// fprintf(ff,"#%s\n",fname);
 fseek(f,0,2);
 int sz=ftell(f);
 fseek(f,0,0);
 char *buf=new char[sz+1];
 fread(buf,sz,1,f);
 buf[sz]=0;
 char malefemale=' ';
 int lastvote=-1;
 for(int i=0;i<sz;i++)
 {
  if(buf[i]!='<') continue;
  if(buf[i+1]=='h'){ if(buf[i+2]=='4'){ malefemale='-'; lastvote=0; } continue; }
  if(buf[i+1]=='i'){
   if(!memcmp(buf+i,"<i class='fa fa-plus-circle'></i>",33)) {
    lastvote=atoi(buf+i+33+1);
    fprintf(ff,"%d\t%c\t%s\n",lastvote,malefemale,temp);
   }
   if(!memcmp(buf+i,"<i class='fa fa-minus-circle'></i>",34)) {
    lastvote=-atoi(buf+i+34+1);
    fprintf(ff,"%d\t%c\t%s\n",lastvote,malefemale,temp);
   }
   if(!memcmp(buf+i,"<i class='fa fa-circle'></i>",28)) {
    fprintf(ff,"%d\t%c\t%s\n",0,malefemale,temp);
   }
   continue;
  }
  if( buf[i+1]=='a')
  {
  if(!memcmp(buf+i,"<a href=\"https://www.omorashi.org/tags/female/\">",48)) malefemale='f';
  else if(!memcmp(buf+i,"<a href=\"https://www.omorashi.org/tags/male/\">",46)) malefemale='m';
  else if(!memcmp(buf+i+9,topic,len40))// <a href=\"https://peefans.com/topic/");)
  {
   char *pt1=buf+i+len40;
//   char *p2=strchr(temp,'/'); if(p2) *p2=0;
   if(what==4){ while(*pt1!='/') pt1++; pt1++; }  //for peesearch
   strncpy(temp,buf+i+9+len40,255);
   temp[255]=0;

   int num=atoi(temp);
   if(num>0 && num<M100T)// && !array100[num])
   {

    int subpages=1;
    char *ptr2=strchr(temp,'/');
    if(ptr2) {
     //*ptr2++=0;
//if(pg==1 && errc++<40) MessageBox(0,buf+i,ptr2,0);

     if(!strncmp(ptr2,"/page/",6)) {
      int nn=atoi(ptr2+6);
      if(nn>subpages) subpages=nn;  //fprintf(f2,"#%s\n",ptr2); }
     }
     if(!array100[num] && subpages>=depth)
     {
      array100[num]=1;
    // if(!strchr(buf+i,'%'))
     //  for(int k=array100[num];k<maxpages;k++)
      //  if(k) fprintf(f2,"%s/page/%d\n",buf+i+9,k+1); else
       if(!depth) {
        *ptr2=0;
        fprintf(f2,"%s%s\n",topic,temp);
       }
       else {
        char *px=strchr(ptr2+6,'/');
        *ptr2=0;
        fprintf(f2,"%s%s/page/%d\n",topic,temp,depth);
       }
     }
//     *--ptr2='/';
    }
    //array100[num]=maxpages;
   }
   char *p0=strchr(temp,'/'); if(p0) *p0=0;
  }
  }
 }
 delete[] buf;
// fprintf(f2,"%s\n",fname);
 fclose(f);
 return 0;
}

void ScanDir(char *path0, int depth)
{
char shablon[128];
int sum=0;

  WIN32_FIND_DATA find1, find2;
  strcat(strcpy(shablon,path0),"/*.*");
  HANDLE h1 = FindFirstFile(shablon, &find1);
  if (h1== INVALID_HANDLE_VALUE) MessageBox(0,shablon,0,0);
  else do
  {
   if(find1.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
    sprintf(shablon,"%s/%s/*.htm",path0,find1.cFileName);
    HANDLE h2 = FindFirstFile(shablon, &find2);
    if (h1!= INVALID_HANDLE_VALUE) do {
     int nn=atoi(find2.cFileName);
     if(nn>0 && nn<M100T) {
      if(depth) {
       char *ptr=strchr(find2.cFileName,'#');
       if(ptr && atoi(ptr+1)==depth){ array100[nn]=1; sum++; }
      }
      else { array100[nn]++; sum++; }
     }
    }
    while(FindNextFile(h2, &find2));
    FindClose(h2);
   }
  }
  while(FindNextFile(h1, &find1));
  FindClose(h1);

//   sprintf(shablon,"found%d",sum);
//   MessageBox(0,shablon,"found:",0);
array100[70789]=
array100[72566]=
array100[71396]=
array100[71949]=
array100[53338]=
array100[10760]=
array100[10075]=1;

}

void ScanRefsList()
{
char ch='x',path0[128],fname[128];
int stopi,what=2;
int depth=2;//HERE

 memset(array100,0,M100T);
 if(what==1) { // peefict
  strcpy(path0,"d:/omo/l_peefict"); stopi=55;
  ScanDir("d:/omo/p_peefict",depth);
  ch='f';
 }
 if(what==2) { //omo-real
  strcpy(path0,"d:/omo/l_omoreal"); stopi=371; ch='r';
  ScanDir("d:/omo/p_omoreal",depth);
 }
 else if(what==3) { //omo-fict
  strcpy(path0,"d:/omo/l_omofict"); stopi=256; ch='p';//256
//  strcpy(path0,"d:/omo/l_omofict"); stopi=5; ch='q';
  ScanDir("d:/omo/p_omofict",depth);
 }
 else if(what==4) {
  strcpy(path0,"d:/omo/l_peesrch"); stopi=474;  ch='s';
  ScanDir("d:/omo/p_peesrch",depth);
 }

 FILE *ff=fopen("count_x.txt","w");
 FILE *f2=fopen("f2.txt","w");
 if(f2 && ff) for(int i=1;i<=stopi;i++)
 {
  sprintf(fname,"%s/%c%03d.html",path0,ch,i);
  if(ScanOnePage(i,fname,f2,ff,what,depth)){ MessageBox(0,fname,0,0); break; }
 }
 fclose(f2);
 fclose(ff);
 MessageBox(0,"Done","",0);
}

void GetExtraList()
{
// const char path0[]="d:/omo/omofict1";
 const char path0[]="d:/omo/p_omoreal/omoreal1";
 const char chap[]="<link rel=\"last\"";
 FILE *f2=fopen("rr.txt","w");
 char shablon[128],fname[256];

  WIN32_FIND_DATA FindFileData;
  strcat(strcpy(shablon,path0),"/*.htm");
  HANDLE hFind = FindFirstFile(shablon, &FindFileData);
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,shablon,0,0);
  else do
  {
   sprintf(fname,"%s/%s",path0,FindFileData.cFileName);
   FILE *f=fopen(fname,"r");
   int last=0;
   if(f) for(int i=0;i<50;i++)
   {
    fgets(fname,255,f);
    if(feof(f)) break;
    if(!strncmp(fname,chap,16)){
     char *ptr=strstr(fname,"/page/");
     if(ptr) {
      *ptr=0;
      last=atoi(ptr+6);
      for(int k=2;k<=last;k++) if(k==10) fprintf(f2,"%s/page/%d/\n",fname+23,k);
     }
    }
   }
   if(!last) MessageBox(0,fname,FindFileData.cFileName,0); //return; }
   fclose(f);
  }
  while(FindNextFile(hFind, &FindFileData));
  FindClose(hFind);
 fclose(f2);
 MessageBox(0,"Done","",0);
}

void RestoreNoname()
{
const char path0[]="d:/omo/p_omofict/omofict9/", SDEF[]="/topic"; //int defnum=9;
//const char path0[]="d:/omo/peesrch/1/", SDEF[]="/showthread/"; int defnum=0;
char shablon[128],fname[256];
int filecount=0;

  WIN32_FIND_DATA FindFileData;
  strcat(strcpy(shablon,path0),"*.htm");
  HANDLE hFind = FindFirstFile(shablon, &FindFileData);

  FILE *ferr=fopen("err.txt","w");
  int lenn=strlen(SDEF);
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,shablon,0,0);
  else do
  {
   if(!strchr(FindFileData.cFileName,'#'))
   {
    strcat(strcpy(fname,path0),FindFileData.cFileName);
    FILE *f=fopen(fname,"rb");
    fseek(f,0,2);
    int sz=ftell(f);
    fseek(f,0,0);
    char *buf=new char[sz+1];
    fread(buf,sz,1,f);
    fclose(f);

    char *ptr0=strstr(buf,"<link rel=\"canonical\"");
    if(!ptr0){ MessageBox(0,"ERR",0,0); return; }
    //
    char *ptr=strstr(ptr0,SDEF);//"/topic/");
    if(ptr) {
     char *ptr2=strchr(ptr,'"');
     if(ptr2) {
      *ptr2=0;
      char *ptr1=strchr(ptr+strlen(SDEF),'/');
      char *ptr9=strchr(ptr1,'"');
      if(ptr9) *ptr9=0;

      int defnum=0;
      char *ptr3=strstr(ptr1,"/page/");
      if(ptr3){ *ptr3=0; defnum=atoi(ptr3+6); }

      if(strlen(ptr+1)>100) ptr[102]=0;
      sprintf(fname,"d:/omo/new/%s",ptr1+1);
      if(defnum) sprintf(fname+strlen(fname)-1,"#%02d.htm",defnum);
       else strcpy(fname+strlen(fname)-1,".htm");
      FILE *f2=fopen(fname,"wb");
      if(f2) fwrite(buf,sz,1,f2); else MessageBox(0,fname,0,0); //break;}
      fclose(f2);
      *ptr2='"';
     }
    }
    delete[] buf;
    filecount++;
//    if(filecount>1) break;
   }
  }
  while(FindNextFile(hFind, &FindFileData));
  FindClose(hFind);
  fclose(ferr);
  sprintf(shablon,"%d = %s",filecount,path0);
  MessageBox(0,shablon,filecount?"Ok":"Empty",0);
}

//-

int SimplifyHtm(char *path1, char *path2,char *fname,FILE *ferr,int START12)
{
char temp[16],path[256],lastdate[20]="";
 lastdate[16]=0;

 FILE *f=fopen(strcat(strcpy(path,path1),fname),"rb");
 if(!f){ MessageBox(0,path,0,0); return 1; }
 fseek(f,0,2);
 int sz=ftell(f);
 fseek(f,0,0);
 byte *b1=new byte[sz+1];
 byte *b2=new byte[sz+4];
 fread(b1,sz,1,f);
 fclose(f);


 int sum=0,tag=0,script=0,div=0,divcount=0,blo=0,def=0,tit=0,sel=0,h3=0;
 //if(START12)
 { memcpy(b2,utf8sign,3); sum=3; }
 int starcount=-1;
 byte prev=0;

 int OFFSET=0;
 char* hptr=strchr(fname,'#');
 if(hptr) OFFSET=atoi(++hptr);
 if(OFFSET>1) OFFSET=(OFFSET-1)*20; else OFFSET=0;//0-19,20-39,,,

 for(int i=0;i<sz;i++)
 {
  byte b=b1[i];
  if(b=='-' && b1[i+1]=='>'){ script=0; continue; }
  if(b=='<')// && !script)
  {
   char p=b1[i+1],q=b1[i+2],r=b1[i+3];
   if(p=='i' && q=='f' && r=='r'){
    if(strncmp(b1+i,"<iframe src=\"https://www.google.com/recaptcha",44)) b2[sum++]='$';//youtube
    continue;
   }//iframe
   else {
    tag=1;
    if(p=='!'){
     if(!strncmp(b1+i,"<!-- title -->",14)){ div=def=1; i+=13; }
     else if(!strncmp(b1+i,"<!-- /title -->",15)){ div=def=0; i+=14; }
     else if(!strncmp(b1+i,"<!-- status icon and date -->",29)){
     	 div=def=1; //b2[sum++]=13; b2[sum++]=prev=10;
         i+=28;
	 if(++starcount>0) break;
         //b2[sum++]='*';
     }
     else if(!strncmp(b1+i,"<!-- / status icon and date -->",31)){ div=def=0; i+=30; }
     else if(!strncmp(b1+i,"<!-- user info -->",18)){ div=def=1; b2[sum++]=13; b2[sum++]=prev=10; i+=17; }
     else if(!strncmp(b1+i,"<!-- / user info -->",20)){ div=def=0; i+=19; }
     else if(!strncmp(b1+i,"<!-- message -->",16)){
         div=def=1;b2[sum++]=13;
         b2[sum++]=10;
     	 b2[sum++]=13; b2[sum++]=prev=10;
         i+=15;
     }
     else if(!strncmp(b1+i,"<!-- / message -->",18)){ div=def=0; i+=17; }
      else script=1;
     continue;
    }
    //b2[sum++]='#';
    if(p=='/') {
     if(q=='s' && r=='c') script=0; //script
     else if(q=='h' && r=='3') h3=0; //strong
     else if(q=='a' && r=='>'){ if(def){ b2[sum++]=13; b2[sum++]=prev=10; }} //a
     else if(q=='t' && r=='i') tit=0; //title, time
     else if(q=='d' && r=='i'){ divcount--; if(div>divcount) div=0;}//*/
     else if(q=='b' && r=='l'){ blo=0; if(def) { strcpy(b2+sum,"}}\r\n"); sum+=4; }}
     else if(q=='p' && r=='>'){ if(div && def) { b2[sum++]=13; b2[sum++]=prev=10; }}
    }
    else {
     if((p=='b' && q=='r') || p=='p')
     {
      if(div){ b2[sum++]=13; b2[sum++]=prev=10; }
     }
     else if(p=='s' && q=='c') script=1; //script
     else if(p=='h' && q=='3') h3=1; //h3:author
     else if(p=='l' && q=='i'){//link
      if(!strncmp(b1+i,"<link rel=\"canonical\"",21)) {
       i+=80; while(b1[i+3]!='>') b2[sum++]=b1[i++];
       b2[sum++]=13; b2[sum++]=prev=10;
      }
     }
     else if(p=='t' && q=='i'){//<time datetime='2010-06-22T22:33:23Z'
       if(r=='t') tit=1; //title, time
        else if(r=='m'){ memcpy(lastdate,b1+i+16,16); lastdate[10]=' '; }
     }
     else if(p=='d') //div
     {
      divcount++;
      if(!memcmp(b1+i,"<div data-role='commentContent'",31)
      || !memcmp(b1+i,"<div class=\"ipsPhotoPanel",25)){
       div=divcount;
       if(starcount+OFFSET<START12 || sel) def=1; else def=0;
       if(def){
        int num3=starcount+START12-1;
        if(num3>=0) num3+=OFFSET;
        sprintf(b2+sum,"\r\n*%03d*%s\r\n",num3,lastdate);
        while(b2[sum]) sum++;
       }
       starcount++;
       sel=0;
      }
     }
     else if(p=='b' && q=='l'){ blo=1; if(def){b2[sum++]='{'; b2[sum++]=prev='{';}}
     else if(p=='s' && q=='t') { // strong
      if(!memcmp(b1+i,"<strong class=\"ipsBadge",23)) sel=1;
     }
     else if(p=='i' && q=='m') { // strong
      if(!memcmp(b1+i,"<img class=\"ipsImage",20)){ b2[sum++]='$'; b2[sum++]='$'; }
     }
     else if(p=='a' && q==' ') { // strong
      if(!memcmp(b1+i,"<a href=\"https://www.omorashi.org/uploads/",42)){ b2[sum++]='$'; b2[sum++]='$'; }
     }
    }
    continue;
   }
  }
  if(b=='>'){ tag=0; continue; }
  if(!tag && !script){
   if(b=='&' && b1[i+1]!=' ') {
    if(b1[i+2]=='#'){
     sum+=Put3Symbol(b2,atoi(b1+i+2));
     while(b1[i]!=';') i++; continue;
    }
    else if(!memcmp(b1+i,"&amp;",5)){ b='&'; i+=4;}
    else if(!memcmp(b1+i,"&lt;",4)){ b='<'; i+=3;}
    else if(!memcmp(b1+i,"&gt;",4)){ b='>'; i+=3;}
    else if(!memcmp(b1+i,"&nbsp;",6)||!memcmp(b1+i,"&#160;",6)){ b=' '; i+=5;}
    else if(!memcmp(b1+i,"&quot;",6)){ b='"'; i+=5;}
    else if(!memcmp(b1+i,"&laquo;",7)){ b='<'; i+=6;}
    else if(!memcmp(b1+i,"&raquo;",7)){ b='>'; i+=6;}
    else if(!memcmp(b1+i,"&copy;",6)){ b='c'; i+=6;}
    else if(!memcmp(b1+i,"&#039;",6)){ b='`'; i+=5;}
    else if(!memcmp(b1+i,"&#39;",5)){ b='`'; i+=4;}
    else if(!memcmp(b1+i,"&times;",7)){ b='x'; i+=6;}
    else {
     strncpy(temp,b1+i,15);
     temp[15]=0;
     if(ferr) fprintf(ferr,"%s %s\n",fname,temp);
      else { MessageBox(0,temp,"unrecognized symbol!",0); errcount++; }
    }
   }
   if(b>' ' || prev>' ') {
    if(b<' ') b=' ';
    if((div && def)) b2[sum++]=b;
    else if(starcount<1) if(tit || h3) b2[sum++]=b;
   }
   prev=b;
  }
 }
 delete[] b1;

 if(sum>=3)
 {
  char *pt=strrchr(fname,'.');
  if(pt) {
   strcpy(pt,".txt");
   FILE *f2=fopen(strcat(strcpy(path,path2),fname),"wb");
   if(f2) fwrite(b2,sum,1,f2); else { MessageBox(0,path,0,0); return 1; }
   fclose(f2);
  }
 }
 else MessageBox(0,path,"empty",0);
 delete[] b2;
 if(!ferr) MessageBox(0,fname,"ok",0);
 return 0;
}

int CheckAllLen()
{
char st[4096],path[256],path1[]="d:/omo/txt/omofict-/";
int filecount=0,errcount=0;

  WIN32_FIND_DATA found;
  strcat(strcpy(path,path1),"*.txt");
  HANDLE hFind = FindFirstFile(path,&found);

  FILE *ferr=fopen("errl.txt","w");
  fwrite(utf8sign,3,1,ferr);
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,path,0,0);
  else do
  {
   filecount++;
   FILE *f=fopen(strcat(strcpy(path,path1),found.cFileName),"r");
   int started=0;
   if(f) for(;;) {
    fgets(st,4095,f);
    if(feof(f)) break;
    if(CalcUtf8Len(st)>2000) {
     if(!started++) fprintf(ferr,"\n%s\n\n",found.cFileName);
     st[100]=0;
     fprintf(ferr,"%s\n",st);
     errcount++;
    }
   }
   fclose(f);
  }
  while(FindNextFile(hFind, &found));
  FindClose(hFind);
  fclose(ferr);
  sprintf(path,"%d (%d ERRS)- %s",filecount,errcount,path1);
  MessageBox(0,path,filecount?"Ok":"Empty",0);

}

void SimplifyAllHtm(bool selected)
{
char shablon[256];
//char path1[]="d:/omo/p_peesrch/peesrch00/",path2[]="d:/omo/txt/peestxt1/"; int START12=0;
//char path1[]="d:/omo/p_peesrch/peesrch/",path2[]="d:/omo/txt/peestxt/"; int START12=0;
//char path1[]="d:/omo/p_peesrch/peesrch0/",path2[]="d:/omo/txt/peestxt2/"; int START12=0;
//char path1[]="d:/omo/p_omofict/omofict/",path2[]="d:/omo/txt/omofict-/"; int START12=1;
char path1[]="d:/omo/p_omoreal/dummy/",path2[]="d:/omo/txt/empty/"; int START12=1;
//char path1[]="d:/omo/p_omofict/omofict0/",path2[]="d:/omo/txt/omofict0/"; int START12=1;
//char path1[]="d:/omo/p_omoreal/bladder/",path2[]="d:/omo/txt/omorealb/"; int START12=1;
//char path1[]="d:/omo/p_peefict/",path2[]="d:/omo/txt/peefict/"; int START12=2;
int filecount=0;

  WIN32_FIND_DATA FindFileData;
  strcat(strcpy(shablon,path1),"*.htm");
  HANDLE hFind = FindFirstFile(shablon, &FindFileData);

  FILE *ferr=fopen("err.txt","w");
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,shablon,0,0);
  else do
  {
   filecount++;
   if(selected) {
    int n=atoi(FindFileData.cFileName);
    if(n>0 && n<M100T) if(iss[n]==0) continue;
   }
   if(SimplifyHtm(path1,path2,FindFileData.cFileName,ferr,START12)) break;
   //if(filecount>1) break;
  }
  while(FindNextFile(hFind, &FindFileData));
  FindClose(hFind);
  fclose(ferr);
  sprintf(shablon,"%d - %s",filecount,path2);
  MessageBox(0,shablon,filecount?"Ok":"Empty",0);
}

void PrepareList(int param)
{
char path[256],st[4096],title[256],sdate[64];
const char M12[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
//char path1[]="d:/omo/txt/omofict0/";
char path1[]="d:/omo/txt/peestxt/";
//char path1[]="d:/omo/omoreal/",path2[]="d:/omo/txt/omoreal/";
//char path1[]="d:/omo/peefict/",path2[]="d:/omo/txt/peefict/";
int filecount=0;

  title[255]=0;
  WIN32_FIND_DATA found;
  strcat(strcpy(path,path1),"*.txt");
  HANDLE hFind = FindFirstFile(path, &found);

  FILE *f2=fopen("!list!.txt","w");
  FILE *f3=fopen("!errs!.txt","w");
  fwrite(utf8sign,3,1,f2);
  if (hFind== INVALID_HANDLE_VALUE) MessageBox(0,path,"empty dir",0);
  else do
  {
   filecount++;
   strcat(strcpy(path,path1),found.cFileName);
   FILE *f=fopen(path,"rb");
   fread(st,3,1,f);
   if(strncmp(st,utf8sign,3)){ MessageBox(0,path,"not utf8",0); break; }
   if(param==2) { //peestxt
     for(int i=0;i<5;i++)
     {
      fgets(st,255,f);
      if(strlen(st)<3) fgets(st,255,f);
      strtok(st,"\r\n");
      if(i==2) {
	int day=atoi(st+1),month=0;
	char *ptr=strchr(st,' ');
	for(int c=0;c<12;c++) if(!strncmp(ptr+1,M12[c],3)){ month=c+1; break; }
	if(!month) MessageBox(0,st,0,0);
	ptr=strchr(++ptr,' ');
	fprintf(f2,"%d-%02d-%02d %s\t",atoi(ptr),month,day,ptr+7);
      }
      else fprintf(f2,"%s\t",st);
    }//for
    fprintf(f2,"%s\t%d\n",found.cFileName,found.nFileSizeLow);
   }
   else
   {
    fgets(st,255,f); strtok(st,"\r\n"); strncpy(title,st,255);
    if(title[0]=='=' || title[0]=='-') title[0]='/';//for Excel
    for(int i=0;i<6;i++)
    {
     fgets(st,256,f); strtok(st,"\r\n");
     if(i==2) fprintf(f2,"%s\t%s\t%s\tx\t",found.cFileName,st,title);
    }
    fprintf(f2,"%s\t%d\n",st+5,found.nFileSizeLow);
   }
   //if(filecount>1) break;
   fclose(f);
  }
  while(FindNextFile(hFind, &found));
  FindClose(hFind);
  fclose(f2);
  fclose(f3);
  sprintf(path,"%d - %s",filecount,path1);
  MessageBox(0,path,filecount?"Ok":"Empty",0);
}

void CalcSize()
{
char st[125],prev[128];
int sum=0;
 FILE *f1=fopen("in.txt","r");
 FILE *f2=fopen("out.txt","w");
 prev[0]=0;
 if(f1 && f2) for(;;) {
	fgets(st,127,f1);
        if(feof(f1)) break;
        char *ptr=strchr(st,9);
        if(!ptr) break;
        *ptr++=0;
        if(strcmp(st,prev)) {
         if(*prev) fprintf(f2,"%s\t%d\n",prev,sum);
         strcpy(prev,st);
         sum=0;
//         authots++;
        }
        sum+=atoi(ptr);
 }
 fclose(f1);
 fclose(f2);
 MessageBox(0,"Done","",0);
}

void EnableSelected()
{
char fname[128];
int count=0;

 memset(iss,0,sizeof(iss));
 for(int i=2;i<=3;i++)
 {
  sprintf(fname,"d:/omo/p_omofict/sel%d.txt",i);
  FILE *f=fopen(fname,"r");
  if(f) for(;;) {
   fgets(fname,127,f);
   if(feof(f)) break;
   int n=atoi(fname);
   if(n>0 && n<M100T){ if(!iss[n]) count++; iss[n]=1; }
  }
  else MessageBox(0,fname,0,0);
  fclose(f);
 }
 SimplifyAllHtm(true);
 sprintf(fname,"%d",count);
 MessageBox(0,"DONE SEl",fname,0);
}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR s, int)
{
//Pass1();
//Pass2();
//SeparateRus();
// FindRepeats(); return 1;
//  RestoreNoname(); return 1;
 ///
// ScanRefsList(); return 1;
// GetExtraList();
//
//SimplifyHtm("","","1.htm",NULL,1);
//SimplifyHtm("","","2.htm",NULL,0);
//SimplifyHtm("","","6.htm",NULL,1);
// SimplifyHtm("","","7.htm",NULL,2);
 SimplifyAllHtm(0); return 1;

//PrepareList(1); return 1; //2=peestxt
// CalcSize(); return 1;
 EnableSelected(); return 1;
 CheckAllLen(); return 1;

}
