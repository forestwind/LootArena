// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/LISampleItem.h"
#include "LIGameplayTags.h"

FGameplayTag FLISampleItemRow::GetItemType() const
{
	return LIGameplayTags::Item_Type_Sample;
}
