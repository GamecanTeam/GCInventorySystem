// Fill out your copyright notice in the Description page of Project Settings.

#include "GCInventoryGISSubsystems.h"
#include "Interfaces/GCInventoryInterface.h"
#include "Modules/GCInventorySystem.h"
#include "Engine/GCInventoryMappingDataAsset.h"
#include <GameFramework/PlayerState.h>
#include <InstancedStruct.h>
#include <Kismet/DataTableFunctionLibrary.h>

UGCInventoryGISSubsystems::UGCInventoryGISSubsystems()
{
	ItemsDataAsset = nullptr;
	AllItemsInventory.Empty();
}

UGCInventoryGISSubsystems* UGCInventoryGISSubsystems::Get(const UObject* worldContextObject)
{
	UGCInventoryGISSubsystems* InventorySubsystem = nullptr;

	if (const auto world = GEngine->GetWorldFromContextObject(worldContextObject, EGetWorldErrorMode::Assert))
	{
		InventorySubsystem = UGameInstance::GetSubsystem<UGCInventoryGISSubsystems>(world->GetGameInstance());
	}

	check(InventorySubsystem);

	return InventorySubsystem;
}

void UGCInventoryGISSubsystems::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	InitializeItemsInformation();
}

void UGCInventoryGISSubsystems::Deinitialize()
{
	Super::Deinitialize();
}

void UGCInventoryGISSubsystems::ItemAddedToInventory(FGameplayTag itemTag, APlayerState* playerReference)
{
	if (playerReference && playerReference->GetClass()->ImplementsInterface(UGCInventoryInterface::StaticClass()))
	{
	}
}

FItemKeyInfo UGCInventoryGISSubsystems::GetItemInformationFromTag(FGameplayTag itemTag)
{
	if (AllItemsInventory.Contains(itemTag))
	{
		return AllItemsInventory[itemTag];
	}

	UE_LOG(LogInventorySystem, Warning, TEXT("[%s] Could not find the item with the tag: %s"), ANSI_TO_TCHAR(__FUNCTION__), *itemTag.ToString());

	return FItemKeyInfo();
}

void UGCInventoryGISSubsystems::InitializeItemsInformation()
{
	if (ensureMsgf(ItemsDataAsset.IsValid(), TEXT("Items data asset is not valid, without this file the system won't work. Please Fix it")))
	{
		if (const auto& dataAsset = ItemsDataAsset.LoadSynchronous())
		{
			TArray<FGameplayTag> itemCategories;
			dataAsset->ItemsCategoryMap.GetKeys(itemCategories);

			if (itemCategories.Num() > 0)
			{
				for (const auto categoryTag : itemCategories)
				{
					if (const auto itemCategory = dataAsset->FindItemsDataTable(categoryTag))
					{
						TArray<FName> tableRowNames;
						UDataTableFunctionLibrary::GetDataTableRowNames(itemCategory, tableRowNames);

						if (tableRowNames.Num() > 0)
						{
							for (const auto itemNameTag : tableRowNames)
							{
								const auto itemTag = FGameplayTag::RequestGameplayTag(itemNameTag);
								FItemKeyInfo newItemInfo;
								newItemInfo.ItemTag = itemTag;
								newItemInfo.ItemCategoryTag = categoryTag;
								AllItemsInventory.Add(itemTag, newItemInfo);
							}
						}
						else
						{
							UE_LOG(LogInventorySystem, Warning, TEXT("[%s] Item Category info is empty. Please fill it with information."), ANSI_TO_TCHAR(__FUNCTION__));
						}
					}
					else
					{
						UE_LOG(LogInventorySystem, Error, TEXT("[%s] Failed to find the desired item category"), ANSI_TO_TCHAR(__FUNCTION__));
					}
				}
			}
			else
			{
				UE_LOG(LogInventorySystem, Warning, TEXT("[%s] Item Data Asset is empty. Please fill it with information."), ANSI_TO_TCHAR(__FUNCTION__));
			}
		}
		else
		{
			UE_LOG(LogInventorySystem, Error, TEXT("[%s] Failed to find ItemsDataAsset. Cannot fill the item information."), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("[%s] Item Data asset reference is not valid"), ANSI_TO_TCHAR(__FUNCTION__));
	}
}