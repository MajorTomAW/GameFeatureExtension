// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFeaturePluginURL.generated.h"

/**
 * A struct that contains a string reference to a game feature plugin.
 * This can be used to reference game feature plugins that are loaded on demand.
 */
USTRUCT(BlueprintType)
struct FGameFeaturePluginURL
{
	GENERATED_BODY()

public:
	FGameFeaturePluginURL() = default;
	FGameFeaturePluginURL(const FGameFeaturePluginURL& Other) = default;
	FGameFeaturePluginURL(FGameFeaturePluginURL&& Other) = default;
	~FGameFeaturePluginURL() = default;
	FGameFeaturePluginURL& operator=(const FGameFeaturePluginURL& Path) = default;
	FGameFeaturePluginURL& operator=(FGameFeaturePluginURL&& Path) = default;

	/** Construct from a plugin path string. */
	explicit FGameFeaturePluginURL(const FString& InURL) { SetURL(InURL); }
	explicit FGameFeaturePluginURL(TYPE_OF_NULLPTR) {}
	FGameFeaturePluginURL(const FString& InURL, const FString& InName);

	/** Sets the plugin path of this reference based on a string path. */
	GAMEFEATURESEXTENSION_API void SetURL(FString InURL, FString InName = FString());
	GAMEFEATURESEXTENSION_API void SetURL(FName URL);

	/** Returns the URL of the game feature plugin. */
	const FString& GetURL() const { return PluginURL; }
	FString GetURL() { return PluginURL; }

	/** Returns the name of the game feature plugin. */
	const FString& GetPluginName() const { return PluginName; }
	FString GetPluginName() { return PluginName; }

	/** Resets reference to null. */
	void Reset()
	{
		PluginURL.Reset();
		PluginName.Reset();
	}

	/** Check if this could be a valid plugin URL */
	FORCEINLINE bool IsValid() const
	{
		return !PluginURL.IsEmpty();
	}

	/** Struct overrides */
	GAMEFEATURESEXTENSION_API bool Serialize(FArchive& Ar);
	GAMEFEATURESEXTENSION_API bool operator==(FGameFeaturePluginURL const& Other) const;
	bool operator!=(FGameFeaturePluginURL const& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE friend uint32 GetTypeHash(FGameFeaturePluginURL const& This)
	{
		uint32 Hash = 0;
		Hash = HashCombine(Hash, GetTypeHash(This.PluginURL));
		Hash = HashCombine(Hash, GetTypeHash(This.PluginName));
		return Hash;
	}

	/** Serializes the internal plugin url. */
	GAMEFEATURESEXTENSION_API void SerializePath(FArchive& Ar);

private:
	/** The URL of the game feature plugin. */
	UPROPERTY(VisibleDefaultsOnly)
	FString PluginURL;
	
	/** The name of the game feature plugin. */
	UPROPERTY(VisibleDefaultsOnly)
	FString PluginName;
};
