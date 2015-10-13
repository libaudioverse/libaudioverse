//This is undocumented  COM magic.  Including this here should be sufficient for most of the library, but maybe not.
#include <initguid.h>
#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/com.hpp>
#include <audio_io/private/logging.hpp>
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

namespace audio_io {
namespace implementation {

WasapiOutputDeviceFactory::WasapiOutputDeviceFactory() {
	IMMDeviceEnumerator* enumerator_raw = nullptr;
	auto res = APARTMENTCALL(CoCreateInstance, CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator_raw);
	if(IS_ERROR(res)) {
		logCritical("Could not create device enumerator in factory constructor. Error %i\n", res);
		throw AudioIOError("Could not create Wasapi device enumerator");
	}
	enumerator = wrapComPointer(enumerator_raw);
	rescan();
}

WasapiOutputDeviceFactory::~WasapiOutputDeviceFactory() {
	//Release is handled by the smart pointers.
}

std::vector<std::string> WasapiOutputDeviceFactory::getOutputNames() {
	return names;
}

std::vector<int> WasapiOutputDeviceFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<OutputDevice> WasapiOutputDeviceFactory::createDevice(std::function<void(float*, int)> callback, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) {
	HRESULT res;
	IMMDevice* dev = nullptr;
	if(index == -1) res = APARTMENTCALL(enumerator->GetDefaultAudioEndpoint, eRender, eMultimedia, &dev);
	else res = APARTMENTCALL(enumerator->GetDevice, ids_to_id_strings[index].c_str(), &dev);
	if(IS_ERROR(res)) {
		logCritical("Could not create IMMDevice instance.  COM error %i", res);
		throw AudioIOError("Could not create IMMDeviceInstance");
	}
	auto ret = std::make_shared<WasapiOutputDevice>(callback, wrapComPointer(dev), blockSize, channels, sr, minLatency, startLatency, maxLatency);
	created_devices.push_back(ret);
	return ret;
}

unsigned int WasapiOutputDeviceFactory::getOutputCount() {
	return names.size();
}

std::string WasapiOutputDeviceFactory::getName() {
	return "Wasapi";
}

void WasapiOutputDeviceFactory::rescan() {
	names.clear();
	max_channels.clear();
	ids_to_id_strings.clear();
	IMMDeviceCollection *collection_raw = nullptr;
	auto res = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection_raw);
	if(IS_ERROR(res)) {
		logCritical("Failed to get IMMDeviceCollection.  COM error %i", res);
		throw AudioIOError("Could not create IMMDeviceCollection.");
	}
	auto collection = wrapComPointer(collection_raw);
	UINT count;
	res = collection->GetCount(&count);
	if(IS_ERROR(res)) {
		logCritical("Failure to retrieve device count. Error %i", res);
		throw AudioIOError("Could not determine device count.");
	}
	for(UINT i = 0; i < count; i++) {
		std::wstring identifierString;
		int channels;
		IMMDevice *device_raw = nullptr;
		res = collection->Item(i, &device_raw);
		if(IS_ERROR(res)) {
			logCritical("Couldn't obtain device info for device %i.  Error %i", i, res);
			throw AudioIOError("Failure to retrieve device information.");
		}
		auto device = wrapComPointer(device_raw);
		//We have to open the property store and use it to get the informationh.
		IPropertyStore *properties_raw = nullptr;
		res = device->OpenPropertyStore(STGM_READ, &properties_raw);
		if(IS_ERROR(res)) {
			logCritical("Couldn't open property store for device %i. Error %i", i, res);
			throw AudioIOError("Cannot open property store.");
		}
		auto properties = wrapComPointer(properties_raw);
		PROPVARIANT prop;
		res = properties->GetValue(PKEY_AudioEngine_DeviceFormat, &prop);
		if(IS_ERROR(res)) {
			logCritical("Could not obtain device format for device %i. Error: %i", i, res);
			throw AudioIOError("Couldn't get device format.");
		}
		WAVEFORMATEX *format = (WAVEFORMATEX*)prop.blob.pBlobData;
		channels = format->nChannels;
		//Easy string is the identifier.
		LPWSTR identifier = nullptr;
		res = device->GetId(&identifier);
		if(IS_ERROR(res)) {
			logCritical("Could not retreive identifier for device %i. Error %i", i, res);
			throw AudioIOError("Failure to retrieve device identifier");
		}
		identifierString = std::wstring(identifier);
		CoTaskMemFree(identifier);
		res = properties->GetValue(PKEY_DeviceInterface_FriendlyName, &prop);
		if(IS_ERROR(res)) {
			logCritical("Failure to obtain friendly name for device %i.  Error %i", i, res);
			throw AudioIOError("Couldn't get friendly name");
		}
		LPWSTR rawname = prop.pwszVal;
		char* utf8Name = nullptr;
		int length = WideCharToMultiByte(CP_UTF8, 0, rawname, -1, NULL, 0, NULL, NULL);
		//MSDN is unclear if we get a null, so we make sure to add one.
		utf8Name = new char[length+1]();
		length = WideCharToMultiByte(CP_UTF8, 0, rawname, -1, utf8Name, length, NULL, NULL);
		utf8Name[length] = '\0';
		auto name = std::string(utf8Name);
		delete[] utf8Name;
		this->max_channels.push_back(channels);
		names.push_back(name);
		ids_to_id_strings[i] = identifierString;
	}
}

OutputDeviceFactory* createWasapiOutputDeviceFactory() {
	//In order to determine if we have Wasapi, we attempt to open and close the default output device without error.
	try {
		//Let's not create multiple instances if we can avoid it. Wasapi is buggy, and this might help.
		{
			SingleThreadedApartment sta;
			IMMDeviceEnumerator* enumerator_raw = nullptr;
			IMMDevice *default_device_raw = nullptr;
			IAudioClient* client_raw = nullptr;
			auto res = sta.callInApartment(CoCreateInstance, CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator_raw);
			if(IS_ERROR(res)) {
				logDebug("Couldn't create IMMDeviceEnumerator.  COM error %i", (int)res);
				return nullptr;
			}
			auto enumerator = wrapComPointer(enumerator_raw);
			//Attempt to get the default device.
			res = APARTMENTCALL(enumerator->GetDefaultAudioEndpoint, eRender, eMultimedia, &default_device_raw);
			if(IS_ERROR(res)) {
				logDebug("Couldn't initialize IMMDevice for the default audio device.  COM error %i", (int)res);
				return nullptr;
			}
			auto default_device = wrapComPointer(default_device_raw);
			res = APARTMENTCALL(default_device->Activate, IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client_raw);
			if(IS_ERROR(res)) {
				logDebug("Couldn't activate default Wasapi audio device. COM error %i", (int)res);
				return nullptr;
			}
			auto client = wrapComPointer(client_raw);
			//We now have an IAudioClient.  We can attempt to initialize it with the default mixer format in shared mode, which is supposed to always be accepted.
			WAVEFORMATEX *format = nullptr;
			res = APARTMENTCALL(client->GetMixFormat, &format);
			if(IS_ERROR(res)) {
				logDebug("Couldn't get mix format for default device.  COM error %i", (int)res);
				return nullptr;
			}
			//We don't request a specific buffer length, we just want to know if we can open and otherwise don't care.
			res = APARTMENTCALL(client->Initialize, AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, format, NULL);
			if(IS_ERROR(res)) {
				logDebug("Couldn't initialize default audio device. COM error %i", (int)res);
				return nullptr;
			}
		} //End the test scope.
		return new WasapiOutputDeviceFactory();
	}
	catch(...) {
		logDebug("Attempt to initialize Wasapi failed. Falling back to next preferred factory.");
		return nullptr;
	}
	return nullptr;
}

}
}