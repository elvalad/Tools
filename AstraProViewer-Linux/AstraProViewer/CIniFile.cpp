/************************************************************************
  Filename   : CIniFile.C
  Usage: this file make to parse the  ini file , to read and write section
  @huadao
************************************************************************/

/* defines for, or consts and inline functions for C++ */

/* global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
//#include <conio.h>	/* Only needed for the test function */

/* local includes */
#include "CIniFile.h"

/* Global Variables */

struct INIENTRY *Entry = NULL;
struct INIENTRY *CurEntry = NULL;
char Result[255] = {""};
FILE *IniFile;

/* Private functions declarations */
void AddpKey (struct INIENTRY *Entry, cchr * pKey, cchr * Value);
void FreeMem (void *Ptr);
void FreeAllMem (void);
bool FindpKey (cchr * Section, cchr * pKey, EFIND * List);
bool AddSectionAndpKey (cchr * Section, cchr * pKey, cchr * Value);
struct INIENTRY *MakeNewEntry (void);


#ifdef DONT_HAVE_STRUPR
/* DONT_HAVE_STRUPR is set when INI_REMOVE_CR is defined */
void strupr( char *str )
{
        //We dont check the ptr because the original also dont do it.
	while(*str != 0)
	{
 		if ( islower( *str ) )
    		{
	     		*str = toupper( *str );
    		}
    		str++;
	}
}
#endif

/*=========================================================================
  Opens an ini file or creates a new one if the requested file doesnt exists.
  notice: sure to call CloseIniFile to free all mem allocated during operation!
========================================================================*/
bool OpenIniFile (cchr * FileName)
{
	char Str[255];
	char *pStr;
	struct INIENTRY *pEntry;

	FreeAllMem ();

	if (FileName == NULL)
	{
		return FALSE;
	}
	
	if ((IniFile = fopen (FileName, "r")) == NULL)
	{
		return FALSE;
	}

	while( fgets (Str, 255, IniFile) != NULL )
	{
		pStr = strchr (Str, '\n');
		if (pStr != NULL)
		{
			*pStr = 0;
		}
		pEntry = MakeNewEntry ();
		if (pEntry == NULL)
		{
			return FALSE;
		}

#ifdef INI_REMOVE_CR
	      	int Len = strlen(Str);
		if ( Len > 0 )
		{
	        	if ( Str[Len-1] == '\r' )
	        	{
	          		Str[Len-1] = '\0';
			}
	        }
#endif

		pEntry->Text = (char *) malloc (strlen (Str) + 1);
		if (pEntry->Text == NULL)
		{
			FreeAllMem ();
			return FALSE;
		}
		
		strcpy (pEntry->Text, Str);
		pStr = strchr (Str, ';');
		if (pStr != NULL)
		{
			*pStr = 0;
		}			/* Cut all comments */
		if ((strstr (Str, "[") > 0) && (strstr (Str, "]") > 0))	/* Is Section */
		{
			pEntry->Type = tpSECTION;
		}
		else
		{
			if (strstr (Str, "=") > 0)
			{
				pEntry->Type = tpKEYVALUE;
			}
			else
			{
				pEntry->Type = tpCOMMENT;
			}
		}

		CurEntry = pEntry;

	}

	fclose (IniFile);
	IniFile = NULL;
	return TRUE;
}

/*=========================================================================
   Frees the memory and closes the ini file without any  modifications.
========================================================================*/
void CloseIniFile (void)
{
	FreeAllMem ();
	if (IniFile != NULL)
	{
		fclose (IniFile);
		IniFile = NULL;
	}
}

/*=========================================================================
   Writes the iniFile to the disk and close it. Frees all memory allocated by WriteIniFile;
========================================================================*/
bool WriteIniFile (const char *FileName)
{
	struct INIENTRY *pEntry = Entry;
	IniFile = NULL;
	if (IniFile != NULL)
	{
		fclose (IniFile);
	}
	
	if ((IniFile = fopen (FileName, "wb")) == NULL)
	{
		FreeAllMem ();
		return FALSE;
	}

	while (pEntry != NULL)
	{
		if (pEntry->Type != tpNULL)
		{

#ifdef INI_REMOVE_CR
			fprintf (IniFile, "%s\n", pEntry->Text);
#else
			fprintf (IniFile, "%s\r\n", pEntry->Text);
#endif
			pEntry = pEntry->pNext;
		}
   	}

	fclose (IniFile);
	IniFile = NULL;
	return TRUE;
}


/*=========================================================================
   Writes a string to the ini file
*========================================================================*/
void WriteString (cchr * Section, cchr * pKey, cchr * Value)
{
	EFIND List;
	char Str[255];

	if (PtrValid (Section, pKey, Value) == FALSE)
	{
		return;
	}
	if (FindpKey (Section, pKey, &List) == TRUE)
	{
		sprintf (Str, "%s=%s%s", List.KeyText, Value, List.Comment);
		FreeMem (List.pKey->Text);
		List.pKey->Text = (char *) malloc (strlen (Str) + 1);
		strcpy (List.pKey->Text, Str);
	}
	else
	{
		if ((List.pSec != NULL) && (List.pKey == NULL))	/* section exist, pKey not */
		{
			AddpKey (List.pSec, pKey, Value);
		}
		else
		{
			AddSectionAndpKey (Section, pKey, Value);
		}
	}
}

/*=========================================================================
   Writes a boolean to the ini file
*========================================================================*/
void WriteBool (cchr * Section, cchr * pKey, bool Value)
{
	char Val[2] = {'0', 0};
	if (Value != 0)
	{
		Val[0] = '1';
	}
	WriteString (Section, pKey, Val);
}

/*=========================================================================
   Writes an integer to the ini file
*========================================================================*/
void WriteInt (cchr * Section, cchr * pKey, int Value)
{
	char Val[12];			/* 32bit maximum + sign + \0 */
	sprintf (Val, "%d", Value);
	WriteString (Section, pKey, Val);
}

/*=========================================================================
   Writes a double to the ini file
*========================================================================*/
void WriteDouble (cchr * Section, cchr * pKey, double Value)
{
	char Val[32];			/* DDDDDDDDDDDDDDD+E308\0 */
	sprintf (Val, "%1.10lE", Value);
	WriteString (Section, pKey, Val);
}


/*=========================================================================
   Reads a string from the ini file
*========================================================================*/
const char * ReadString (cchr * Section, cchr * pKey, cchr * Default)
{
	EFIND List;
	if (PtrValid (Section, pKey, Default) == FALSE)
	{
		return Default;
	}
	if (FindpKey (Section, pKey, &List) == TRUE)
	{
		strcpy (Result, List.ValText);
		return Result;
	}
	return Default;
}

/*=========================================================================
   ReadBool : Reads a boolean from the ini file
*========================================================================*/
bool ReadBool (cchr * Section, cchr * pKey, bool Default)
{
	char Val[2] = {"0"};
	if (Default != 0)
	{
		Val[0] = '1';
	}
	return (atoi (ReadString (Section, pKey, Val)) ? 1 : 0);	/* Only 0 or 1 allowed */
}

/*=========================================================================
   Reads a integer from the ini file
*========================================================================*/
int ReadInt (cchr * Section, cchr * pKey, int Default)
{
	char Val[12];
	sprintf (Val, "%d", Default);
	return (atoi (ReadString (Section, pKey, Val)));
}

/*=========================================================================
   Reads a double from the ini file
*========================================================================*/
double ReadDouble (cchr * Section, cchr * pKey, double Default)
{
	double Val;
	sprintf (Result, "%1.10lE", Default);
	sscanf (ReadString (Section, pKey, Result), "%lE", &Val);
	return Val;
}

/*=========================================================================
   Deletes a pKey from the ini file.
*========================================================================*/
bool DeleteKey (cchr *Section, cchr *pKey)
{
    EFIND         List;
    struct INIENTRY *pPrev;
    struct INIENTRY *pNext;

    if (FindpKey (Section, pKey, &List) == TRUE)
    {
        pPrev = List.pKey->pPrev;
        pNext = List.pKey->pNext;
        if (pPrev)
        {
            pPrev->pNext=pNext;
        }
        if (pNext)
        {
            pNext->pPrev=pPrev;
        }
        FreeMem (List.pKey->Text);
        FreeMem (List.pKey);
        return TRUE;
    }
    return FALSE;
}


/*=========================================================================
   Frees a pointer. It is set to NULL by Free AllMem
*========================================================================*/
void FreeMem (void *Ptr)
{
	if (Ptr != NULL)
	{
		free (Ptr);
	}
}

/*=========================================================================
   Frees all allocated memory and set the pointer to NULL. Thats IMO one of the most important issues relating
             	to pointers :
*========================================================================*/
void FreeAllMem (void)
{
	struct INIENTRY *pEntry;
	struct INIENTRY *pNextEntry;
	pEntry = Entry;
	while (1)
	{
		if (pEntry == NULL)
		{
			break;
		}
		pNextEntry = pEntry->pNext;
		FreeMem (pEntry->Text);	/* Frees the pointer if not NULL */
		FreeMem (pEntry);
		pEntry = pNextEntry;
	}
	Entry = NULL;
	CurEntry = NULL;
}

/*=========================================================================
   Searches the chained list for a section. The section  must be given without the brackets!
*========================================================================*/
struct INIENTRY * FindSection (cchr * Section)
{
	char Sec[130];
	char iSec[130];
	struct INIENTRY *pEntry;
	sprintf (Sec, "[%s]", Section);
	//strupr (Sec);
	pEntry = Entry;		/* Get a pointer to the first Entry */
	while (pEntry != NULL)
	{
		if (pEntry->Type == tpSECTION)
		{
			strcpy (iSec, pEntry->Text);
			//strupr (iSec);
			if (strcmp (Sec, iSec) == 0)
			{
				return pEntry;
			}
		}
		pEntry = pEntry->pNext;
	}
	
	return NULL;
}

/*=========================================================================
   Searches the chained list for a pKey under a given section
*========================================================================*/
bool FindpKey (cchr * Section, cchr * pKey, EFIND * List)
{
	char Search[130];
	char Found[130];
	char Text[255];
	char *pText;
	struct INIENTRY *pEntry;
	List->pSec = NULL;
	List->pKey = NULL;
	pEntry = FindSection (Section);
	if (pEntry == NULL)
	{
		return FALSE;
	}
	
	List->pSec = pEntry;
	List->KeyText[0] = 0;
	List->ValText[0] = 0;
	List->Comment[0] = 0;
	pEntry = pEntry->pNext;
	if (pEntry == NULL)
	{
		return FALSE;
	}
	
  	sprintf (Search, "%s", pKey);
  	//strupr (Search);
  	while (pEntry != NULL)
	{
      		if ((pEntry->Type == tpSECTION) ||	/* Stop after next section or EOF */
	  		(pEntry->Type == tpNULL))
		{
	  		return FALSE;
		}
			
      		if (pEntry->Type == tpKEYVALUE)
		{
			strcpy (Text, pEntry->Text);
			pText = strchr (Text, ';');
			if (pText != NULL)
			{
				strcpy (List->Comment, Text);
				*pText = 0;
			}
			pText = strchr (Text, '=');
	  		if (pText != NULL)
			{
				*pText = 0;
				strcpy (List->KeyText, Text);
				strcpy (Found, Text);
				*pText = '=';
				//strupr (Found);
				/* printf ("%s,%s\n", Search, Found); */
				if (strcmp (Found, Search) == 0)
				{
					strcpy (List->ValText, pText + 1);
					List->pKey = pEntry;
					return TRUE;
				}
			}
		}

		pEntry = pEntry->pNext;
	}
	
  	return FALSE;
}

/*=========================================================================
   Adds an item (pKey or section) to the chaines list
*========================================================================*/
bool AddItem (char Type, cchr * Text)
{
	struct INIENTRY *pEntry = MakeNewEntry ();
	if (pEntry == NULL)
	{
		return FALSE;
	}
	
	pEntry->Type = Type;
	pEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pEntry->Text == NULL)
	{
		free (pEntry);
		return FALSE;
	}
	
	strcpy (pEntry->Text, Text);
	pEntry->pNext = NULL;
	if (CurEntry != NULL)
	{
		CurEntry->pNext = pEntry;
	}
	
	CurEntry = pEntry;
	return TRUE;
}

/*=========================================================================
   Adds an item at a selected position. This means, that the chained list will be broken at the selected position and
   that the new item will be Inserted.
*========================================================================*/
bool AddItemAt (struct INIENTRY * EntryAt, char Mode, cchr * Text)
{
	struct INIENTRY *pNewEntry;
	if (EntryAt == NULL)
	{
		return FALSE;
	}
	
	pNewEntry = (struct INIENTRY *) malloc (sizeof (INIENTRY));
	if (pNewEntry == NULL)
	{
		return FALSE;
	}
	
	pNewEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pNewEntry->Text == NULL)
	{
		free (pNewEntry);
		return FALSE;
	}
	
	strcpy (pNewEntry->Text, Text);
	if (EntryAt->pNext == NULL)	/* No following nodes. */
	{
		EntryAt->pNext = pNewEntry;
		pNewEntry->pNext = NULL;
	}
	else
	{
		pNewEntry->pNext = EntryAt->pNext;
		EntryAt->pNext = pNewEntry;
	}
	
	pNewEntry->pPrev = EntryAt;
	pNewEntry->Type = Mode;
	return TRUE;
}

/*=========================================================================
   Adds a section and then a pKey to the chained list
*========================================================================*/
bool AddSectionAndpKey (cchr * Section, cchr * pKey, cchr * Value)
{
	char Text[255];
	sprintf (Text, "[%s]", Section);
	if (AddItem (tpSECTION, Text) == FALSE)
	{
		return FALSE;
	}
	
	sprintf (Text, "%s=%s", pKey, Value);
	return AddItem (tpKEYVALUE, Text);
}

/*=========================================================================
   Adds a pKey to the chained list
*========================================================================*/
void AddpKey (struct INIENTRY *SecEntry, cchr * pKey, cchr * Value)
{
	char Text[255];
	sprintf (Text, "%s=%s", pKey, Value);
	AddItemAt (SecEntry, tpKEYVALUE, Text);
}

/*=========================================================================
   Allocates the memory for a new entry. This is only the new empty structure, that must be filled from
   function like AddItem etc.
*==========================================================================*/
struct INIENTRY * MakeNewEntry (void)
{
	struct INIENTRY *pEntry;
	pEntry = (struct INIENTRY *) malloc (sizeof (INIENTRY));
	if (pEntry == NULL)
	{
		FreeAllMem ();
		return NULL;
	}
	
	if (Entry == NULL)
	{
		Entry = pEntry;
	}
	
	pEntry->Type = tpNULL;
	pEntry->pPrev = CurEntry;
	pEntry->pNext = NULL;
	pEntry->Text = NULL;
	if (CurEntry != NULL)
	{
		CurEntry->pNext = pEntry;
	}
	return pEntry;
}



//#define INIFILE_TEST_THIS_FILE

#ifdef INIFILE_TEST_THIS_FILE
#define INIFILE_TEST_READ_AND_WRITE
int main (void)
{
	printf ("Hello World\n");
	OpenIniFile ("Test.Ini");
#ifdef INIFILE_TEST_READ_AND_WRITE
	WriteString  ("Test", "Name", "Value");
	WriteString  ("Test", "Name", "OverWrittenValue");
	WriteString  ("Test", "Port", "COM1");
	WriteString  ("Test", "User", "James Brown jr.");
	WriteString  ("Configuration", "eDriver", "MBM2.VXD");
	WriteString  ("Configuration", "Wrap", "LPT.VXD");
	WriteInt 	 ("IO-Port", "Com", 2);
	WriteBool 	 ("IO-Port", "IsValid", 0);
	WriteDouble  ("TheMoney", "TheMoney", 67892.00241);
	WriteInt     ("Test"    , "ToDelete", 1234);
	WriteIniFile ("Test.Ini");
	printf ("Key ToDelete created. Check ini file. Any key to continue");
	while (!kbhit());
	OpenIniFile  ("Test.Ini");
	DeleteKey    ("Test"	  , "ToDelete");
	WriteIniFile ("Test.Ini");
#endif
	printf ("[Test] Name = %s\n", ReadString ("Test", "Name", "NotFound"));
	printf ("[Test] Port = %s\n", ReadString ("Test", "Port", "NotFound"));
	printf ("[Test] User = %s\n", ReadString ("Test", "User", "NotFound"));
	printf ("[Configuration] eDriver = %s\n", ReadString ("Configuration", "eDriver", "NotFound"));
	printf ("[Configuration] Wrap = %s\n", ReadString ("Configuration", "Wrap", "NotFound"));
	printf ("[IO-Port] Com = %d\n", ReadInt ("IO-Port", "Com", 0));
	printf ("[IO-Port] IsValid = %d\n", ReadBool ("IO-Port", "IsValid", 0));
	printf ("[TheMoney] TheMoney = %1.10lf\n", ReadDouble ("TheMoney", "TheMoney", 111));
	CloseIniFile ();
	return 0;
}
#endif
