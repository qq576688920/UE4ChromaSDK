// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ChromaSDKPlugin.h"
#include "AnimationBase.h"
#include "Animation1D.h"
#include "Animation2D.h"
#include "ChromaThread.h"

#define LOCTEXT_NAMESPACE "FChromaSDKPluginModule"

#if PLATFORM_WINDOWS

#include "AllowWindowsPlatformTypes.h" 

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#else
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif

typedef unsigned char byte;
#define ANIMATION_VERSION 1

using namespace ChromaSDK;
using namespace ChromaSDK::ChromaLink;
using namespace ChromaSDK::Headset;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace std;

bool FChromaSDKPluginModule::ValidateGetProcAddress(bool condition, FString methodName)
{
	if (condition)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin failed to load %s!"), *methodName);
	}
	else
	{
		//UE_LOG(LogTemp, Log, TEXT("ChromaSDKPlugin loaded %s."), *methodName);
	}
	return condition;
}

#endif

void FChromaSDKPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if PLATFORM_WINDOWS
	_mInitialized = false;
	_mAnimationId = 0;
	_mAnimationMapID.clear();
	_mAnimations.clear();
	_mPlayMap1D.clear();
	_mPlayMap2D.clear();

	_mLibraryChroma = LoadLibrary(CHROMASDKDLL);
	if (_mLibraryChroma == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin failed to load!"));
		return;
	}
	//UE_LOG(LogTemp, Log, TEXT("ChromaSDKPlugin loaded."));

	// GetProcAddress will throw 4191 because it's an unsafe type cast
#pragma warning(disable: 4191)
	_mMethodInit = (CHROMA_SDK_INIT)GetProcAddress(_mLibraryChroma, "Init");
	if (ValidateGetProcAddress(_mMethodInit == nullptr, FString("Init")))
	{
		return;
	}
	_mMethodUnInit = (CHROMA_SDK_UNINIT)GetProcAddress(_mLibraryChroma, "UnInit");
	if (ValidateGetProcAddress(_mMethodUnInit == nullptr, FString("UnInit")))
	{
		return;
	}
	_mMethodQueryDevice = (CHROMA_SDK_QUERY_DEVICE)GetProcAddress(_mLibraryChroma, "QueryDevice");
	if (ValidateGetProcAddress(_mMethodQueryDevice == nullptr, FString("QueryDevice")))
	{
		return;
	}
	_mMethodUnInit = (CHROMA_SDK_UNINIT)GetProcAddress(_mLibraryChroma, "UnInit");
	if (ValidateGetProcAddress(_mMethodUnInit == nullptr, FString("UnInit")))
	{
		return;
	}

	_mMethodCreateChromaLinkEffect = (CHROMA_SDK_CREATE_CHROMA_LINK_EFFECT)GetProcAddress(_mLibraryChroma, "CreateChromaLinkEffect");
	if (ValidateGetProcAddress(_mMethodCreateChromaLinkEffect == nullptr, FString("CreateChromaLinkEffect")))
	{
		return;
	}
	_mMethodCreateHeadsetEffect = (CHROMA_SDK_CREATE_HEADSET_EFFECT)GetProcAddress(_mLibraryChroma, "CreateHeadsetEffect");
	if (ValidateGetProcAddress(_mMethodCreateHeadsetEffect == nullptr, FString("CreateHeadsetEffect")))
	{
		return;
	}
	_mMethodCreateKeyboardEffect = (CHROMA_SDK_CREATE_KEYBOARD_EFFECT)GetProcAddress(_mLibraryChroma, "CreateKeyboardEffect");
	if (ValidateGetProcAddress(_mMethodCreateKeyboardEffect == nullptr, FString("CreateKeyboardEffect")))
	{
		return;
	}
	_mMethodCreateMouseEffect = (CHROMA_SDK_CREATE_MOUSE_EFFECT)GetProcAddress(_mLibraryChroma, "CreateMouseEffect");
	if (ValidateGetProcAddress(_mMethodCreateMouseEffect == nullptr, FString("CreateMouseEffect")))
	{
		return;
	}
	_mMethodCreateMousepadEffect = (CHROMA_SDK_CREATE_MOUSEPAD_EFFECT)GetProcAddress(_mLibraryChroma, "CreateMousepadEffect");
	if (ValidateGetProcAddress(_mMethodCreateMousepadEffect == nullptr, FString("CreateMousepadEffect")))
	{
		return;
	}
	_mMethodCreateKeypadEffect = (CHROMA_SDK_CREATE_KEYPAD_EFFECT)GetProcAddress(_mLibraryChroma, "CreateKeypadEffect");
	if (ValidateGetProcAddress(_mMethodCreateKeypadEffect == nullptr, FString("CreateKeypadEffect")))
	{
		return;
	}

	_mMethodCreateEffect = (CHROMA_SDK_CREATE_EFFECT)GetProcAddress(_mLibraryChroma, "CreateEffect");
	if (ValidateGetProcAddress(_mMethodCreateEffect == nullptr, FString("CreateEffect")))
	{
		return;
	}
	_mMethodSetEffect = (CHROMA_SDK_SET_EFFECT)GetProcAddress(_mLibraryChroma, "SetEffect");
	if (ValidateGetProcAddress(_mMethodSetEffect == nullptr, FString("SetEffect")))
	{
		return;
	}
	_mMethodDeleteEffect = (CHROMA_SDK_DELETE_EFFECT)GetProcAddress(_mLibraryChroma, "DeleteEffect");
	if (ValidateGetProcAddress(_mMethodDeleteEffect == nullptr, FString("DeleteEffect")))
	{
		return;
	}
#pragma warning(default: 4191)

	UChromaSDKPluginBPLibrary::ChromaSDKInit();

	ChromaThread::Instance()->Start();
#endif
}

void FChromaSDKPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
#if PLATFORM_WINDOWS
	ChromaThread::Instance()->Stop();

	UChromaSDKPluginBPLibrary::ChromaSDKUnInit();

	if (_mLibraryChroma)
	{
		FreeLibrary(_mLibraryChroma);
		_mLibraryChroma = nullptr;
	}

	//UE_LOG(LogTemp, Log, TEXT("ChromaSDKPlugin unloaded."));
#endif
}

#if PLATFORM_WINDOWS

bool FChromaSDKPluginModule::IsInitialized()
{
	return _mInitialized;
}

int FChromaSDKPluginModule::ChromaSDKInit()
{
	if (_mMethodInit == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin Init method is not set!"));
		return -1;
	}

	int result = _mMethodInit();
	if (result == 0)
	{
		_mInitialized = true;
	}
	UE_LOG(LogTemp, Log, TEXT("ChromaSDKPlugin [INITIALIZED] result=%d"), result);
	return result;
}

int FChromaSDKPluginModule::ChromaSDKUnInit()
{
	if (_mMethodUnInit == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin UnInit method is not set!"));
		return -1;
	}

	while (_mAnimations.size() > 0)
	{
		int animationId = _mAnimations.begin()->first;
		StopAnimation(animationId);
		CloseAnimation(animationId);
	}

	int result = _mMethodUnInit();
	_mInitialized = false;
	_mAnimationId = 0;
	_mAnimationMapID.clear();
	_mAnimations.clear();
	_mPlayMap1D.clear();
	_mPlayMap2D.clear();
	//UE_LOG(LogTemp, Log, TEXT("ChromaSDKPlugin [UNINITIALIZED] result=%d"), result);
	return result;
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateEffect(RZDEVICEID deviceId, ChromaSDK::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateEffect(deviceId, effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateChromaLinkEffect(ChromaSDK::ChromaLink::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateChromaLinkEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateChromaLinkEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateChromaLinkEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateHeadsetEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateHeadsetEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateHeadsetEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateKeyboardEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateKeyboardEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateKeyboardEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateKeypadEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateKeypadEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateKeypadEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateMouseEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateMouseEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateMouseEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKCreateMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_mMethodCreateMousepadEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin CreateMousepadEffect method is not set!"));
		return -1;
	}

	return _mMethodCreateMousepadEffect(effect, pParam, pEffectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKSetEffect(RZEFFECTID effectId)
{
	if (_mMethodSetEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin SetEffect method is not set!"));
		return -1;
	}

	return _mMethodSetEffect(effectId);
}

RZRESULT FChromaSDKPluginModule::ChromaSDKDeleteEffect(RZEFFECTID effectId)
{
	if (_mMethodDeleteEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ChromaSDKPlugin DeleteEffect method is not set!"));
		return -1;
	}

	return _mMethodDeleteEffect(effectId);
}

int FChromaSDKPluginModule::ToBGR(const FLinearColor& color)
{
	int red = color.R * 255;
	int green = color.G * 255;
	int blue = color.B * 255;
	return RGB(red, green, blue);
}

FLinearColor FChromaSDKPluginModule::ToLinearColor(int color)
{
	float red = GetRValue(color) / 255.0f;
	float green = GetGValue(color) / 255.0f;
	float blue = GetBValue(color) / 255.0f;
	return FLinearColor(red, green, blue, 1.0f);
}

int FChromaSDKPluginModule::GetMaxLeds(const EChromaSDKDevice1DEnum& device)
{
#if PLATFORM_WINDOWS
	switch (device)
	{
	case EChromaSDKDevice1DEnum::DE_ChromaLink:
		return ChromaSDK::ChromaLink::MAX_LEDS;
	case EChromaSDKDevice1DEnum::DE_Headset:
		return ChromaSDK::Headset::MAX_LEDS;
	case EChromaSDKDevice1DEnum::DE_Mousepad:
		return ChromaSDK::Mousepad::MAX_LEDS;
	}
#endif
	return 0;
}

int FChromaSDKPluginModule::GetMaxRow(const EChromaSDKDevice2DEnum& device)
{
#if PLATFORM_WINDOWS
	switch (device)
	{
	case EChromaSDKDevice2DEnum::DE_Keyboard:
		return ChromaSDK::Keyboard::MAX_ROW;
	case EChromaSDKDevice2DEnum::DE_Keypad:
		return ChromaSDK::Keypad::MAX_ROW;
	case EChromaSDKDevice2DEnum::DE_Mouse:
		return ChromaSDK::Mouse::MAX_ROW;
	}
#endif
	return 0;
}

int FChromaSDKPluginModule::GetMaxColumn(const EChromaSDKDevice2DEnum& device)
{
	int result = 0;
#if PLATFORM_WINDOWS
	switch (device)
	{
	case EChromaSDKDevice2DEnum::DE_Keyboard:
		return ChromaSDK::Keyboard::MAX_COLUMN;
	case EChromaSDKDevice2DEnum::DE_Keypad:
		return ChromaSDK::Keypad::MAX_COLUMN;
	case EChromaSDKDevice2DEnum::DE_Mouse:
		return ChromaSDK::Mouse::MAX_COLUMN;
	}
#endif
	return result;
}

int FChromaSDKPluginModule::OpenAnimation(const char* path)
{
	AnimationBase* animation = nullptr;

	//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: %s"), path);

	FILE* stream = nullptr;
	if (0 != fopen_s(&stream, path, "rb") ||
		stream == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Failed to open animation! %s"), *FString(UTF8_TO_TCHAR(path)));
		return -1;
	}

	long read = 0;
	long expectedRead = 1;
	long expectedSize = sizeof(byte);

	//version
	int version = 0;
	expectedSize = sizeof(int);
	read = fread(&version, expectedSize, 1, stream);
	if (read != expectedRead)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Failed to read version!"));
		std::fclose(stream);
		return -1;
	}
	if (version != ANIMATION_VERSION)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Unexpected Version!"));
		std::fclose(stream);
		return -1;
	}

	//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Version: %d"), version);

	//device
	byte device = 0;

	// device type
	byte deviceType = 0;
	expectedSize = sizeof(byte);
	read = fread(&deviceType, expectedSize, 1, stream);
	if (read != expectedRead)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Unexpected DeviceType!"));
		std::fclose(stream);
		return -1;
	}

	//device
	switch ((EChromaSDKDeviceTypeEnum)deviceType)
	{
	case EChromaSDKDeviceTypeEnum::DE_1D:
		//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: DeviceType: 1D"));
		break;
	case EChromaSDKDeviceTypeEnum::DE_2D:
		//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: DeviceType: 2D"));
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Unexpected DeviceType!"));
		std::fclose(stream);
		return -1;
	}

	switch ((EChromaSDKDeviceTypeEnum)deviceType)
	{
	case EChromaSDKDeviceTypeEnum::DE_1D:
		read = fread(&device, expectedSize, 1, stream);
		if (read != expectedRead)
		{
			UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Unexpected Device!"));
			std::fclose(stream);
			return -1;
		}
		else
		{
			switch ((EChromaSDKDevice1DEnum)device)
			{
			case EChromaSDKDevice1DEnum::DE_ChromaLink:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_ChromaLink"));
				break;
			case EChromaSDKDevice1DEnum::DE_Headset:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_Headset"));
				break;
			case EChromaSDKDevice1DEnum::DE_Mousepad:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_Mousepad"));
				break;
			}

			Animation1D* animation1D = new Animation1D();
			animation = animation1D;

			// device
			animation1D->SetDevice((EChromaSDKDevice1DEnum)device);

			//frame count
			int frameCount;

			expectedSize = sizeof(int);
			read = fread(&frameCount, expectedSize, 1, stream);
			if (read != expectedRead)
			{
				UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading frame count!"));
				delete animation1D;
				std::fclose(stream);
				return -1;
			}
			else
			{
				vector<FChromaSDKColorFrame1D>& frames = animation1D->GetFrames();
				for (int index = 0; index < frameCount; ++index)
				{
					FChromaSDKColorFrame1D frame = FChromaSDKColorFrame1D();
					int maxLeds = GetMaxLeds((EChromaSDKDevice1DEnum)device);

					//duration
					float duration = 0.0f;
					expectedSize = sizeof(float);
					read = fread(&duration, expectedSize, 1, stream);
					if (read != expectedRead)
					{
						UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading duration!"));
						delete animation1D;
						std::fclose(stream);
						return -1;
					}
					else
					{
						if (duration < 0.1f)
						{
							duration = 0.1f;
						}
						frame.Duration = duration;

						// colors
						expectedSize = sizeof(int);
						for (int i = 0; i < maxLeds; ++i)
						{
							int color = 0;
							read = fread(&color, expectedSize, 1, stream);
							if (read != expectedRead)
							{
								UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading color!"));
								delete animation1D;
								std::fclose(stream);
								return -1;
							}
							else
							{
								frame.Colors.Add(FChromaSDKPluginModule::ToLinearColor(color));
							}
						}
						if (index == 0)
						{
							frames[0] = frame;
						}
						else
						{
							frames.push_back(frame);
						}
					}
				}
			}
		}
		break;
	case EChromaSDKDeviceTypeEnum::DE_2D:
		read = fread(&device, expectedSize, 1, stream);
		if (read != expectedRead)
		{
			UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Unexpected Device!"));
			std::fclose(stream);
			return -1;
		}
		else
		{
			switch ((EChromaSDKDevice2DEnum)device)
			{
			case EChromaSDKDevice2DEnum::DE_Keyboard:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_Keyboard"));
				break;
			case EChromaSDKDevice2DEnum::DE_Keypad:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_Keypad"));
				break;
			case EChromaSDKDevice2DEnum::DE_Mouse:
				//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Device: DE_Mouse"));
				break;
			}

			Animation2D* animation2D = new Animation2D();
			animation = animation2D;

			//device
			animation2D->SetDevice((EChromaSDKDevice2DEnum)device);

			//frame count
			int frameCount;

			expectedSize = sizeof(int);
			read = fread(&frameCount, expectedSize, 1, stream);
			if (read != expectedRead)
			{
				UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading frame count!"));
				delete animation2D;
				std::fclose(stream);
				return -1;
			}
			else
			{
				vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
				for (int index = 0; index < frameCount; ++index)
				{
					FChromaSDKColorFrame2D frame = FChromaSDKColorFrame2D();
					int maxRow = GetMaxRow((EChromaSDKDevice2DEnum)device);
					int maxColumn = GetMaxColumn((EChromaSDKDevice2DEnum)device);

					//duration
					float duration = 0.0f;
					expectedSize = sizeof(float);
					read = fread(&duration, expectedSize, 1, stream);
					if (read != expectedRead)
					{
						UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading duration!"));
						delete animation2D;
						std::fclose(stream);
						return -1;
					}
					else
					{
						if (duration < 0.1f)
						{
							duration = 0.1f;	
						}
						frame.Duration = duration;

						// colors
						expectedSize = sizeof(int);
						for (int i = 0; i < maxRow; ++i)
						{
							FChromaSDKColors row = FChromaSDKColors();
							for (int j = 0; j < maxColumn; ++j)
							{
								int color = 0;
								read = fread(&color, expectedSize, 1, stream);
								if (read != expectedRead)
								{
									UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Error detected reading color!"));
									delete animation2D;
									std::fclose(stream);
									return -1;
								}
								else
								{
									row.Colors.Add(FChromaSDKPluginModule::ToLinearColor(color));
								}
							}
							frame.Colors.Add(row);
						}
						if (index == 0)
						{
							frames[0] = frame;
						}
						else
						{
							frames.push_back(frame);
						}
					}
				}
			}
		}
		break;
	}

	std::fclose(stream);

	if (animation == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OpenAnimation: Animation is null! name=%s"), *FString(UTF8_TO_TCHAR(path)));
		return -1;
	}

	//UE_LOG(LogTemp, Log, TEXT("OpenAnimation: Loaded %s"), *FString(UTF8_TO_TCHAR(path)));
	animation->SetName(path);
	int id = _mAnimationId;
	_mAnimations[id] = animation;
	++_mAnimationId;
	_mAnimationMapID[path] = id;
	return id;
}

int FChromaSDKPluginModule::CloseAnimation(int animationId)
{
	try
	{
		if (_mAnimations.find(animationId) != _mAnimations.end())
		{
			AnimationBase* animation = _mAnimations[animationId];
			if (animation == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("CloseAnimation: Animation is null! id=%d"), animationId);
				return -1;
			}
			animation->Stop();
			string animationName = animation->GetName();
			if (_mAnimationMapID.find(animationName) != _mAnimationMapID.end())
			{
				_mAnimationMapID.erase(animationName);
			}
			delete _mAnimations[animationId];
			_mAnimations.erase(animationId);
			return animationId;
		}
	}
	catch (exception)
	{
		UE_LOG(LogTemp, Error, TEXT("CloseAnimation: Exception animationId=%d"), (int)animationId);
	}
	return -1;
}

int FChromaSDKPluginModule::CloseAnimationName(const char* path)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("CloseAnimationName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return -1;
	}
	return CloseAnimation(animationId);
}

int FChromaSDKPluginModule::GetAnimationIdFromInstance(AnimationBase* animation)
{
	if (animation == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GetAnimationIdFromInstance: Invalid animation!"));
		return -1;
	}
	for (int index = 0; index < _mAnimations.size(); ++index)
	{
		if (_mAnimations[index] == animation)
		{
			return index;
		}
	}
	return -1;
}

AnimationBase* FChromaSDKPluginModule::GetAnimationInstance(int animationId)
{
	if (_mAnimations.find(animationId) != _mAnimations.end())
	{
		return _mAnimations[animationId];
	}
	return nullptr;
}

int FChromaSDKPluginModule::GetAnimationFrameCount(int animationId)
{
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (nullptr == animation)
	{
		return -1;
	}
	return animation->GetFrameCount();
}

int FChromaSDKPluginModule::GetAnimationFrameCountName(const char* path)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("GetFrameCountName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return -1;
	}
	return GetAnimationFrameCount(animationId);
}

void FChromaSDKPluginModule::SetKeyColor(int animationId, int frameId, int rzkey, COLORREF color)
{
	StopAnimation(animationId);
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (nullptr == animation)
	{
		return;
	}
	if (animation->GetDeviceType() == EChromaSDKDeviceTypeEnum::DE_2D &&
		animation->GetDeviceId() == (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		Animation2D* animation2D = (Animation2D*)(animation);
		vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
		if (frameId >= 0 &&
			frameId < frames.size())
		{
			FChromaSDKColorFrame2D& frame = frames[frameId];
			frame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)] = ToLinearColor(color);
		}
	}
}

void FChromaSDKPluginModule::SetKeyColorName(const char* path, int frameId, int rzkey, COLORREF color)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("SetKeyColorName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return;
	}
	SetKeyColor(animationId, frameId, rzkey, color);
}

COLORREF FChromaSDKPluginModule::GetKeyColor(int animationId, int frameId, int rzkey)
{
	StopAnimation(animationId);
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (nullptr == animation)
	{
		return 0;
	}
	if (animation->GetDeviceType() == EChromaSDKDeviceTypeEnum::DE_2D &&
		animation->GetDeviceId() == (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		Animation2D* animation2D = (Animation2D*)(animation);
		vector<FChromaSDKColorFrame2D>& frames = animation2D->GetFrames();
		if (frameId >= 0 &&
			frameId < frames.size())
		{
			FChromaSDKColorFrame2D& frame = frames[frameId];
			FLinearColor& color = frame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)];
			return ToBGR(color);
		}
	}
	return 0;
}

COLORREF FChromaSDKPluginModule::GetKeyColorName(const char* path, int frameId, int rzkey)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("GetKeyColorName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return 0;
	}
	return GetKeyColor(animationId, frameId, rzkey);
}


void FChromaSDKPluginModule::CopyKeyColor(int sourceAnimationId, int targetAnimationId, int frameId, int rzkey)
{
	StopAnimation(targetAnimationId);
	AnimationBase* sourceAnimation = GetAnimationInstance(sourceAnimationId);
	if (nullptr == sourceAnimation)
	{
		return;
	}
	AnimationBase* targetAnimation = GetAnimationInstance(targetAnimationId);
	if (nullptr == targetAnimation)
	{
		return;
	}
	if (sourceAnimation->GetDeviceType() != EChromaSDKDeviceTypeEnum::DE_2D ||
		sourceAnimation->GetDeviceId() != (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		return;
	}
	if (targetAnimation->GetDeviceType() != EChromaSDKDeviceTypeEnum::DE_2D ||
		targetAnimation->GetDeviceId() != (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		return;
	}
	if (frameId < 0)
	{
		return;
	}
	Animation2D* sourceAnimation2D = (Animation2D*)(sourceAnimation);
	Animation2D* targetAnimation2D = (Animation2D*)(targetAnimation);
	vector<FChromaSDKColorFrame2D>& sourceFrames = sourceAnimation2D->GetFrames();
	vector<FChromaSDKColorFrame2D>& targetFrames = targetAnimation2D->GetFrames();
	if (sourceFrames.size() == 0)
	{
		return;
	}
	if (targetFrames.size() == 0)
	{
		return;
	}
	if (frameId < targetFrames.size())
	{
		FChromaSDKColorFrame2D& sourceFrame = sourceFrames[frameId % sourceFrames.size()];
		FChromaSDKColorFrame2D& targetFrame = targetFrames[frameId];
		targetFrame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)] = sourceFrame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)];
	}
}

void FChromaSDKPluginModule::CopyKeyColorName(const char* sourceAnimation, const char* targetAnimation, int frameId, int rzkey)
{
	int sourceAnimationId = GetAnimation(sourceAnimation);
	if (sourceAnimationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("CopyKeyColorName: Source Animation not found! %s"), *FString(UTF8_TO_TCHAR(sourceAnimation)));
		return;
	}
	int targetAnimationId = GetAnimation(targetAnimation);
	if (targetAnimationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("CopyKeyColorName: Target Animation not found! %s"), *FString(UTF8_TO_TCHAR(targetAnimation)));
		return;
	}
	CopyKeyColor(sourceAnimationId, targetAnimationId, frameId, rzkey);
}


void FChromaSDKPluginModule::CopyNonZeroKeyColor(int sourceAnimationId, int targetAnimationId, int frameId, int rzkey)
{
	StopAnimation(targetAnimationId);
	AnimationBase* sourceAnimation = GetAnimationInstance(sourceAnimationId);
	if (nullptr == sourceAnimation)
	{
		return;
	}
	AnimationBase* targetAnimation = GetAnimationInstance(targetAnimationId);
	if (nullptr == targetAnimation)
	{
		return;
	}
	if (sourceAnimation->GetDeviceType() != EChromaSDKDeviceTypeEnum::DE_2D ||
		sourceAnimation->GetDeviceId() != (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		return;
	}
	if (targetAnimation->GetDeviceType() != EChromaSDKDeviceTypeEnum::DE_2D ||
		targetAnimation->GetDeviceId() != (int)EChromaSDKDevice2DEnum::DE_Keyboard)
	{
		return;
	}
	if (frameId < 0)
	{
		return;
	}
	Animation2D* sourceAnimation2D = (Animation2D*)(sourceAnimation);
	Animation2D* targetAnimation2D = (Animation2D*)(targetAnimation);
	vector<FChromaSDKColorFrame2D>& sourceFrames = sourceAnimation2D->GetFrames();
	vector<FChromaSDKColorFrame2D>& targetFrames = targetAnimation2D->GetFrames();
	if (sourceFrames.size() == 0)
	{
		return;
	}
	if (targetFrames.size() == 0)
	{
		return;
	}
	if (frameId < targetFrames.size())
	{
		FChromaSDKColorFrame2D& sourceFrame = sourceFrames[frameId % sourceFrames.size()];
		FChromaSDKColorFrame2D& targetFrame = targetFrames[frameId];
		FLinearColor& color = sourceFrame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)];
		if (ToBGR(color) != 0)
		{
			targetFrame.Colors[HIBYTE(rzkey)].Colors[LOBYTE(rzkey)] = color;
		}
	}
}

void FChromaSDKPluginModule::CopyNonZeroKeyColorName(const char* sourceAnimation, const char* targetAnimation, int frameId, int rzkey)
{
	int sourceAnimationId = GetAnimation(sourceAnimation);
	if (sourceAnimationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("CopyNonZeroKeyColorName: Source Animation not found! %s"), *FString(UTF8_TO_TCHAR(sourceAnimation)));
		return;
	}
	int targetAnimationId = GetAnimation(targetAnimation);
	if (targetAnimationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("CopyNonZeroKeyColorName: Target Animation not found! %s"), *FString(UTF8_TO_TCHAR(targetAnimation)));
		return;
	}
	CopyNonZeroKeyColor(sourceAnimationId, targetAnimationId, frameId, rzkey);
}


void FChromaSDKPluginModule::LoadAnimation(int animationId)
{
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (nullptr == animation)
	{
		return;
	}
	animation->Load();
}

void FChromaSDKPluginModule::LoadAnimationName(const char* path)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return;
	}
	LoadAnimation(animationId);
}

void FChromaSDKPluginModule::UnloadAnimation(int animationId)
{
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (nullptr == animation)
	{
		return;
	}
	animation->Stop();
	animation->Unload();
}

void FChromaSDKPluginModule::UnloadAnimationName(const char* path)
{
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UnloadName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return;
	}
	UnloadAnimation(animationId);
}

int FChromaSDKPluginModule::GetAnimation(const char* path)
{
	for (std::map<string, int>::iterator it = _mAnimationMapID.begin(); it != _mAnimationMapID.end(); ++it)
	{
		const string& item = (*it).first;
		if (item.compare(path) == 0) {
			return (*it).second;
		}
	}
	return OpenAnimation(path);
}

const char* FChromaSDKPluginModule::GetAnimationName(int animationId)
{
	if (animationId < 0)
	{
		return "";
	}
	AnimationBase* animation = GetAnimationInstance(animationId);
	if (animation == nullptr)
	{
		return "";
	}
	return animation->GetName().c_str();
}

int FChromaSDKPluginModule::GetAnimationCount()
{
	return _mAnimationMapID.size();
}

int FChromaSDKPluginModule::GetAnimationId(int index)
{
	int i = 0;
	for (std::map<string, int>::iterator it = _mAnimationMapID.begin(); it != _mAnimationMapID.end(); ++it)
	{
		if (index == i)
		{
			return (*it).second;
		}
		++i;
	}
	return -1;
}

int FChromaSDKPluginModule::GetPlayingAnimationCount()
{
	if (ChromaThread::Instance() == nullptr)
	{
		return 0;
	}
	return ChromaThread::Instance()->GetAnimationCount();
}

int FChromaSDKPluginModule::GetPlayingAnimationId(int index)
{
	if (ChromaThread::Instance() == nullptr)
	{
		return -1;
	}
	return ChromaThread::Instance()->GetAnimationId(index);
}

void FChromaSDKPluginModule::PlayAnimation(int animationId, bool loop)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	if (_mAnimations.find(animationId) != _mAnimations.end())
	{
		AnimationBase* animation = _mAnimations[animationId];
		if (animation == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("PlayAnimation: Animation is null! id=%d"), animationId);
			return;
		}
		StopAnimationType(animation->GetDeviceTypeId(), animation->GetDeviceId());
		switch (animation->GetDeviceType())
		{
		case EChromaSDKDeviceTypeEnum::DE_1D:
			_mPlayMap1D[(EChromaSDKDevice1DEnum)animation->GetDeviceId()] = animationId;
			break;
		case EChromaSDKDeviceTypeEnum::DE_2D:
			_mPlayMap2D[(EChromaSDKDevice2DEnum)animation->GetDeviceId()] = animationId;
			break;
		}
		//UE_LOG(LogTemp, Log, TEXT("PlayAnimation: %s"), *FString(UTF8_TO_TCHAR(animation->GetName().c_str())));
		animation->Play(loop);
	}
}

void FChromaSDKPluginModule::PlayAnimationName(const char* path, bool loop)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayAnimationName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return;
	}
	PlayAnimation(animationId, loop);
}

void FChromaSDKPluginModule::StopAnimation(int animationId)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	if (_mAnimations.find(animationId) != _mAnimations.end())
	{
		AnimationBase* animation = _mAnimations[animationId];
		if (animation == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("StopAnimation: Animation is null! id=%d"), animationId);
			return;
		}
		//UE_LOG(LogTemp, Log, TEXT("StopAnimation: %s"), *FString(UTF8_TO_TCHAR(animation->GetName().c_str())));
		animation->Stop();
	}
}

void FChromaSDKPluginModule::StopAnimationName(const char* path)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("StopAnimationName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return;
	}
	StopAnimation(animationId);
}

void FChromaSDKPluginModule::StopAnimationType(int deviceType, int device)
{
	switch ((EChromaSDKDeviceTypeEnum)deviceType)
	{
	case EChromaSDKDeviceTypeEnum::DE_1D:
		{
			if (_mPlayMap1D.find((EChromaSDKDevice1DEnum)device) != _mPlayMap1D.end())
			{
				int prevAnimation = _mPlayMap1D[(EChromaSDKDevice1DEnum)device];
				if (prevAnimation != -1)
				{
					StopAnimation(prevAnimation);
					_mPlayMap1D[(EChromaSDKDevice1DEnum)device] = -1;
				}
			}
		}
		break;
	case EChromaSDKDeviceTypeEnum::DE_2D:
		{
			if (_mPlayMap2D.find((EChromaSDKDevice2DEnum)device) != _mPlayMap2D.end())
			{
				int prevAnimation = _mPlayMap2D[(EChromaSDKDevice2DEnum)device];
				if (prevAnimation != -1)
				{
					StopAnimation(prevAnimation);
					_mPlayMap2D[(EChromaSDKDevice2DEnum)device] = -1;
				}
			}
		}
		break;
	}
}

bool FChromaSDKPluginModule::IsAnimationPlaying(int animationId)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	if (_mAnimations.find(animationId) != _mAnimations.end())
	{
		AnimationBase* animation = _mAnimations[animationId];
		if (animation == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("IsAnimationPlaying: Animation is null! id=%d"), animationId);
			return false;
		}
		return animation->IsPlaying();
	}
	return false;
}

bool FChromaSDKPluginModule::IsAnimationPlayingName(const char* path)
{
	if (!IsInitialized())
	{
		ChromaSDKInit();
	}
	int animationId = GetAnimation(path);
	if (animationId < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("IsAnimationPlayingName: Animation not found! %s"), *FString(UTF8_TO_TCHAR(path)));
		return false;
	}
	return IsAnimationPlaying(animationId);
}

bool FChromaSDKPluginModule::IsAnimationPlayingType(int deviceType, int device)
{
	switch ((EChromaSDKDeviceTypeEnum)deviceType)
	{
	case EChromaSDKDeviceTypeEnum::DE_1D:
		{
			if (_mPlayMap1D.find((EChromaSDKDevice1DEnum)device) != _mPlayMap1D.end())
			{
				int prevAnimation = _mPlayMap1D[(EChromaSDKDevice1DEnum)device];
				if (prevAnimation != -1)
				{
					return IsAnimationPlaying(prevAnimation);
				}
			}
		}
		break;
	case EChromaSDKDeviceTypeEnum::DE_2D:
		{
			if (_mPlayMap2D.find((EChromaSDKDevice2DEnum)device) != _mPlayMap2D.end())
			{
				int prevAnimation = _mPlayMap2D[(EChromaSDKDevice2DEnum)device];
				if (prevAnimation != -1)
				{
					return IsAnimationPlaying(prevAnimation);
				}
			}
		}
		break;
	}
	return false;
}

#include "HideWindowsPlatformTypes.h"

#endif

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChromaSDKPluginModule, ChromaSDKPlugin)
