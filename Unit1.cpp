﻿//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "FileCtrl.hpp"
#include <sqlite3.h>
#include <fstream>

#include "Factory.h"
#include "PassDecorator.h"
#include "CheckSign.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma resource "*.dfm"
TForm1 *Form1;
FileSystem File;
Iterator iterator;
char* db_name = "Databases.db";
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
    VirtualStringTree1->NodeDataSize =sizeof(TreeNodeStruct);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Edit1DblClick(TObject *Sender)
{

	if(RadioGroup1->ItemIndex==0)
	{
	String Dir = "c:\\";
	if(SelectDirectory("Укажите путь к папке","",Dir))
	Edit1->Text = Dir;
	}

	if(RadioGroup1->ItemIndex==1){
	OpenDialog1->Execute();
	Edit1->Text = OpenDialog1->FileName;
	}
	OpenFSButtonClick(Sender);


}
//---------------------------------------------------------------------------



void __fastcall TForm1::OpenFSButtonClick(TObject *Sender)
{

	if(Edit1->Text !="")
	{
		Factory Factory;
		UnicodeString str1="\\\\.\\";
		UnicodeString str2=Edit1->Text;

		if(str2[str2.Length()]=='\\')
		str2[str2.Length()]='\0';
		str2 = StringReplace(str2,"\\","\\\\",TReplaceFlags()<<rfReplaceAll);
		UnicodeString str3=str1+str2;

		wchar_t *fileName=str3.t_str();
		File=Factory.CreateFS(fileName);

        RadioGroup2Click(Sender);
		File.ViewInfo(Label1);
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
	VirtualStringTree1->Clear();
	VirtualStringTree1->BeginUpdate();

	ofstream out(db_name);
	out.close();

	AnsiString str = "CREATE TABLE FILE (ID INT NOT NULL, NAME TEXT NOT NULL);";

	sqlite3 *db;

	if ( sqlite3_open(db_name,&db))
	{
		ShowMessage("Can't open database: " + (String)sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_exec(db, str.c_str(), NULL, 0,NULL);

	if(File.countCluster!=0)
	{
		if(Edit2->Text=="")
			iterator.First();
		else
			iterator.SetPosition(StrToLong(Edit2->Text));

		if(Edit3->Text=="")
			iterator.End();
		else
			iterator.SetDone(StrToLong(Edit3->Text));

		byte *dataBuffer = new byte[File.clusterSize];
		CheckSign check;
		for(;!iterator.IsDone();iterator.Next()){
		 if(iterator.GetCurrent(dataBuffer))
			{
				UnicodeString type=check.CheckSigns(dataBuffer);
			 if(type!="NON")
			 {
				PVirtualNode entryNode = VirtualStringTree1->AddChild(VirtualStringTree1->RootNode);
				TreeNodeStruct *nodeData = (TreeNodeStruct*) VirtualStringTree1->GetNodeData(entryNode);
				nodeData->clusterNum = iterator.GetPosition();
				nodeData->name=type;

				AnsiString sql="INSERT INTO FILE (ID, NAME) VALUES ("+(AnsiString)nodeData->clusterNum+", \""+nodeData->name+"\" );";
                sqlite3_exec(db, sql.c_str(), NULL, 0,NULL);
			 }

			 }
		}


		delete[] dataBuffer;
	}
    sqlite3_close(db);
    VirtualStringTree1->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormDestroy(TObject *Sender)
{
	File.ClosedHandle();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RadioGroup2Click(TObject *Sender)
{
	if(RadioGroup2->ItemIndex==0)
	iterator=Iterator(File);

	if(RadioGroup2->ItemIndex==1)
    iterator=PassDecorator(iterator);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText)

{
  TreeNodeStruct *nodeData = (TreeNodeStruct*) VirtualStringTree1->GetNodeData(Node);

	switch (Column) {
		case 0:
		{
		  CellText=nodeData->clusterNum;
		  break;
		}
	case 1:
		{
		  CellText=nodeData->name;
		  break;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{

	PVirtualNode numstr =  VirtualStringTree1->GetFirstSelected();
	if(numstr==NULL)
	 {
		return;
	 }

	TreeNodeStruct *nodeData = (TreeNodeStruct*) VirtualStringTree1->GetNodeData(numstr);
	AnsiString str="DELETE from FILE where id = "+(AnsiString)nodeData->clusterNum +" ;";
	WorkToDB(str);
    ReloadViewDB();
}
//---------------------------------------------------------------------------
void TForm1::ReloadViewDB()
{
	VirtualStringTree1->Clear();
	VirtualStringTree1->BeginUpdate();

	AnsiString str="Select * from FILE ;";
	sqlite3 *db;
	sqlite3_stmt *pStmt;
	int rc,coln,i,j;

	if ( sqlite3_open(db_name,&db))
	{
		ShowMessage("Can't open database: " + (String)sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	if (sqlite3_prepare(db, str.c_str(), -1, &pStmt, NULL))
	{
		sqlite3_finalize(pStmt);
		sqlite3_close(db);
	}

	while((rc = sqlite3_step(pStmt)) == SQLITE_ROW)
	{
		PVirtualNode entryNode = VirtualStringTree1->AddChild(VirtualStringTree1->RootNode);
		TreeNodeStruct *nodeData = (TreeNodeStruct*) VirtualStringTree1->GetNodeData(entryNode);
		coln = sqlite3_data_count(pStmt);
		for(j=0; j<coln; j++)
		{
			AnsiString otwet;
			otwet.printf("%s",sqlite3_column_text(pStmt, j));
				  switch (j) {
					case 0:
					  {
						  nodeData->clusterNum =otwet.ToInt();;
						  break;
					  }
					case 1:
					  {
						  nodeData->name = UnicodeString(otwet);
						  break;
					  }

				  }

		}

	}

	sqlite3_finalize(pStmt);
	sqlite3_close(db);
	VirtualStringTree1->EndUpdate();
}


void __fastcall TForm1::Button4Click(TObject *Sender)
{
		 AnsiString str="DELETE from FILE;";
		 WorkToDB(str);
		 VirtualStringTree1->Clear();
}
//---------------------------------------------------------------------------
void TForm1::WorkToDB(AnsiString str){

		sqlite3 *db;

		if ( sqlite3_open(db_name,&db))
		{
			ShowMessage("Can't open database: ");
			ShowMessage(sqlite3_errmsg(db));
			sqlite3_close(db);
		}

		if (sqlite3_exec(db, str.c_str(), NULL, 0,NULL))
		{
			sqlite3_close(db);
		}

		sqlite3_close(db);

}
