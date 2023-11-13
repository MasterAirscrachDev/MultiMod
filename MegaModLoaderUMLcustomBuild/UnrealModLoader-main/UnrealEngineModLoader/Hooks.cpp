#include "Hooks.h"
#include "Utilities/MinHook.h"
#include "GameInfo/GameInfo.h"
#include "PakLoader.h"
#include "Utilities/Dumper.h"
#include "Memory/mem.h"
#include "UnrealEngineModLoader/Utilities/Globals.h"
#include "UnrealEngineModLoader/Memory/CoreModLoader.h"
#include "UE4/Ue4.hpp"
#include "LoaderUI.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdio.h>
using namespace std;
bool bIsProcessInternalsHooked = false;
bool GameStateClassInitNotRan = true;

namespace Hooks
{

	namespace HookedFunctions
	{
		struct PrintStringParams
		{
			UE4::FString Message;
		};

		struct SaveStringParams
		{
			UE4::FString Text;
			UE4::FString FileName;
			bool Debug;
		};

		struct RemoveTextParams
		{
			UE4::FString FileName;
		};

		struct LoadStringParams
		{
			UE4::FString FileName;
		};
		struct GetDataParams
		{
			UE4::FString Filename;
		};
		
		struct GetPersistentObject
		{
			UE4::FString ModName;
		};

		

		PVOID(*origProcessFunction)(UE4::UObject*, UE4::FFrame*, void* const);
		PVOID hookProcessFunction(UE4::UObject* obj, UE4::FFrame* Frame, void* const Result)
		{
			PVOID ret = nullptr;
			if (!GameStateClassInitNotRan)
			{
				if (Frame->Node->GetName() == "PrintToModLoader")
				{
					auto msg = Frame->GetInputParams<PrintStringParams>()->Message;
					if (msg.IsValid())
					{
						Log::Print("%s", msg.ToString().c_str());
					}
				}
				if (Frame->Node->GetName() == "SaveStringToTextFile")
				{
					auto text = Frame->GetInputParams<SaveStringParams>()->Text; //Get the text
					auto FileName = Frame->GetInputParams<SaveStringParams>()->FileName; //Get the file name
					auto Debug = Frame->GetInputParams<SaveStringParams>()->Debug; //Get the debug bool
					if (text.IsValid())
					{
						string FileName2 = FileName.ToString().c_str(); //convert the filename to a string
						FileName2.append(".txt"); //append .txt to the end of the filename
						ofstream myfile; //create a file stream
						myfile.open(FileName2); //open the file
						string text2 = text.ToString(); //convert the text to a char*
						if (Debug) //check if debug is enabled
						{
							Log::Info("Saving (" + text2 + ") To File:" + FileName2);
						}
						myfile << text.ToString().c_str(); //write the text to the file
						myfile.flush(); //flush the file
						myfile.close(); //close the file
						//Log::Info("Saved Text To File, deleting file stream");
						//delete &myfile; //delete the file stream
						text = nullptr;
						//Log::Info("Save 2");
						FileName = nullptr;
						//Log::Info("Save 3");
						text2.clear();
						//Log::Info("Save 4");
						//text2 = nullptr;
						//Log::Info("Save done");
					}
				}
				if (Frame->Node->GetName() == "RemoveTextFile")
				{
					auto FileName = Frame->GetInputParams<RemoveTextParams>()->FileName; //Get the file name
					string FileName2 = FileName.ToString().c_str(); //convert the filename to a string
					FileName2.append(".txt"); //append .txt to the end of the filename
					if (remove( FileName2.c_str() ) != 0) {
						Log::Error("Failed To Delete File");
					}
				}
				if (Frame->Node->GetName() == "GetPersistentObject")
				{
					auto ModName = Frame->GetInputParams<GetPersistentObject>()->ModName;
					for (size_t i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
					{
						auto ModInfo = Global::GetGlobals()->ModInfoList[i];
						if (ModName.c_str() == ModInfo.ModName)
						{
							if (ModInfo.PersistentObject)
							{
								UE4::SetVariable<UE4::UObject*>(obj, "GetPersistentObjectReturnValue", ModInfo.PersistentObject);
							}
						}
					}
				}
				if (Frame->Node->GetName() == "GetDataFile")
				{
					//print Lalt 1
					auto FileName = Frame->GetInputParams<GetDataParams>()->Filename; //Get the file name
					string FileName2 = FileName.ToString().c_str(); //convert the filename to a string
					FileName2.append(".txt"); //append .txt to the end of the filename
					std::ifstream file(FileName2); //open the file
					if (!file.is_open()) { //check if the file is open
						Log::Error("Failed To Open File");
						file.close(); //close the file
						UE4::FString ReturnError = UE4::FString(TEXT("ERR")); //create a return string
						//Frame->SetOutput<UE4::FString>("Text", ReturnError); //set the return value
						UE4::SetVariable<UE4::FString>(obj, "CData", ReturnError);
					}
					else {
						std::string content;
						//Log::Print("Lalt 2.7");
						file >> content; //read the file
						Log::Print("Read: (" + content + ")");
						//Log::Print("Lalt 2.8");
						file.close(); //close the file
						//Log::Print("Lalt 2.9");
						wstring ws(content.begin(), content.end()); //convert the file contents to a wstring
						//Log::Print("Lalt 2.10");
						UE4::FString ret = ws.c_str(); //convert the wstring to a UE4::FString
						//Log::Print("Lalt 2.11");
						//Frame->SetOutput<UE4::FString>("Text", ret); //set the return value
						UE4::SetVariable<UE4::FString>(obj, "CData", ret);
						//Log::Print("Lalt 2.12");
						ret = nullptr;
						//Log::Print("Lalt 2.13");
						ws.clear();
						//ws = nullptr;
						//Log::Print("Lalt 2.14");
						content.clear();
						//Log::Print("Lalt 2.15");
					}
					//Log::Print("Lalt 2.16");
					FileName = nullptr;
					//Log::Print("Lalt 2.17");
					FileName2.clear();
					//Log::Print("Read Done");
				}

				for (size_t i = 0; i < Global::GetGlobals()->GetBPFunctionWrappers().size(); i++)
				{
					if (Frame->Node->GetName() == Global::GetGlobals()->GetBPFunctionWrappers()[i].FunctionName)
					{
						reinterpret_cast<void(*)(UE4::UObject*, UE4::FFrame*, void*)> (Global::GetGlobals()->GetBPFunctionWrappers()[i].FuncPtr) (obj, Frame, (void*)Result);
						return nullptr;
					}
				}
			}
			return origProcessFunction(obj, Frame, Result);
		}


		static bool PreBegin_AlreadyLoaded = false;

		PVOID(*origInitGameState)(void*);
		PVOID hookInitGameState(void* Ret)
		{
			Log::Info("GameStateHook");
			PreBegin_AlreadyLoaded = false;
			if (GameStateClassInitNotRan)
			{
				UE4::InitSDK();
				Log::Info("Engine Classes Loaded");
				if (Global::GetGlobals()->CoreMods.size() > 0)
				{
					for (size_t i = 0; i < Global::GetGlobals()->CoreMods.size(); i++)
					{
						auto CurrentCoreMod = Global::GetGlobals()->CoreMods[i];
						if (CurrentCoreMod->IsFinishedCreating)
						{
							Log::Info("InitializeMod Called For %s", CurrentCoreMod->ModName.c_str());
							CurrentCoreMod->InitializeMod();
						}
						else
						{
							Log::Error("Mod %s wasnt setup in time"), CurrentCoreMod->ModName;
						}
					}
				}

				UE4::FTransform transform = UE4::FTransform::FTransform();
				UE4::FActorSpawnParameters spawnParams = UE4::FActorSpawnParameters::FActorSpawnParameters();
				if (GameProfile::SelectedGameProfile.StaticLoadObject)
				{
					Log::Info("StaticLoadObject Found");
				}
				else
				{
					Log::Error("StaticLoadObject Not Found");
				}
				GameStateClassInitNotRan = false;
			}
			for (int i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
			{
				UE4::AActor* CurrentModActor = Global::GetGlobals()->ModInfoList[i].CurrentModActor;
				if (CurrentModActor)
				{
					if (CurrentModActor->IsA(UE4::AActor::StaticClass()))
					{
						if (!DmgConfig::Instance.SkipCallFunctionByNameWithArguments) {
							CurrentModActor->CallFunctionByNameWithArguments(L"ModCleanUp", nullptr, NULL, true);
						}
					}
				}
				
				Global::GetGlobals()->ModInfoList[i].CurrentModActor = nullptr;
				Global::GetGlobals()->ModInfoList[i].ModButtons.clear();
			}
			if (GameProfile::SelectedGameProfile.StaticLoadObject)
			{
				UE4::FTransform transform;
				transform.Translation = UE4::FVector(0, 0, 0);
				transform.Rotation = UE4::FQuat(0, 0, 0, 0);
				transform.Scale3D = UE4::FVector(1, 1, 1);
				UE4::FActorSpawnParameters spawnParams = UE4::FActorSpawnParameters::FActorSpawnParameters();
				for (int i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
				{
					std::wstring CurrentMod;
					//StartSpawningMods
					CurrentMod = Global::GetGlobals()->ModInfoList[i].ModName;
					if (Global::GetGlobals()->ModInfoList[i].IsEnabled)
					{
						if (GameProfile::SelectedGameProfile.StaticLoadObject)
						{
							std::string str(CurrentMod.begin(), CurrentMod.end());
							const std::wstring Path = L"/Game/Mods/" + CurrentMod + L"/ModActor.ModActor_C";
							UE4::UClass* ModObject = UE4::UClass::LoadClassFromString(Path.c_str(), false);
							if (ModObject)
							{
								UE4::AActor* ModActor = nullptr;
								if (!GameProfile::SelectedGameProfile.IsUsingDeferedSpawn)
								{
									ModActor = UE4::UWorld::GetWorld()->SpawnActor(ModObject, &transform, &spawnParams);
								}
								else
								{
									ModActor = UE4::UGameplayStatics::BeginDeferredActorSpawnFromClass(ModObject, transform, UE4::ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr);
								}
								if (ModActor)
								{
									if (!bIsProcessInternalsHooked)
									{
										if (!GameProfile::SelectedGameProfile.ProcessInternals)
										{
											if (!GameProfile::SelectedGameProfile.UsesFNamePool || !GameProfile::SelectedGameProfile.IsUsing4_22)
											{
												if (ModActor->DoesObjectContainFunction("PostBeginPlay"))
												{
													GameProfile::SelectedGameProfile.ProcessInternals = (DWORD64)ModActor->GetFunction("PostBeginPlay")->GetFunction();
												}
												else if (ModActor->DoesObjectContainFunction("ModMenuButtonPressed"))
												{
													GameProfile::SelectedGameProfile.ProcessInternals = (DWORD64)ModActor->GetFunction("ModMenuButtonPressed")->GetFunction();
												}
											}
										}
										bIsProcessInternalsHooked = true;
										if (GameProfile::SelectedGameProfile.ProcessInternals)
											MinHook::Add(GameProfile::SelectedGameProfile.ProcessInternals, &HookedFunctions::hookProcessFunction, &HookedFunctions::origProcessFunction, "ProcessBlueprintFunctions");
										else
											Log::Warn("ProcessBlueprintFunctions could not be located! Mod Loader Functionality Will be Limited!");
									}

									for (size_t i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
									{
										if (Global::GetGlobals()->ModInfoList[i].ModName == CurrentMod)
										{
											Global::GetGlobals()->ModInfoList[i].CurrentModActor = ModActor;
											if (!Global::GetGlobals()->ModInfoList[i].WasInitialized)
											{
												Global::GetGlobals()->ModInfoList[i].ContainsButton = ModActor->DoesObjectContainFunction("ModMenuButtonPressed");
												UE4::FString Author;
												UE4::FString Description;
												UE4::FString Version;
												if (UE4::GetVariable<UE4::FString>(ModActor, "ModAuthor", Author))
												{
													if (Author.IsValid())
													{
														Global::GetGlobals()->ModInfoList[i].ModAuthor = Author.ToString();
													}
												}
												if (UE4::GetVariable<UE4::FString>(ModActor, "ModDescription", Description))
												{
													if (Description.IsValid())
													{
														Global::GetGlobals()->ModInfoList[i].ModDescription = Description.ToString();
													}
												}
												if (UE4::GetVariable<UE4::FString>(ModActor, "ModVersion", Version))
												{
													if (Version.IsValid())
													{
														Global::GetGlobals()->ModInfoList[i].ModVersion = Version.ToString();
													}
												}
												const std::wstring ModInstancePath = L"/Game/Mods/" + CurrentMod + L"/ModInstanceObject.ModInstanceObject_C";
												UE4::UClass* ModObjectInstanceClass = UE4::UClass::LoadClassFromString(ModInstancePath.c_str(), false);
												if (ModObjectInstanceClass)	// Check if ModInstanceObject Exists
												{
													Global::GetGlobals()->ModInfoList[i].PersistentObject = UE4::UObject::StaticConstructObject_Internal(ModObjectInstanceClass, UE4::UGameplayStatics::GetGameInstance(), "", 0, UE4::EInternalObjectFlags::GarbageCollectionKeepFlags, nullptr, false, nullptr, false);
												}
												Global::GetGlobals()->ModInfoList[i].WasInitialized = true;
											}
										}
									}
									// UE4.26 FIX: duplicated ModActor(PostBeginPlay called twice) on ToggleDebugCamera
									if (!DmgConfig::Instance.SkipCallFunctionByNameWithArguments) {
										ModActor->CallFunctionByNameWithArguments(L"PreBeginPlay", nullptr, NULL, true);
									}
									Log::Info("Sucessfully Loaded %s", str.c_str());
								}
							}
							else
							{
								Log::Info("Could not locate ModActor for %s", str.c_str());
							}
						}
					}
				}
				Log::Info("Finished Spawning PakMods");
				Global::GetGlobals()->eventSystem.dispatchEvent("InitGameState");
			}
			Log::Info("Returning to GameState --------------------------------------------------------");
			return origInitGameState(Ret);
		}

		PVOID(*origBeginPlay)(UE4::AActor*);
		PVOID hookBeginPlay(UE4::AActor* Actor)
		{
			if (!GameStateClassInitNotRan)
			{
				if (Actor->IsA(UE4::ACustomClass::StaticClass(GameProfile::SelectedGameProfile.BeginPlayOverwrite)))
				{
					Log::Info("Beginplay Called");
					for (int i = 0; i < Global::GetGlobals()->ModInfoList.size(); i++)
					{
						UE4::AActor* CurrentModActor = Global::GetGlobals()->ModInfoList[i].CurrentModActor;
						if (CurrentModActor != nullptr)
						{
							UE4::TArray<UE4::FString> ModButtons;
							if (UE4::GetVariable<UE4::TArray<UE4::FString>>(CurrentModActor, "ModButtons", ModButtons))
							{
								for (size_t bi = 0; bi < ModButtons.Num(); bi++)
								{
									auto CurrentButton = ModButtons[bi];
									if (CurrentButton.IsValid())
									{
										Global::GetGlobals()->ModInfoList[i].ModButtons.push_back(CurrentButton.ToString());
									}
								}
							}
							// UE4.26 FIX: duplicated ModActor(PostBeginPlay called twice) on ToggleDebugCamera
							if (!PreBegin_AlreadyLoaded) {
								if (!DmgConfig::Instance.SkipCallFunctionByNameWithArguments) {
									CurrentModActor->CallFunctionByNameWithArguments(L"PostBeginPlay", nullptr, NULL, true);
								}
								Global::GetGlobals()->eventSystem.dispatchEvent("PostBeginPlay", Global::GetGlobals()->ModInfoList[i].ModName, CurrentModActor);
							}
						}
					}
					PreBegin_AlreadyLoaded = true;
					Global::GetGlobals()->eventSystem.dispatchEvent("BeginPlay", Actor);
				}
				//Global::GetGlobals()->eventSystem.dispatchEvent("BeginPlay", Actor);
			}
			return origBeginPlay(Actor);
		}
	};



	DWORD __stdcall InitHooks(LPVOID)
	{
		MinHook::Init();
		Log::Info("MinHook Setup");
		Log::Info("Loading Core Mods");
		CoreModLoader::LoadCoreMods();
		Sleep(10);
		PakLoader::ScanLoadedPaks();
		Log::Info("ScanLoadedPaks Setup");
		MinHook::Add(GameProfile::SelectedGameProfile.GameStateInit, &HookedFunctions::hookInitGameState, &HookedFunctions::origInitGameState, "AGameModeBase::InitGameState");
		MinHook::Add(GameProfile::SelectedGameProfile.BeginPlay, &HookedFunctions::hookBeginPlay, &HookedFunctions::origBeginPlay, "AActor::BeginPlay");
		LoaderUI::GetUI()->CreateUILogicThread();
		if (!GameProfile::SelectedGameProfile.bDelayGUISpawn)
		{
			LoaderUI::HookDX();
		}
		return NULL;
	}

	void SetupHooks()
	{
		Log::Info("Setting Up Loader");
		CreateThread(0, 0, InitHooks, 0, 0, 0);
	}
};