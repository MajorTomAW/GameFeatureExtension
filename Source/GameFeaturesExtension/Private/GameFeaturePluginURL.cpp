#include "GameFeaturePluginURL.h"

FGameFeaturePluginURL::FGameFeaturePluginURL(const FString& InURL, const FString& InName)
{
	PluginURL = InURL;
	PluginName = InName;
}

void FGameFeaturePluginURL::SetURL(FString InURL, FString InName)
{
	PluginURL = InURL;

	if (InName.IsEmpty())
	{
		PluginName = FPaths::GetBaseFilename(InURL);
		PluginName.RemoveFromEnd(FPaths::GetExtension(InURL));
	}
	else
	{
		PluginName = InName;
	}
}

void FGameFeaturePluginURL::SetURL(FName URL)
{
	SetURL(URL.ToString());
}

bool FGameFeaturePluginURL::Serialize(FArchive& Ar)
{
	SerializePath(Ar);
	return true;
}

bool FGameFeaturePluginURL::operator==(FGameFeaturePluginURL const& Other) const
{
	return PluginURL == Other.PluginURL && PluginName == Other.PluginName;
}

void FGameFeaturePluginURL::SerializePath(FArchive& Ar)
{
	bool bSerializeInternals = true;

#if WITH_EDITOR
	if (Ar.IsSaving())
	{
		Ar << PluginURL;
		Ar << PluginName;
	}

	if (Ar.IsLoading())
	{
	}
#endif
}
