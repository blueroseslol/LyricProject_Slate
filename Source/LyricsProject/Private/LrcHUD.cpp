﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "LyricsProject.h"
#include "LrcHUD.h"

#include "StandardWidget.h"
#include "LrcMenuWidget.h"
#include "VorbisAudioInfo.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"




ALrcHUD::ALrcHUD()
{
	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("MyAudioComponent"));
	AudioComp->bAutoActivate = false;

	//ConstructorHelpers::FObjectFinder<USoundBase> MySound(TEXT("SoundWave'/Game/青空のナミダ.青空のナミダ'"));
	//ConstructorHelpers::FObjectFinder<ULyricAsset>	 MyLrcAsset(TEXT("LyricAsset'/Game/高桥瞳_-_青空のナミダ.高桥瞳_-_青空のナミダ'"));

	/*if (MySound.Succeeded())
	{
		AudioComp->SetSound(MySound.Object);
	}

	if (MyLrcAsset.Succeeded())
	{
		LyricAsset = MyLrcAsset.Object;
	}*/
}


void ALrcHUD::BeginPlay()
{
	Super::BeginPlay();

	USoundWave* MySondWave = ImportMusicByFilePath(TEXT("GARNiDELiA - 極楽浄土.ogg"));

	LyricAsset = ImportLyricByFilePath(TEXT("GARNiDELiA - 極楽浄土.lrc"));

	check(MySondWave);

	AudioComp->SetSound(MySondWave);

	GetOwningPlayerController()->bShowMouseCursor = true;

	if (GEngine&&GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(
			SAssignNew(MyWidget, SLrcMenuWidget)
			.OwnerHUD(this)
			.PlayedSeconds_UObject(this,&ALrcHUD::GetPlayedSeconds)
		));
	}

	FTimerHandle  TheHandle;

	GetWorldTimerManager().SetTimer(TheHandle, this, &ALrcHUD::PlaySound, 2);
}



void ALrcHUD::PlaySound()
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, TEXT("PlaySound start"));

	if (AudioComp&&AudioComp->Sound)
	{
		SoundDuration = AudioComp->Sound->Duration;
		StartTime = GetWorld()->GetTimeSeconds();

		AudioComp->Play();

	}
}

float ALrcHUD::GetPlayedSeconds() const
{
	if (StartTime > 0 && ensure(SoundDuration > 0))
	{
		return  FMath::Min(GetWorld()->GetTimeSeconds() - StartTime, SoundDuration);
	}
	else
	{
		return 0;
	}
}

class USoundWave* ALrcHUD::ImportMusicByFilePath(const FString& Filename)
{
	TArray<uint8>  RawFile;

	const	FString Path = FPaths::GameDir() + Filename;
	if (!FFileHelper::LoadFileToArray(RawFile, *Path))
	{
	UE_LOG(LogTemp,Warning,TEXT(" File not exist path:[%s] "), *Path)  ;
		return nullptr;
	}

	USoundWave*  Sound = NewObject<USoundWave>(this);
	check(Sound);

	
	FSoundQualityInfo Info;
	if (!FVorbisAudioInfo().ReadCompressedInfo(RawFile.GetData(), RawFile.Num(), &Info))
	{
		UE_LOG(LogTemp, Warning, TEXT(" File can not read as Ogg "), *Path);
		return nullptr;
	}

	Sound->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	Sound->NumChannels = Info.NumChannels;
	Sound->Duration = Info.Duration;
	Sound->RawPCMDataSize = Info.SampleDataSize;
	Sound->SampleRate = Info.SampleRate;

	FByteBulkData& BulkData = Sound->CompressedFormatData.GetFormat("ogg");

	BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(BulkData.Realloc(RawFile.Num()), RawFile.GetData(), RawFile.Num());
	BulkData.Unlock();

	return Sound;
}

class ULyricAsset* ALrcHUD::ImportLyricByFilePath(const FString& FileName)
{
	FString LrcString;
	FString Path = FPaths::GameDir() + FileName;

	if (!FFileHelper::LoadFileToString(LrcString, *Path))
	{
		UE_LOG(LogTemp, Warning, TEXT(" File not exist path:[%s] "), *Path);
		return nullptr;
	}

	ULyricAsset* NewAsset = NewObject<ULyricAsset>(this);
	NewAsset->LoadFromString(LrcString);
	return  NewAsset;
}

