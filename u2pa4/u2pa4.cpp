////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 头文件
#include <Windows.h>
#include <Shlwapi.h>
#include <regex>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
#pragma comment(linker, "/EXPORT:g2paCreate=_AheadLib_g2paCreate,@1")
#pragma comment(linker, "/EXPORT:g2paCreateWithoutUDC=_AheadLib_g2paCreateWithoutUDC,@2")
#pragma comment(linker, "/EXPORT:g2paDelete=_AheadLib_g2paDelete,@3")
#pragma comment(linker, "/EXPORT:g2paGet=_AheadLib_g2paGet,@4")
#pragma comment(linker, "/EXPORT:g2paGetDefaultLyric=_AheadLib_g2paGetDefaultLyric,@5")
#pragma comment(linker, "/EXPORT:g2paGetDefaultPhoneme=_AheadLib_g2paGetDefaultPhoneme,@6")
#pragma comment(linker, "/EXPORT:g2paGetSyllable=_AheadLib_g2paGetSyllable,@7")
#pragma comment(linker, "/EXPORT:g2paGetVersionString=_AheadLib_g2paGetVersionString,@8")
#pragma comment(linker, "/EXPORT:g2paInit=_AheadLib_g2paInit,@9")
#pragma comment(linker, "/EXPORT:g2paPut=_AheadLib_g2paPut,@10")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//结构体函数
typedef struct g2paPutStruct{  
        char pn[4];  
        char ps[4];  
        wchar_t lyric[100];  
} g2paIn; 

typedef struct g2paGetStruct{
		int cn;
		wchar_t phonme[256];
} g2paOut;
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Route映射用委托
typedef int(*DLL_Void)();
typedef int(*DLL_Hand)(int);
typedef int(*DLL_GetSyllable)(int,int*,int*,int*);
typedef int(*DLL_Put)(int,g2paIn*);
typedef int(*DLL_Get)(int,g2paOut*);
typedef wchar_t* wcstring;
wchar_t* getPhonemeValue(wchar_t* Lyric);
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//全局变量
int GetRound=0; //全局变量：Get的获取轮数
g2paIn* getPhonmeArg=NULL;//全局变量：传入Put的参数

char dllpath[1024];//全局变量：当前DLL名称
char dllbasepath[1024];//全局变量：当前DLL名称
char inibasedir[1024];
char inipath[1024];//全局变量：当前DLL名称
char rotpath[1024];//全局变量：当前DLL名称
char routerpath[1024];//全局变量：当前DLL名称
char Lang[4];
char* dllname;

bool isRightLanguage=false;
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//字典处理函数
wchar_t* char2wchar(char* org)
{
	wchar_t *ret;
	size_t len = strlen(org)+1;
	ret=(wchar_t*)malloc(len*sizeof(wchar_t));
	size_t converted=0;
	mbstowcs_s(&converted,ret,len,org,_TRUNCATE);
	return ret;
}
char* wchar2char(wchar_t* org)
{
	char *ret;
	size_t len = wcslen(org)+1;
	ret=(char*)malloc(len*sizeof(char));
	size_t converted=0;
	wcstombs_s(&converted,ret,len,org,_TRUNCATE);
	return ret;
}
wchar_t* wstring2ptr(wchar_t* org)
{
	wchar_t *ret;
	size_t len = wcslen(org)+1;
	ret=(wchar_t*)malloc(len*sizeof(wchar_t));
	wcsncpy(ret,org,len);
	return ret;
}

wchar_t* ReadDicString(char* SectionName,wchar_t* FindKey,wchar_t* DefaultValue)
{
	wchar_t* sechead=char2wchar(SectionName);
	wchar_t ret[255];
	::GetPrivateProfileStringW(sechead,FindKey,DefaultValue,ret,255,char2wchar(inipath));
	return wstring2ptr(ret);
}

wcstring* ReadDicSection(char* SectionName,int* bcount)
{
	wchar_t* sechead=char2wchar(SectionName);
	wchar_t ret[65536];
	int len=::GetPrivateProfileStringW(sechead,NULL,NULL,ret,65536,char2wchar(inipath));
	wcstring wlist[255];
	if(len>0)
	{
		wchar_t tmp[255];
		int k=0;
		int h=0;
		for(int i=0;i<len;i++)
		{
			if(ret[i]==0)
			{
				tmp[h]=0;
				wlist[k]=(wcstring)malloc(sizeof(wchar_t)*255);
				wcscpy(wlist[k],tmp);
				h=0;
				k++;
			}else
			{
				tmp[h]=ret[i];
				h++;
			}
		}
		wlist[k]=(wcstring)malloc(sizeof(wchar_t)*255);
		wcscpy(wlist[k],tmp);
		*bcount=k;
		return wlist;
	}else{
		*bcount=0;
		return wlist;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//程序初始化入口
int LoadInits(HMODULE hModule)
{
	DisableThreadLibraryCalls(hModule);

		DWORD len = GetModuleFileNameA(hModule, dllpath, sizeof dllpath);
		dllname=PathFindFileName(dllpath);
		strncpy(dllbasepath,dllpath,len-strlen(dllname));
	
		sprintf(inibasedir,"%sPhonemeDictionary\\",dllbasepath);

		CreateDirectory(inibasedir,NULL);
	
		sprintf(inipath,"%s%s",inibasedir,dllname);
		inipath[strlen(inipath)-1]='i';
		inipath[strlen(inipath)-2]='n';
		inipath[strlen(inipath)-3]='i';
	
		isRightLanguage=true;
		strcpy(Lang,wchar2char(ReadDicString("Settings",L"Language",L"   ")));
		if(strcmp(Lang,"   ")==0 || strlen(Lang)==0)
		{
			Lang[0]=dllpath[len-7];
			Lang[1]=dllpath[len-6];
			Lang[2]=dllpath[len-5];
		}

		sprintf(rotpath,"%s%s",dllbasepath,"ywk_cache.dat");
	
		char* OriginRouter=wchar2char(ReadDicString("Settings",L"Router",L"g2pa4_CHS.dll"));
		sprintf(routerpath,"%s%s",inibasedir,OriginRouter);
		if(PathFileExists(routerpath)) return TRUE;

		sprintf(routerpath,"%s%s",dllbasepath,OriginRouter);
		if(strcmp(routerpath,dllpath)!=0)
		{
			if(PathFileExists(routerpath)) return TRUE;
		}

		sprintf(routerpath,"%sg2pa4_%s.dll",inibasedir,Lang);
		if(PathFileExists(routerpath)) return TRUE;
		sprintf(routerpath,"%sg2pa4_%s.dll",dllbasepath,Lang);
		if(strcmp(routerpath,dllpath)!=0)
		{
			if(PathFileExists(routerpath)) return TRUE;
		}

		strcpy(routerpath,"");
		return TRUE;
}
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//主窗口Finder

UINT ReadRotValue(char* SectionName,char* FindKey)
{
	UINT ret=::GetPrivateProfileInt(SectionName,FindKey,0,rotpath);
	return ret;
}
void WriteRotValue(char* SectionName,char* FindKey,UINT Value)
{
	char vstr[255];
	sprintf(vstr,"%ld",Value);
	::WritePrivateProfileString(SectionName,FindKey,vstr,rotpath);
	return;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{ 
   DWORD dwCurProcessId = *((DWORD*)lParam);
   DWORD dwProcessId = 0; 
   GetWindowThreadProcessId(hwnd, &dwProcessId); 

   char szhello[255];  
   GetWindowText(hwnd,szhello,255);

   if(dwProcessId == dwCurProcessId && GetParent(hwnd) == NULL && strlen(szhello)>0)
   { 
      *((HWND *)lParam) = hwnd;
      return FALSE; 
   } 
   return TRUE;
} 
HWND GetMainWindow()
{ 
   DWORD dwCurrentProcessId = GetCurrentProcessId();
   if(!EnumWindows(EnumWindowsProc, (LPARAM)&dwCurrentProcessId))
   {     
		return (HWND)dwCurrentProcessId; 
   } 
   return NULL;
}

const UINT BASEMENUID=100000;
UINT EVENTMAP[255];
void AppendMenuX(HMENU hMenuPop,LPCSTR ItemName,UINT ItemID)
{
	UINT Item=ReadRotValue("MENU","MNUINDEX");
	WriteRotValue("MENU","MNUINDEX",Item+1);
	EVENTMAP[Item]=ItemID;
	UINT KEY=BASEMENUID+Item;
	AppendMenu (hMenuPop,MF_STRING|MFS_ENABLED,(UINT_PTR)KEY,ItemName);//ItemName) ;
}

BOOL Menu_Hook(UINT MenuItemID)
{
	switch(MenuItemID)
	{
		case 202:ShellExecute(NULL,"open","notepad.exe",inipath,NULL,SW_SHOWNORMAL);break;
		case 500:ShellExecute(NULL,"open","http://vocaloidyanwu.sinaapp.com/",NULL,NULL,SW_SHOWNORMAL);break;
	}
	return FALSE;
}
typedef void (*WndProc) (HWND,LONG,WPARAM,LPARAM);
WNDPROC MENUPROC;
void WPROC(HWND hWnd,LONG wMsg,WPARAM wParam,LPARAM lParam)
{
	if(wMsg==WM_COMMAND)
	{
		int IID=((int)wParam)-BASEMENUID;
		UINT MID=EVENTMAP[IID];
		if(Menu_Hook(MID))return;
	}
	CallWindowProc(MENUPROC,hWnd,wMsg,wParam,lParam);
}
void MapMenu(HWND Handle)
{
	MENUPROC = (WNDPROC)GetWindowLong(Handle, GWL_WNDPROC);
	WndProc Proc=WPROC;
	SetWindowLong(Handle, GWL_WNDPROC, (LONG)Proc);
}
char* GetStringResource(int Idx)
{
	LCID lcid =GetUserDefaultUILanguage();	
	switch(Idx)
	{
		case 0:
			if(lcid==0x804)return "扩展功能(&Y)";
			return "Extends(&Y)";
		case 1:
			if(lcid==0x804)return "编辑发音字典(&E)";
			return "Edit Ext-Phonemes Dictionary(&E)";
		case 2:
			if(lcid==0x804)return "%s 字典(%s)";
			return "%s Config(%s)";
		case 500:
			if(lcid==0x804)return "关于言舞PROJECT(&A)";
			return "About YANWU-PROJECT(&A)";
	}
}
void PlusMenu()
{
	UINT CPID=ReadRotValue("VOCALOID_THREAD","PID");
	if(CPID==(UINT)GetCurrentProcessId())
	{
		HWND hWindow=(HWND)ReadRotValue("VOCALOID_THREAD","Handle");
		HMENU hMenu=(HMENU)ReadRotValue("MENU","Handle");
		HMENU hMenuPop=(HMENU)ReadRotValue("MENU","PopMenu");
		HMENU hMenuCHD=(HMENU)ReadRotValue("MENU","G2PAMENU");
		
		char EditItem[255];
		sprintf(EditItem,GetStringResource(2),Lang,dllname);
		AppendMenuX(hMenuCHD,EditItem,202);
		MapMenu(hWindow);
//		AppendMenu (hMenuPop,MF_POPUP,(UINT_PTR)hMenuCHD,GetStringResource(1));
//		AppendMenu (hMenu,MF_POPUP,(UINT_PTR)hMenuPop,GetStringResource(0));
	}else
	{
		HWND MainHandle=GetMainWindow();
		if(MainHandle!=NULL)
		{
			WriteRotValue("VOCALOID_THREAD","PID",(UINT)GetCurrentProcessId());

			WriteRotValue("VOCALOID_THREAD","Handle",(UINT)MainHandle);
			HMENU hMenu=GetMenu(MainHandle);
			WriteRotValue("MENU","Handle",(UINT)hMenu);
			
			HMENU hMenuPop=CreatePopupMenu();
			WriteRotValue("MENU","PopMenu",(UINT)hMenuPop);
			WriteRotValue("MENU","MNUINDEX",0);
						
			HMENU hMenuCHD=CreatePopupMenu();
			WriteRotValue("MENU","G2PAMENU",(UINT)hMenuCHD);
	
			char EditItem[255];
			sprintf(EditItem,GetStringResource(2),Lang,dllname);
			AppendMenuX(hMenuCHD,EditItem,202);
			AppendMenu (hMenuPop,MF_POPUP,(UINT_PTR)hMenuCHD,GetStringResource(1));

			AppendMenu(hMenuPop,MF_SEPARATOR,0,"");
			AppendMenuX(hMenuPop,GetStringResource(500),500);
			AppendMenu (hMenu,MF_POPUP,(UINT_PTR)hMenuPop,GetStringResource(0));
			MapMenu(MainHandle);
		}		
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 入口函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{	
	
	if (dwReason == DLL_PROCESS_ATTACH || dwReason == 1)
	{
		int Ret=LoadInits(hModule);
	
		PlusMenu();
		
		return Ret;
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
extern "C" int __cdecl AheadLib_g2paCreate(int hand)
{	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
extern "C" int __cdecl AheadLib_g2paCreateWithoutUDC()
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数 初始化
extern "C" int __cdecl AheadLib_g2paInit(int hand)
{
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数 
extern "C" int __cdecl AheadLib_g2paDelete(int hand)
{	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数 默认歌词
extern "C" wchar_t* __cdecl AheadLib_g2paGetDefaultLyric()
{
	return ReadDicString("Settings",L"DefaultLyric",L"a");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数 默认发音
extern "C" wchar_t* __cdecl AheadLib_g2paGetDefaultPhoneme()
{
	return getPhonemeValue(ReadDicString("Settings",L"DefaultLyric",L"a"));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数:音节分析
extern "C" int __cdecl AheadLib_g2paGetSyllable(int pt1,char** pt2,char** pt3,char** pt4)
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
extern "C" wchar_t* __cdecl AheadLib_g2paGetVersionString()
{
	wchar_t* vstr=ReadDicString("Settings",L"Version",L"unknown");
	wchar_t c[255];
	wsprintfW(c,L"YanwuProject Re_G2PA 1.5.1 : %s",vstr);
	return wstring2ptr(c);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数 读取和取出音标q //17381-18202
extern "C" int __cdecl AheadLib_g2paPut(int a,g2paIn *b)
{
	int* ci=(int*)b;
	if(ci==0)
	{
		return 0;
	}
	GetRound=0;
	getPhonmeArg=b;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

wchar_t* getRouterPhoneme(wchar_t* Lyric)
{
	wchar_t* ret;

	DLL_Put router_Put;
	DLL_Get router_Get;
	HINSTANCE hdll=LoadLibrary(routerpath);
	if(hdll==NULL)
	{
		FreeLibrary(hdll);
		return L"a";
	}
	router_Put=(DLL_Put)GetProcAddress(hdll,"g2paPut");
	router_Get=(DLL_Get)GetProcAddress(hdll,"g2paGet");
	if(router_Put==NULL || router_Get==NULL)
	{
		FreeLibrary(hdll);
		return L"a";
	}
	
	g2paIn* gIn=(g2paIn*)malloc(sizeof(g2paIn));
	g2paOut* gOut=(g2paOut*)malloc(sizeof(g2paOut));
	strncpy(gIn->pn,"P\n\0\0\0\0",4);
	strncpy_s(gIn->ps,"P\t\0\0\0\0",4);
	wcscpy(gIn->lyric,Lyric);

	router_Put(0,gIn);

	int R=router_Get(0,gOut);
	if(R==0)
	{
		if(gOut->cn==1)
		{
			ret=wstring2ptr(gOut->phonme);
		}else if(gOut->cn>1)
		{
			wchar_t result[255]=L"";
			int maxcn=gOut->cn;
			wchar_t tmp[255];
			int h=0;
			int i=0;
			while(true)
			{
				if(gOut->phonme[i]==0)
				{
					tmp[h]=0;
					wsprintfW(result,L"%s%s",result,tmp);
					h=0;
					maxcn--;
					if(maxcn==0)break;
				}else if(gOut->phonme[i]>0 && gOut->phonme[i]<255)
				{
					tmp[h]=gOut->phonme[i];
					h++;
				}
				++i;
			}
			ret=wstring2ptr(result);
		}
	}
	FreeLibrary(hdll);
	free(gIn);
	return ret;
}

wchar_t* getPhonemeDicValue(wchar_t* Lyric)
{
	if(!isRightLanguage)return L"a";
	wchar_t* ret=ReadDicString("DictionaryMap",Lyric,L"{NONE}");
	if(wcsicmp(ret,L"{NONE}")==0)
	{
		getRouterPhoneme(Lyric);
	}else{
		return ret;
	}
}

void StringReplace(std::wstring &strBase, std::wstring strSrc, std::wstring strDes)
{
	std::wstring::size_type pos = 0;
	std::wstring::size_type srcLen = strSrc.size();
	std::wstring::size_type desLen = strDes.size();
	pos=strBase.find(strSrc, pos); 
	while ((pos != std::wstring::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos=strBase.find(strSrc, (pos+desLen));
	}
}


wchar_t* getPhonemeMapValue(wchar_t* Lyric)
{
	wchar_t* realLyric=ReadDicString("ReplaceLyric",Lyric,Lyric);
	return getPhonemeDicValue(realLyric);
}

wchar_t* getPhonemeValue(wchar_t* Lyric)
{
	int c=0;
	wcstring* mrot=ReadDicSection("RegExMap",&c);
	for(int i=0;i<c;i++)
	{
		wcstring pat=mrot[i];
		wcstring val=ReadDicString("RegExMap",pat,L"{NONE}");
		if(wcsicmp(val,L"{NONE}")!=0)
		{
			const std::tr1::wregex pattern(pat);
			
			std::wstring inputstring=(std::wstring)Lyric;
			std::wsmatch result;
			bool match = std::regex_search(inputstring,result,pattern);
			if(match)
			{
				std::wstring ret=(std::wstring)val;
				for(size_t i=1;i<result.size();++i)
				{
					wchar_t mgroup_f1[255];
					wsprintfW(mgroup_f1,L"{$%d}",i);
					wchar_t mgroup_f2[255];
					wsprintfW(mgroup_f2,L"{#%d}",i);
					wchar_t mgroup_f3[255];

					std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>> brr=result.str(i);
					wchar_t* mgroup_i=(wchar_t*)brr.c_str();
					wchar_t* mgroup_v;
					if(std::regex_search((std::wstring)mgroup_i,result,pattern))
					{
						mgroup_v=getPhonemeMapValue(mgroup_i);
					}else
					{
						mgroup_v=getPhonemeValue(mgroup_i);
					}
					StringReplace(ret,mgroup_f1,mgroup_v);
					StringReplace(ret,mgroup_f2,mgroup_i);
				}
				wchar_t* cret=(wchar_t*)ret.c_str();
				return wstring2ptr(cret);
			}
		}
	}
	return getPhonemeMapValue(Lyric);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
extern "C" int __cdecl AheadLib_g2paGet(int a,g2paOut *b)
{
	g2paOut* gpo=b;
	if(GetRound>=2)
	{
		gpo->cn=0;
		wcscpy(gpo->phonme,L"");
	}else
	{
		gpo->cn=1;
		wcscpy(gpo->phonme,getPhonemeValue(wstring2ptr(getPhonmeArg->lyric)));
		GetRound++;
	}
	return GetRound-1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
