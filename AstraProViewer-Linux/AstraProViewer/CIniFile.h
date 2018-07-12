#ifndef CINIFILE_H
#define CINIFILE_H

#ifdef LINUX /* Remove CR, on unix systems. */
#define INI_REMOVE_CR
#define DONT_HAVE_STRUPR
#endif

#ifndef CCHR_H
#define CCHR_H
typedef const char cchr;
#endif

#ifndef __cplusplus
typedef char bool;
#define true  1
#define TRUE  1
#define false 0
#define FALSE 0
#else
#define TRUE 1
#define FALSE 0
#endif

#define tpNULL       0
#define tpSECTION    1
#define tpKEYVALUE   2
#define tpCOMMENT    3


struct INIENTRY
{
   char   Type;
   char  *Text;
   struct INIENTRY *pPrev;
   struct INIENTRY *pNext;
} ;

typedef struct
{
   struct INIENTRY *pSec;
   struct INIENTRY *pKey;
   char          KeyText [128];
   char          ValText [128];
   char          Comment [255];
} EFIND;

/* Macros */
#define PtrValid(Sec,Key,Val) ((Sec!=NULL)&&(Key!=NULL)&&(Val!=NULL))

/* Connectors of this file (Prototypes) */

bool    OpenIniFile (cchr *FileName);

bool    ReadBool    (cchr *Section, cchr *Key, bool   Default);
int     ReadInt     (cchr *Section, cchr *Key, int    Default);
double  ReadDouble  (cchr *Section, cchr *Key, double Default);
cchr   *ReadString  (cchr *Section, cchr *Key, cchr  *Default);

void    WriteBool   (cchr *Section, cchr *Key, bool   Value);
void    WriteInt    (cchr *Section, cchr *Key, int    Value);
void    WriteDouble (cchr *Section, cchr *Key, double Value);
void    WriteString (cchr *Section, cchr *Key, cchr  *Value);

bool	DeleteKey (cchr *Section, cchr *Key);

void    CloseIniFile ();
bool    WriteIniFile (cchr *FileName);

#endif


