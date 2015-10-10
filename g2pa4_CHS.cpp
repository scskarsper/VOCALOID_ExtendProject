////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 头文件
#include <Windows.h>
#include <stdio.h>
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
char threechar_lang[4];
char inibasedir[1024];
char inipath[1024];//全局变量：当前DLL名称
char routerpath[1024];//全局变量：当前DLL名称

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 入口函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{	
    DWORD len = GetModuleFileNameA(hModule, dllpath, sizeof dllpath);
	strncpy(dllbasepath,dllpath,len-13);
	threechar_lang[0]=dllpath[len-7];
	threechar_lang[1]=dllpath[len-6];
	threechar_lang[2]=dllpath[len-5];
	
	sprintf(inibasedir,"%sPhonemeDictionary\\",dllbasepath);

	CreateDirectory(inibasedir,NULL);
	
	sprintf(inipath,"%sDictionary_%s.ini",inibasedir,threechar_lang);
	
	wchar_t* Language=ReadDicString("Settings",L"Language",L"   ");
	if(wcsicmp(Language,char2wchar(threechar_lang))==0)
	{
		isRightLanguage=true;
	}

	char* OriginRouter=wchar2char(ReadDicString("Settings",L"Router",L"g2pa4_CHS.dll"));
	sprintf(routerpath,"%s%s",inibasedir,OriginRouter);

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
extern "C" int __cdecl AheadLib_g2paGetSyllable(int pt1,int* pt2,int* pt3,int* pt4)
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

	ret=wstring2ptr(gOut->phonme);
	
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
