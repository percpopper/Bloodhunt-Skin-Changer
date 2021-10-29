#include "engine.h"
#include "Util.h"
#include <Psapi.h>

std::string FNameEntry::String()
{
	if (Header.bIsWide) { return std::string(); }
	return { AnsiName, Header.Len };
}

std::string FName::GetName()
{
	auto entry = NamePoolData->Allocator.GetById(Index);
	auto name = entry->String();
	if (Number > 0)
	{
		name += '_' + std::to_string(Number);
	}
	auto pos = name.rfind('/');
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1);
	}
	return name;
}

std::string UObject::GetName()
{
	return NamePrivate.GetName();
}

std::string UObject::GetFullName()
{
	std::string name;
	for (auto outer = OuterPrivate; outer; outer = outer->OuterPrivate) { name = outer->GetName() + "." + name; }
	name = ClassPrivate->GetName() + " " + name + this->GetName();
	return name;
}

bool UObject::IsA(void* cmp)
{
	for (auto super = ClassPrivate; super; super = static_cast<UClass*>(super->SuperStruct)) { if (super == cmp) { return true; } }
	return false;
}

UObject* TUObjectArray::GetObjectPtr(uint32_t id) const
{
	if (id >= NumElements) return nullptr;
	uint64_t chunkIndex = id / 65536;
	if (chunkIndex >= NumChunks) return nullptr;
	auto chunk = Objects[chunkIndex];
	if (!chunk) return nullptr;
	uint32_t withinChunkIndex = id % 65536 * 24;
	auto item = *reinterpret_cast<UObject**>(chunk + withinChunkIndex);
	return item;
}

UObject* TUObjectArray::FindObject(const char* name) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		auto object = GetObjectPtr(i);
		if (object && object->GetFullName() == name) { return object; }
	}
	return nullptr;
}

void UObject::ProcessEvent(void* UFunction, void* Params)
{
	auto vtable = *reinterpret_cast<void***>(this);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[67])(this, UFunction, Params);
}

FNamePool* NamePoolData = nullptr;
TUObjectArray* ObjObjects = nullptr;
UWorld* WRLD = nullptr;
UObject* SetAppearanceUFunction = nullptr;
void* ProcessEventAddress = nullptr;

bool EngineInit()
{
	auto main = GetModuleHandleA(nullptr);

	static byte objSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };
	ObjObjects = reinterpret_cast<decltype(ObjObjects)>(FindPointer(main, objSig, sizeof(objSig), 0));
	if (!ObjObjects) return false;

	static byte poolSig[] = { 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xC6, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0F, 0x10, 0x03, 0x4C, 0x8D, 0x44, 0x24, 0x20, 0x48, 0x8B, 0xC8 };
	NamePoolData = reinterpret_cast<decltype(NamePoolData)>(FindPointer(main, poolSig, sizeof(poolSig), 0));
	if (!NamePoolData) return false;

	static byte worldSig[] = { 0x48, 0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xDB, 0x74, 0x3B };
	WRLD = reinterpret_cast<decltype(WRLD)>(FindPointer(main, worldSig, sizeof(worldSig), 0));
	if (!WRLD) return false;

	MODULEINFO info;
	if (K32GetModuleInformation(GetCurrentProcess(), main, &info, sizeof(MODULEINFO))) {
		auto base = static_cast<byte*>(info.lpBaseOfDll);
		static byte PESig[] = { 0x40, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x6C, 0x24, 0x00, 0x48, 0x89, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x33, 0xC5, 0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x41, 0x0C, 0x45, 0x33, 0xF6, 0x3B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x8B, 0xF8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xE1, 0x41, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x7D, 0x2A, 0x99, 0x41, 0x23, 0xD0, 0x03, 0xC2, 0x8B, 0xC8, 0x41, 0x23, 0xC0 };
		ProcessEventAddress = FindSignature(base, base + info.SizeOfImage - 1, PESig, sizeof(PESig));
		if (!ProcessEventAddress) return false;
	}

	SetAppearanceUFunction = ObjObjects->FindObject("Function Tiger.TigerCharCustomizationComponent.SetAppearance");

	return true;
}
