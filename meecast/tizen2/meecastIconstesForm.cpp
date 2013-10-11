/* vim: set sw=4 ts=4 et: */
/*
 * This file is part of Meecast for Tizen2
 *
 * Copyright (C) 2013 Vlad Vasilyeu
 *     for the code
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU  General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU  General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
*/
/*******************************************************************************/
#include <FApp.h>
#include <FIo.h>
#include "meecastIconsetsForm.h"

using namespace Tizen::Base;
using namespace Tizen::App;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Ui::Scenes;
using namespace Tizen::Graphics;
using namespace Tizen::Base;
using namespace Tizen::Base::Collection;
using namespace Tizen::Io;

static const int LIST_HEIGHT = 112;
static const int BUTTON_HEIGHT = 74;


meecastIconsetsForm::meecastIconsetsForm(void)
                    : __pListView(null)
                    , __pSearchBar(null){
}

meecastIconsetsForm::~meecastIconsetsForm(void){
	if (__fileList.GetCount() > 0){
		__fileList.RemoveAll(true);
	}
}

bool
meecastIconsetsForm::Initialize(void)
{
    Construct(L"ICONSETS_FORM");
    return true;
}

result
meecastIconsetsForm::OnInitializing(void){
    result r = E_SUCCESS;

    Tizen::Graphics::Rectangle rect = GetClientAreaBounds();
    SetFormBackEventListener(this);

    Header* pHeader = GetHeader();
    pHeader->SetTitleText(_("Select the iconset"));


    // Creates an instance of ListView
    __pListView = static_cast <ListView*> (GetControl(L"IDC_LISTVIEW"));
    __pListView->SetItemProvider(*this);
    __pListView->AddListViewItemEventListener(*this);
    __pListView->AddFastScrollListener(*this);

    // Adds the list view to the form
    AddControl(*__pListView);
	r = GetFilesList();
	TryReturn(r == E_SUCCESS, r, "Could not get iconsets files present in iconsets directory");
 	FloatRectangle clientRect = GetClientAreaBoundsF();
 	__pListView->SetSize(FloatDimension(clientRect.width, clientRect.height));
    return r;
}

result
meecastIconsetsForm::OnTerminating(void){
    result r = E_SUCCESS;

    // TODO:
    // Add your termination code here
    return r;
}



void
meecastIconsetsForm::OnActionPerformed(const Tizen::Ui::Control& source, int actionId){
    SceneManager* pSceneManager = SceneManager::GetInstance();
    AppAssert(pSceneManager);

    AppLog("Check Action");
    switch(actionId)
    {
    default:
        break;
    }
}

void
meecastIconsetsForm::OnFormBackRequested(Tizen::Ui::Controls::Form& source)
{
	SceneManager* pSceneManager = SceneManager::GetInstance();
	AppAssert(pSceneManager);

	pSceneManager->GoBackward(BackwardSceneTransition(SCENE_TRANSITION_ANIMATION_TYPE_RIGHT));

}

void
meecastIconsetsForm::OnSceneActivatedN(const Tizen::Ui::Scenes::SceneId& previousSceneId,
                                          const Tizen::Ui::Scenes::SceneId& currentSceneId, Tizen::Base::Collection::IList* pArgs)
{
    /*
    AppLog("OnSceneActivatedN Regions");
    if (pArgs != null) {
        __db = static_cast < Core::DatabaseSqlite* > (pArgs->GetAt(0));
        __SourceId = *(dynamic_cast<String*> (pArgs->GetAt(1)));
        __CountryName = *(dynamic_cast<String*> (pArgs->GetAt(2)));
        pArgs->RemoveAt(2);
        pArgs->RemoveAt(1);
        AppLog("SOurceID %S", __SourceId.GetPointer());
        delete pArgs;
        LoadList();
    }
    */
}

void
meecastIconsetsForm::OnSceneDeactivated(const Tizen::Ui::Scenes::SceneId& currentSceneId,
                                           const Tizen::Ui::Scenes::SceneId& nextSceneId)
{
    // TODO:
    // Add your scene deactivate code here
    AppLog("OnSceneDeactivated");
}

int
meecastIconsetsForm::GetItemCount(void){
   /* AppLog("Count %i",  __fileList.GetCount()); */
    return __fileList.GetCount();
}

bool
meecastIconsetsForm::DeleteItem(int index, Tizen::Ui::Controls::ListItemBase* pItem, int itemWidth)
{
	delete pItem;
	return true;
}


Tizen::Ui::Controls::ListItemBase*
meecastIconsetsForm::CreateItem (int index, int itemWidth)
{
    CustomItem* pItem = new (std::nothrow) CustomItem();
    TryReturn(pItem != null, null, "Out of memory");

    pItem->Construct(Tizen::Graphics::Dimension(itemWidth, LIST_HEIGHT), LIST_ANNEX_STYLE_NORMAL);

    String* pStr = dynamic_cast< String* >(__fileList.GetAt(index));
    pItem->AddElement(Tizen::Graphics::Rectangle(26, 32, 600, 50), 0, *pStr, false);

    Tizen::Media::Image *image = null;
    Tizen::Graphics::Bitmap* mainIconBitmap = null;

    image = new (std::nothrow) Tizen::Media::Image();
    image->Construct();

    /* AppLog("Iconset %s", config->iconSet().c_str()); */
    mainIconBitmap = image->DecodeN(App::GetInstance()->GetAppResourcePath() + L"screen-density-xhigh/icons/" + *pStr + "/30.png", BITMAP_PIXEL_FORMAT_ARGB8888);
    pItem->AddElement(Rectangle(600, 10, 100, 100), 502, *mainIconBitmap, null, null);
    SAFE_DELETE(image);
    SAFE_DELETE(mainIconBitmap);

    return pItem;
}

void
meecastIconsetsForm::OnListViewItemStateChanged(Tizen::Ui::Controls::ListView& listView, int index, int elementId, Tizen::Ui::Controls::ListItemStatus status)
{
    ConfigTizen *config;
    config = ConfigTizen::Instance( std::string("config.xml"),
                                       Core::AbstractConfig::prefix+
                                       Core::AbstractConfig::schemaPath+
                                       "config.xsd");

	if (status == LIST_ITEM_STATUS_SELECTED || status == LIST_ITEM_STATUS_MORE){
        SceneManager* pSceneManager = SceneManager::GetInstance();
        AppAssert(pSceneManager);

	    AppLog("LIST_ITEM_STATUS_SELECTED ");
        String* pStr = dynamic_cast< String* >(__fileList.GetAt(index));

        std::string temp_string = (const char*) (Tizen::Base::Utility::StringUtil::StringToUtf8N(*pStr)->GetPointer());
        config->iconSet(temp_string);
        config->saveConfig();
        /* Select 'Source location' */
        if (index == 0){
	        AppLog("i111LIST_ITEM_STATUS_SELECTED ");
//            pSceneManager->GoForward(SceneTransitionId(L"ID_SCNT_MAINSCENE"));
        }
	    pSceneManager->GoBackward(BackwardSceneTransition(SCENE_TRANSITION_ANIMATION_TYPE_RIGHT));

	}
    config->DeleteInstance();
}

void
meecastIconsetsForm::OnListViewItemSwept(Tizen::Ui::Controls::ListView& listView, int index, Tizen::Ui::Controls::SweepDirection direction)
{
}

void
meecastIconsetsForm::OnListViewContextItemStateChanged(Tizen::Ui::Controls::ListView& listView, int index, int elementId, Tizen::Ui::Controls::ListContextItemStatus state)
{
}

void
meecastIconsetsForm::OnItemReordered(Tizen::Ui::Controls::ListView& view, int oldIndex, int newIndex)
{
}

void
meecastIconsetsForm::OnFastScrollIndexSelected(Control& source, Tizen::Base::String& index)
{
}

void
meecastIconsetsForm::OnTouchPressed (const Control& source, const Point& currentPosition, const TouchEventInfo &touchInfo) {
    Invalidate(true);
}

result
meecastIconsetsForm::GetFilesList(void){
	Directory* pDir = null;
	DirEnumerator* pDirEnum = null;
	StringComparer strComparer;
	result r = E_SUCCESS;

     AppLog("meecastSourcesForm::GetFilesList"); 
	__fileList.Construct();

	pDir = new (std::nothrow) Directory();

	r = pDir->Construct(App::GetInstance()->GetAppResourcePath() + "/screen-density-xhigh/icons");

	TryCatch(r == E_SUCCESS, delete pDir ,"[%s] Failed to construct directory %S", GetErrorMessage(r), ((App::GetInstance()->GetAppResourcePath() + "/screen-density-xhigh/icons").GetPointer()));

	pDirEnum = pDir->ReadN();

	TryCatch(pDirEnum != null, delete pDir ,"[%s] Failed to read entries from directory %S", GetErrorMessage(GetLastResult()), (App::GetInstance()->GetAppResourcePath() + "/screen-density-xhigh/icons").GetPointer());

	while (pDirEnum->MoveNext() == E_SUCCESS){
		DirEntry dirEntry = pDirEnum->GetCurrentDirEntry();
		if ((dirEntry.IsDirectory() == true) && (dirEntry.GetName() != ".") && (dirEntry.GetName() != "..")){
			String* fullFileName = new (std::nothrow) String;
            String fileName(dirEntry.GetName());
			fullFileName->Append(fileName);
			__fileList.Add(fullFileName);
            AppLog("sssssssssss %S", dirEntry.GetName().GetPointer());
		}
	}

	delete pDir;
	delete pDirEnum;

	if (__fileList.GetCount() >= 0){
		return E_SUCCESS;
	}else{
		return E_FAILURE;
	}
CATCH:
	return r;
}
