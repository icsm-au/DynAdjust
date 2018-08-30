/////////////////////////////////////////////
// Roger Fraser

#ifndef     __REGISTRY_PROCEDURES__
#define     __REGISTRY_PROCEDURES__

const char * const preferencesFile = "ICSM\\Nterpolate\\Preferences.dat";
const char * const preferencesFolder = "ICSM\\Nterpolate\\";
const char * const icsmFolder = "ICSM\\";
const char * const noGridSpecified = "no_grid_specified";

typedef struct {
	CString GridFile;
	int GridFileStatus;
	CString GridType;
	CString LogFile;
} T_user;


class CRegProc
{
// Construction
public:
	CRegProc();
	
	CString GetPreferencesFolder();
	bool PreferencesFileExists();
	bool CreatePreferencesFile();
	bool PreferencesFolderExists();
	bool PreferencesSubFolderExists(const char* const);
	bool CreatePreferencesFolder(char*);
	void InitialiseSettings(T_user*);
	bool GetTemporaryFileSettings(T_user*);
	void SaveTemporaryFileSettings(T_user*);

	void SaveTFDword(FILE*, int);
	int GetTFDword(FILE*);
	void SaveTFString(FILE*, char*, int);
	CString GetTFString(FILE*, int);
};

#endif