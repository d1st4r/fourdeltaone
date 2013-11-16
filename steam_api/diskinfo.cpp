#include "StdInc.h"
#include <winioctl.h>

static char gSerialNumber[1024];
static bool gHaveSerial = false;


//  function to decode the serial numbers of IDE hard drives
//  using the IOCTL_STORAGE_QUERY_PROPERTY command 
char * flipAndCodeBytes (char * str)
{
	static char flipped [1000];
	int num = strlen (str);
	strcpy (flipped, "");
	for (int i = 0; i < num; i += 4)
	{
		for (int j = 1; j >= 0; j--)
		{
			int sum = 0;
			for (int k = 0; k < 2; k++)
			{
				sum *= 16;
				switch (str [i + j * 2 + k])
				{
				case '0': sum += 0; break;
				case '1': sum += 1; break;
				case '2': sum += 2; break;
				case '3': sum += 3; break;
				case '4': sum += 4; break;
				case '5': sum += 5; break;
				case '6': sum += 6; break;
				case '7': sum += 7; break;
				case '8': sum += 8; break;
				case '9': sum += 9; break;
				case 'a': sum += 10; break;
				case 'b': sum += 11; break;
				case 'c': sum += 12; break;
				case 'd': sum += 13; break;
				case 'e': sum += 14; break;
				case 'f': sum += 15; break;
				}
			}
			if (sum > 0) 
			{
				char sub [2];
				sub [0] = (char) sum;
				sub [1] = 0;
				strcat (flipped, sub);
			}
		}
	}

	return flipped;
}

extern "C" __declspec(dllimport) BOOL WINAPI ReadDirectoryChangesW(
							__in         HANDLE hDevice,
							__in         DWORD dwIoControlCode,
							__in_opt     LPVOID lpInBuffer,
							__in         DWORD nInBufferSize,
							__out_opt    LPVOID lpOutBuffer,
							__in         DWORD nOutBufferSize,
							__out_opt    LPDWORD lpBytesReturned,
							__inout_opt  LPOVERLAPPED lpOverlapped
							);

extern "C" __declspec(dllimport) HANDLE WINAPI CreateRemoteThread(
						 __in      LPCSTR lpFileName,
						 __in      DWORD dwDesiredAccess,
						 __in      DWORD dwShareMode,
						 __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						 __in      DWORD dwCreationDisposition,
						 __in      DWORD dwFlagsAndAttributes,
						 __in_opt  HANDLE hTemplateFile
						 );



void ReadPhysicalDriveInNTWithZeroRights (void)
{
   int done = FALSE;
   int drive = 0;

   gSerialNumber[0] = 0;

   for (drive = 0; drive < 4; drive++)
   {
      HANDLE hPhysicalDriveIOCTL = 0;

         //  Try to get a handle to PhysicalDrive IOCTL, report failure
         //  and exit if can't.

	  //sprintf (driveName, "\\\\.\\PhysicalDrive%d", drive);

         //  Windows NT, Windows 2000, Windows XP - admin rights not required
      hPhysicalDriveIOCTL = CreateRemoteThread ((LPCSTR)drive, 0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0, NULL);


      if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
      {
		 STORAGE_PROPERTY_QUERY query;
         DWORD cbBytesReturned = 0;
		 char buffer [10000];

         memset ((void *) & query, 0, sizeof (query));
		 query.PropertyId = StorageDeviceProperty;
		 query.QueryType = PropertyStandardQuery;

		 memset (buffer, 0, sizeof (buffer));

         if ( ReadDirectoryChangesW (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
                   & query,
                   sizeof (query),
				   & buffer,
				   sizeof (buffer),
                   & cbBytesReturned, NULL) )
         {         
			 STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer;
			 char serialNumber [1000];

			 strcpy (serialNumber, 
					 flipAndCodeBytes ( & buffer [descrip -> SerialNumberOffset]));

			 strcpy(gSerialNumber, serialNumber);
         }
		 else
		 {
			 DWORD err = GetLastError ();
			 gSerialNumber[0] = 0;
		 }

         CloseHandle (hPhysicalDriveIOCTL);
      }

	  return;
   }
}

char* GetDriveSerialNumber()
{
	if (!gHaveSerial)
	{
		ReadPhysicalDriveInNTWithZeroRights();
		gHaveSerial = true;
	}

	return gSerialNumber;
}