#include "Multicore.h"
#include "Utilities/MinHook.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

// BPFUNCTION(WriteToFile)
// {
// 	std::cout << "WriteToFile" << std::endl;
// 	struct InputParams
// 	{
// 		UE4::FString NameTest;
// 	};
// 	auto Inputs = stack->GetInputParams<InputParams>();
// 	stack->SetOutput<UE4::FString>("OutPutString", L"KboyGang");
// 	stack->SetOutput<bool>("ReturnValue", true);
// }
BPFUNCTION(SavePlaintext)
{
	std::cout << "SavePlaintext" << std::endl;
	struct InputParams
	{
		UE4::FString Filename;
		UE4::FString Text;
	};
	auto Inputs = stack->GetInputParams<InputParams>();
	string filecontent = Inputs.Text;
	string fileName = Inputs.Filename;
	while (std::cin >> std::noskipws >> fileContent) {
        // Append the content to the existing content (if any)
        std::ofstream outputFile(fileName, std::ios::app);

        // Check if the file is open
        if (!outputFile.is_open()) {
            std::cerr << "Error opening the file for writing." << std::endl;
			stack->SetOutput<bool>("Success", false);
            return 1; // Return an error code

        }

        // Write the content to the file
        outputFile << fileContent;
        outputFile.close();

        std::cout << "Content added to the file." << std::endl;

        // Clear the fileContent string for the next input
        fileContent.clear();
    }

    std::cout << "File saved successfully." << std::endl;
	stack->SetOutput<bool>("Success", true);
}
BPFUNCTION(ReadPlaintext)
{
	std::cout << "ReadPlaintext" << std::endl;
	struct InputParams
	{
		UE4::FString Filename;
	};
	auto Inputs = stack->GetInputParams<InputParams>();
	string fileName = Inputs.Filename;

	 // Create an input file stream and open the file
    std::ifstream inputFile(fileName);

    // Check if the file is open
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file for reading." << std::endl;
		stack->SetOutput<bool>("Success", false);
        //return 1; // Return an error code
    }

    // Read and display the contents of the file
    std::string line;
    while (std::getline(inputFile, line)) {
        std::cout << line << std::endl;
    }

    // Close the file
    inputFile.close();
	stack->SetOutput<bool>("Success", true);
}
BPFUNCTION(DeleteFile)
{
	std::cout << "DeleteFile" << std::endl;
	struct InputParams
	{
		UE4::FString Filename;
	};
	auto Inputs = stack->GetInputParams<InputParams>();
	if (std::remove(Inputs.Filename) != 0) {
        perror("Error deleting the file");
		stack->SetOutput<bool>("Success", false);
    } else {
        printf("File deleted successfully.\n");
		stack->SetOutput<bool>("Success", true);
    }
}

// Only Called Once, if you need to hook shit, declare some global non changing values
void MultiMod::InitializeMod()
{
	UE4::InitSDK();
	SetupHooks();

	//REGISTER_FUNCTION(WriteToFile);
	REGISTER_FUNCTION(SavePlaintext);
	REGISTER_FUNCTION(ReadPlaintext);
	REGISTER_FUNCTION(DeleteFile);

	//MinHook::Init(); //Uncomment if you plan to do hooks

	//UseMenuButton = true; // Allows Mod Loader To Show Button
}

void MultiMod::InitGameState()
{
}

void MultiMod::BeginPlay(UE4::AActor* Actor)
{
	
}

void MultiMod::PostBeginPlay(std::wstring ModActorName, UE4::AActor* Actor)
{
	// Filters Out All Mod Actors Not Related To Your Mod
	std::wstring TmpModName(ModName.begin(), ModName.end());
	if (ModActorName == TmpModName)
	{
		//Sets ModActor Ref
		ModActor = Actor;
	}
}

void MultiMod::DX11Present(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRenderTargetView)
{
}


void MultiMod::OnModMenuButtonPressed()
{
}

void MultiMod::DrawImGui()
{
}