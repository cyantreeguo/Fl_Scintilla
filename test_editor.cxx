// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef __MWERKS__
# define FL_DLL
#endif

#include "Scintilla.h"
#include "fltk/Fl_Scintilla.h"

#include <FL/Fl.H>
#include <FL/x.H> // for fl_open_callback
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/filename.H>
#include <FL/Fl_ComboBox.h>

//using namespace Scintilla;

Fl_Scintilla *editor;
Fl_Button *btn1;
void cb_btn(Fl_Widget *w, void *x)
{
	//if ( btn1->visible() ) btn1->hide();
	//else btn1->show();

	Fl_Native_File_Chooser *fnfc = new Fl_Native_File_Chooser();
	fnfc->type(Fl_Native_File_Chooser::BROWSE_FILE);
	if (fnfc->show()) {
		delete fnfc;
		return;
	}

	const char *fn = fnfc->filename();
	char filename[512];
	strcpy(filename, fn);
	//fl_utf8to_mb(fn, strlen(fn), filename, 512);
	delete fnfc;

	FILE *fp = fl_fopen(filename, "rb");
	if ( fp == NULL ) return;
	unsigned char buf[1024];
	int len;
	int all=0;
	while (1) {
		len = fread(buf, 1, 1024, fp);
		if ( len <= 0 ) break;
		all += len;
	}

	fclose(fp);

	editor->SendEditor(SCI_CLEARALL);
	fp = fl_fopen(filename, "rb");
	if ( fp == NULL ) return;
	unsigned char *b = (unsigned char *)malloc(all);
	len = fread(b, 1, all, fp);
	editor->SendEditor(SCI_APPENDTEXT, len, (sptr_t)b);
	fclose(fp);
	free(b);
}

#define MARGIN_FOLD_INDEX 3

static const char *minus_xpm[] = {
	/* width height num_colors chars_per_pixel */
	"     9     9       16            1",
	/* colors */
	"` c #8c96ac",
	". c #c4d0da",
	"# c #daecf4",
	"a c #ccdeec",
	"b c #eceef4",
	"c c #e0e5eb",
	"d c #a7b7c7",
	"e c #e4ecf0",
	"f c #d0d8e2",
	"g c #b7c5d4",
	"h c #fafdfc",
	"i c #b4becc",
	"j c #d1e6f1",
	"k c #e4f2fb",
	"l c #ecf6fc",
	"m c #d4dfe7",
	/* pixels */
	"hbc.i.cbh",
	"bffeheffb",
	"mfllkllfm",
	"gjkkkkkji",
	"da`````jd",
	"ga#j##jai",
	"f.k##k#.a",
	"#..jkj..#",
	"hemgdgc#h"
};
/* XPM */
static const char *plus_xpm[] = {
	/* width height num_colors chars_per_pixel */
	"     9     9       16            1",
	/* colors */
	"` c #242e44",
	". c #8ea0b5",
	"# c #b7d5e4",
	"a c #dcf2fc",
	"b c #a2b8c8",
	"c c #ccd2dc",
	"d c #b8c6d4",
	"e c #f4f4f4",
	"f c #accadc",
	"g c #798fa4",
	"h c #a4b0c0",
	"i c #cde5f0",
	"j c #bcdeec",
	"k c #ecf1f6",
	"l c #acbccc",
	"m c #fcfefc",
	/* pixels */
	"mech.hcem",
	"eldikille",
	"dlaa`akld",
	".#ii`ii#.",
	"g#`````fg",
	".fjj`jjf.",
	"lbji`ijbd",
	"khb#idlhk",
	"mkd.ghdkm"
};

//
const size_t FUNCSIZE=2;
char* g_szFuncList[FUNCSIZE]= { //函数名
	"file-",
	"MoveWindow("
};
char* g_szFuncDesc[FUNCSIZE]= { //函数信息
	"HWND CreateWindow-"
	"LPCTSTR lpClassName,"
	" LPCTSTR lpWindowName,"
	" DWORD dwStyle, "
	" PVOID lpParam"
	"="
	,
	"BOOL MoveWindow-"
	"HWND hWnd,"
	" int X,"
	" int Y,"
	" int nWidth,"
	" int nHeight,"
	" BOOL bRepaint"
	"="
};

static void cb_editor(Scintilla::SCNotification *scn, void *data)
{
	Scintilla::SCNotification *notify = scn;

	if(notify->nmhdr.code == SCN_MARGINCLICK ) {
		printf("%d\n", notify->nmhdr.idFrom);
		if ( notify->margin == 3) {
			// 确定是页边点击事件
			const int line_number = editor->SendEditor(SCI_LINEFROMPOSITION,notify->position);
			editor->SendEditor(SCI_TOGGLEFOLD, line_number);
		}
	}

	if(notify->nmhdr.code == SCN_CHARADDED) {
		// 函数提示功能
		static const char* pCallTipNextWord = NULL;//下一个高亮位置
		static const char* pCallTipCurDesc = NULL;//当前提示的函数信息
		if(notify->ch == '-') { //如果输入了左括号，显示函数提示
			char word[1000]; //保存当前光标下的单词(函数名)
			Scintilla::TextRange tr;    //用于SCI_GETTEXTRANGE命令
			int pos = editor->SendEditor(SCI_GETCURRENTPOS); //取得当前位置（括号的位置）
			int startpos = editor->SendEditor(SCI_WORDSTARTPOSITION,pos-1);//当前单词起始位置
			int endpos = editor->SendEditor(SCI_WORDENDPOSITION,pos-1);//当前单词终止位置
			tr.chrg.cpMin = startpos;  //设定单词区间，取出单词
			tr.chrg.cpMax = endpos;
			tr.lpstrText = word;
			editor->SendEditor(SCI_GETTEXTRANGE,0, sptr_t(&tr));

			for(size_t i=0; i<FUNCSIZE; i++) { //找找有没有我们认识的函数？
				if(memcmp(g_szFuncList[i],word,strlen(g_szFuncList[i])) == 0) {
					printf("show all\n");
					//找到啦，那么显示提示吧
					pCallTipCurDesc = g_szFuncDesc[i]; //当前提示的函数信息
					editor->SendEditor(SCI_CALLTIPSHOW,pos,sptr_t(pCallTipCurDesc));//显示这个提示
					const char *pStart = strchr(pCallTipCurDesc,'-')+1; //高亮第一个参数
					const char *pEnd = strchr(pStart,',');//参数列表以逗号分隔
					if(pEnd == NULL) pEnd = strchr(pStart,'=');//若是最后一个参数，后面是右括号
					editor->SendEditor(SCI_CALLTIPSETHLT, pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
					pCallTipNextWord = pEnd+1;//指向下一参数位置
					break;
				}
			}
		} else if(notify->ch == '=') { //如果输入右括号，就关闭函数提示
			printf("close\n");
			editor->SendEditor(SCI_CALLTIPCANCEL);
			pCallTipCurDesc = NULL;
			pCallTipNextWord = NULL;
		} else if(notify->ch == ',' && editor->SendEditor(SCI_CALLTIPACTIVE) && pCallTipCurDesc) {
			printf("show param\n");
			//输入的是逗号，高亮下一个参数
			const char *pStart = pCallTipNextWord;
			const char *pEnd = strchr(pStart,',');
			if(pEnd == NULL) pEnd = strchr(pStart,'=');
			if(pEnd == NULL) //没有下一个参数啦，关闭提示
				editor->SendEditor(SCI_CALLTIPCANCEL);
			else {
				printf("show param, %d %d\n", pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
				editor->SendEditor(SCI_CALLTIPSETHLT,pStart-pCallTipCurDesc, pEnd-pCallTipCurDesc);
				pCallTipNextWord = pEnd+1;
			}
		}

		if(notify->ch == '.') {
			char word[1000]; //保存当前光标下的单词
			Scintilla::TextRange tr;    //用于SCI_GETTEXTRANGE命令
			int pos = editor->SendEditor(SCI_GETCURRENTPOS); //取得当前位置
			int startpos = editor->SendEditor(SCI_WORDSTARTPOSITION,pos-1);//当前单词起始位置
			int endpos = editor->SendEditor(SCI_WORDENDPOSITION,pos-1);//当前单词终止位置
			tr.chrg.cpMin = startpos;  //设定单词区间，取出单词
			tr.chrg.cpMax = endpos;
			tr.lpstrText = word;
			editor->SendEditor(SCI_GETTEXTRANGE,0, sptr_t(&tr));
			if(strcmp(word,"file.") == 0) { //输入file.后提示file对象的几个方法
				editor->SendEditor(SCI_REGISTERIMAGE, 2, sptr_t(minus_xpm));
				editor->SendEditor(SCI_REGISTERIMAGE, 5, sptr_t(plus_xpm));
				editor->SendEditor(SCI_AUTOCSHOW,0,
				                   sptr_t(
				                           "close?2 "
				                           "eof?4 "
				                           "goodjhfg "
				                           "open?5 "
				                           "rdbuf1中文 "
				                           "size "
										   "t1 "
										   "t2 "
										   "t3 "
										   "t4 "
										   "t5?5"
				                   ));
			}
		}
	}

	
}
int main(int argc, char **argv)
{
	Fl::get_system_colors();

	Fl_Window *win = new Fl_Double_Window(800, 400, 800, 600, "scintilla for fltk");

	win->color(fl_rgb_color(0, 128, 128));

	win->begin();
	editor = new Fl_Scintilla(60, 50, 700, 500);
	editor->SetNotify(cb_editor, 0);
	{
		//editor->box(FL_FLAT_BOX);
		//*
		editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
		editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);

		char *sss = "file\n";
		editor->SendEditor(SCI_APPENDTEXT, strlen(sss), (sptr_t)sss);

		const char* szKeywords1=
		        "asm auto break case catch class const "
		        "const_cast continue default delete do double "
		        "dynamic_cast else enum explicit extern false "
		        "for friend goto if inline mutable "
		        "namespace new operator private protected public "
		        "register reinterpret_cast return signed "
		        "sizeof static static_cast struct switch template "
		        "this throw true try typedef typeid typename "
		        "union unsigned using virtual volatile while";
		const char* szKeywords2=
		        "bool char float int long short void wchar_t";

		// 设置全局风格
		editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Courier New");
		editor->SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT,10);
		editor->SendEditor(SCI_STYLECLEARALL);

		editor->SendEditor(SCI_CALLTIPUSESTYLE, 0);
		editor->SendEditor(SCI_STYLESETFONT, STYLE_CALLTIP,(sptr_t)"Courier New");
		editor->SendEditor(SCI_STYLESETSIZE, STYLE_CALLTIP,9);

		//C++语法解析
		editor->SendEditor(SCI_SETLEXER, SCLEX_CPP);
		editor->SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);//设置关键字
		editor->SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);//设置关键字

		// 下面设置各种语法元素风格
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);   //关键字
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_WORD2, 0x00800080);   //关键字
		editor->SendEditor(SCI_STYLESETBOLD, SCE_C_WORD2, 1);   //关键字
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3); //字符串
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3); //字符
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x00808080);//预编译开关
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00008000);//块注释
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00008000);//行注释
		editor->SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x00008000);//文档注释（/**开头）

		editor->SendEditor(SCI_SETCARETLINEVISIBLE, 1);
		editor->SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
		editor->SendEditor(SCI_SETMARGINTYPEN,0,SC_MARGIN_SYMBOL);
		editor->SendEditor(SCI_SETMARGINWIDTHN,0, 9);
		editor->SendEditor(SCI_SETMARGINMASKN,0, 0x01);

		// 1号页边，宽度为9，显示1,2号标记(0..0110B)
		editor->SendEditor(SCI_SETMARGINTYPEN,1, SC_MARGIN_SYMBOL);
		editor->SendEditor(SCI_SETMARGINWIDTHN,1, 9);
		editor->SendEditor(SCI_SETMARGINMASKN,1, 0x06);

		// 2号页边，宽度为20，显示行号
		editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
		editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);

		// 设置标记的前景色
		editor->SendEditor(SCI_MARKERSETFORE,0,0x0000ff);//0-红色
		editor->SendEditor(SCI_MARKERSETFORE,1,0x00ff00);//1-绿色
		editor->SendEditor(SCI_MARKERSETFORE,2,0xff0000);//2-蓝色

		editor->SendEditor(SCI_SETPROPERTY,(sptr_t)"fold",(sptr_t)"1");
		editor->SendEditor(SCI_SETMARGINTYPEN, MARGIN_FOLD_INDEX, SC_MARGIN_SYMBOL);//页边类型
		editor->SendEditor(SCI_SETMARGINMASKN, MARGIN_FOLD_INDEX, SC_MASK_FOLDERS); //页边掩码
		editor->SendEditor(SCI_SETMARGINWIDTHN, MARGIN_FOLD_INDEX, 11); //页边宽度
		editor->SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_FOLD_INDEX, true); //响应鼠标消息

		// 折叠标签样式
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_ARROW);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_CIRCLEPLUSCONNECTED);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
		/*
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PIXMAP);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_PIXMAP);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,  SC_MARK_PIXMAP);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_PIXMAP);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
		editor->SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);
		*/

		/*
		editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDER, (sptr_t)plus_xpm);
		editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPEN, (sptr_t)plus_xpm);
		editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEREND, (sptr_t)plus_xpm);
		editor->SendEditor(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPENMID, (sptr_t)minus_xpm);
		*/

		// 折叠标签颜色
		editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
		editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
		editor->SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);
		editor->SendEditor(SCI_SETFOLDFLAGS, 16|4, 0); //如果折叠就在折叠行的上下各画一条横线
		//*/

		editor->SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8);
		//editor->SendEditor(SCI_SETCODEPAGE, 936);
		//editor->SendEditor(SCI_STYLESETCHARACTERSET, SC_CHARSET_GB2312);

		//editor->SendEditor(SCI_SETHSCROLLBAR, false);

		Fl_ComboBox *fc = new Fl_ComboBox(5,5,180,25,"Test", 6);
		fc->innerinput()->value("1234");
		fc->add("onexxxxxxxxxxxxxxxxx");
		fc->add("two");
		fc->add("three");
		fc->add("onexxxxxxxxxxxxxx1xxx");
		fc->add("onexxxxxxxxxxxx1ghx");
		fc->add("onexxxxxxx2xxxxxxxx");
		fc->add("onexxxxx1xxxdffdxxxxx");
		fc->add("汉字onexxxxxxxxxxxxxx");
		fc->add("onexx1xxxxxxxxx");
		fc->add("onexxx2xxxxxx");
		fc->add("onexxxxxx1xxxx2xxxxx");
		fc->add("onexxxxxx5xxxx2xxxxx");
		fc->add("onexxxxxxx6xxx2xxxxx");
		fc->add("end");
	}

	Fl_Button *btn = new Fl_Button(100, 560, 80, 30, "Open");
	btn->callback(cb_btn);

	btn1 = new Fl_Button(0, 560, 80, 30, "1");

	win->end();
	win->resizable(win);
	win->show(argc, argv);

	editor->take_focus();

	/*
	editor->SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT, LPARAM("Courier New"));
	editor->SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);

	editor->SendEditor(SCI_SETMARGINTYPEN,2, SC_MARGIN_NUMBER);
	editor->SendEditor(SCI_SETMARGINWIDTHN,2, 20);

	editor->SendEditor(SCI_APPENDTEXT, 12, (sptr_t)"hello world\n");
	editor->SendEditor(SCI_APPENDTEXT, 12, (sptr_t)"hello world\n");
	editor->SendEditor(SCI_APPENDTEXT, 12, (sptr_t)"hello world\n");
	*/



	return Fl::run();
}
