#include "stdafx.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include <shellapi.h>

#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED				4
#define MAGENTA			5
#define BROWN			6
#define LIGHTGRAY		7
#define BASECOLOR		7
#define DARKGRAY		8
#define LIGHTBLUE		9
#define LIGHTGREEN		10
#define LIGHTCYAN		11
#define LIGHTRED		12
#define LIGHTMAGENTA	13
#define YELLOW			14
#define WHITE			15

#define COMMENT '|'

#define OK 0
#define ERROR_OPEN_FILE 1
#define ERROR_URL 2

using namespace std;

HANDLE hConsole;

wstring w_convert(string s) { // no me acuerdo la conversion facil, dejar de usar cmd es la solucion
	wstring ws(s.begin(), s.end());
	return ws;
}

bool isEmpty(string linea) { //see url format
	for (int i = 0; linea[i] != '\0'; i++) {
		if (isgraph(linea[i])) return false;
	}
	return true;
}

int colorFB(int foreground,int background) {
	return foreground + background * 16;
}

void consoleColor(int foreground) {
	SetConsoleTextAttribute(hConsole, colorFB(foreground, BLACK));
}

bool isComment(string inicial) {
	return inicial[0] == COMMENT;
}

int openURLs(string path) {
	int retValue = OK;
	path.erase(remove(path.begin(), path.end(), '"'), path.end());

	ifstream archivo;
	archivo.open(path.c_str(), ifstream::in);

	if (!archivo.is_open()) {
		consoleColor(LIGHTRED);
		cout << "<file_error> No se pudo abrir el archivo </file_error>" << endl << endl;
		consoleColor(BASECOLOR);
		return ERROR_OPEN_FILE;
	}

	string url;
	while (getline(archivo, url)) { //@habria que chequear que este instalado chrome/el navegador que elegi
		if (isEmpty(url)) continue;
		if (isComment(url)) continue;
		HINSTANCE codigo_ejecucion = ShellExecute(0, 0, L"chrome.exe", w_convert(url).c_str(), 0, SW_SHOW); //puedo usar ShellExecuteA y usar strings
		//ShellExecute(0, 0,w_convert(url).c_str(),0, 0, SW_SHOW) usa el browser default (supongo)
		consoleColor(BASECOLOR);
		cout << "<url> Abriendo " << url << " </url>" << endl;
		if (((int)codigo_ejecucion) <= 32) {
			retValue = ERROR_URL;
			consoleColor(LIGHTRED);
			cout << "<url_error> Codigo " << (int)codigo_ejecucion << " </url_error>" << endl;
			consoleColor(BASECOLOR);
		}
	}

	archivo.close();
	return retValue;
}

void intro() {
	consoleColor(LIGHTMAGENTA);
	cout << "<URL Opener>" << endl;
	consoleColor(YELLOW);
	cout << endl << "<info> Recibe un archivo (.pag) formado con una url por linea </info>" << endl;
	cout << endl << "<info> Si la linea comienza con | es un comentario </info>" << endl;
	consoleColor(BASECOLOR);
	cout << endl << "<comments> " << endl;
	cout << "Como de momento uso cmd no tengo utf-8 solo ascii" << endl;
	cout << "Ahora puse que se abra con chrome, poner una opcion pa elegir el navegador" << endl;
	cout << "</comments>" << endl << endl;
	consoleColor(LIGHTCYAN);
	cout << ">>";
}

void outro() {
	consoleColor(BASECOLOR);
	cout << endl << "<end> Autodestruccion en 3 </end>";
	Sleep(1000);
	cout << "\b\b\b\b\b\b\b\b";
	cout << "2 </end>";
	Sleep(1000);
	cout << "\b\b\b\b\b\b\b\b";
	cout << "1 </end>";
	Sleep(1000);
}

bool FileAssociation() {
	//TODO(fran): check file extension isnt already taken
	//http://www.cplusplus.com/forum/windows/26987/
	HKEY hkey;

	wstring extension = L".pag";            // file extension
	//TODO(fran): it seems this is too long
	wstring desc = L"Open http pages in your browser";         // file type description

	wstring app = 
		L"C:\\Users\\Brenda-Vero-Frank\\Documents\\Visual Studio 2017\\Projects\\OpenPages\\Debug\\OpenPages.exe %1";
	// Open with OpenPages.exe an pass filename as 1st argument (Probably)
	
	wstring action = L"Open with Pages";

	wstring path = extension + L"\\shell\\" + action + L"\\command\\";

	//wstring testpath = L".pag_auto_file\\shell\\open\\command\\";
	WCHAR received_string[200];
	DWORD size = sizeof(received_string);
	if (RegGetValueW(HKEY_CLASSES_ROOT, path.c_str(), 0, RRF_RT_REG_SZ, 0, received_string, &size) == ERROR_SUCCESS) {
		//TODO(fran): hay otros checks que se pueden hacer como que me quede sin espacio en el array
		if (wcscmp(app.c_str(), received_string) == 0) {
			return 1; //File association is already correctly done, otherwise we continue
		}
	}

	//More info https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regcreatekeyexa
	// 1: Create subkey for extension -> HKEY_CLASSES_ROOT\.pag
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, extension.c_str(), 0, 0, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
	{
		cerr << "Could not create or open a registrty key\n";
		return 0;
	}
	RegSetValueEx(hkey, L"", 0, REG_SZ, (BYTE*)desc.c_str(), sizeof(desc)); // default vlaue is description of file extension
	
	RegCloseKey(hkey);

	// 2: Create Subkeys for action ( "Open with Pages" )
	// HKEY_CLASSES_ROOT\.pag\Shell\\open with Pages\\command
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, path.c_str(), 0, 0, REG_OPTION_NON_VOLATILE
		, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS)
		//TODO(fran): check why this is failing
	{
		cerr << "Could not create or open a registrty key\n";
		return 0;
	}
	RegSetValueEx(hkey, L"", 0, REG_SZ, (BYTE*)app.c_str(), app.length());

	RegCloseKey(hkey);

	return 1;
}

int main(int argc,char* argv[])//argc = 1 cuando no te pasan nada, solo tenes la dir del exe
{
#if 0
	FileAssociation(); //TODO(fran): finish this, read more
#endif
	bool somethingFailed=false;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	_CONSOLE_CURSOR_INFO cursor{90,TRUE};
	SetConsoleCursorInfo(hConsole, &cursor);

	intro();

	int result;
	if (argc == 1) {
		string nombre_archivo;
		getline(cin, nombre_archivo);
		result = openURLs(nombre_archivo);
		if (result != OK) somethingFailed = true;
	}
	else {
		for (int i = 1; i < argc; i++) {
			result = openURLs(argv[i]);
			if (result != OK) somethingFailed = true;
		}
	}

	//Si no hubo ni un OK entonces no imprimir esto, dejarlo en pause
	if (!somethingFailed) {
		outro();
	}
	else {
		system("pause");
	}
    return 0;
}

