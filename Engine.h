#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

struct FNameEntryHandle {
	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset) {};
	FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535) {};
	operator uint32_t() const { return (Block << 16 | Offset); }
};

struct FNameEntryHeader
{
	uint16_t bIsWide : 1;
	static constexpr uint32_t ProbeHashBits = 5;
	uint16_t LowercaseProbeHash : ProbeHashBits;
	uint16_t Len : 10;
};

struct FNameEntry {
	FNameEntryHeader Header;

	union
	{
		char AnsiName[1024];
		wchar_t	WideName[1024];
	};

	std::string String();

	inline int32_t GetId() const
	{
		return *(uint16_t*)&Header;
	}
};

struct FNameEntryAllocator
{
	unsigned char frwLock[0x8];
	int32_t CurrentBlock;
	int32_t CurrentByteCursor;
	uint8_t* Blocks[8192];

	inline int32_t NumBlocks() const
	{
		return CurrentBlock + 1;
	}
	inline FNameEntry* GetById(int32_t key) const
	{
		int block = key >> 16;
		int offset = (uint16_t)key;

		if (!IsValidIndex(key, block, offset))
			return reinterpret_cast<FNameEntry*>(Blocks[0] + 0); // "None"

		return reinterpret_cast<FNameEntry*>(Blocks[block] + ((uint64_t)offset * 0x02));
	}
	inline bool IsValidIndex(int32_t key) const
	{
		uint32_t block = key >> 16;
		uint16_t offset = key;
		return IsValidIndex(key, block, offset);
	}
	inline bool IsValidIndex(int32_t key, uint32_t block, uint16_t offset) const
	{
		return (key >= 0 && block < NumBlocks() && offset * 0x02 < 0x1FFFE);
	}
};

struct FNamePool
{
	FNameEntryAllocator Allocator;
	uint32_t AnsiCount;
	uint32_t WideCount;
};

struct FName {
	uint32_t Index;
	uint32_t Number;

	FName() :
		Index(0),
		Number(0)
	{ }

	FName(int32_t i) :
		Index(i),
		Number(0)
	{ }
	std::string GetName();
};

struct UObject {
	void** VFTable;
	uint32_t ObjectFlags;
	uint32_t InternalIndex;
	struct UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	std::string GetName();
	std::string GetFullName();
	bool IsA(void* cmp);
	void ProcessEvent(void* fn, void* parms);
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
struct UField : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Struct
// Size: 0xb0 (Inherited: 0x30)
struct UStruct : UField {
	char pad_30[0x10]; // 0x30(0x10)
	UStruct* SuperStruct; // 0x40(0x8)
	char UnknownData_48[0x68]; // 0x48(0x80)
};

// Class CoreUObject.Function
// Size: 0xe0 (Inherited: 0xb0)
struct UFunction : UStruct {
	char pad_B0[0x30]; // 0xb0(0x30)
};

// Class CoreUObject.Class
// Size: 0x230 (Inherited: 0xb0)
struct UClass : UStruct {
	char UnknownData_B0[0x180]; // 0xb0(0x180)
};

struct TUObjectArray {
	BYTE** Objects;
	BYTE* PreAllocatedObjects;
	uint32_t MaxElements;
	uint32_t NumElements;
	uint32_t MaxChunks;
	uint32_t NumChunks;
	UObject* GetObjectPtr(uint32_t id) const;
	UObject* FindObject(const char* name) const;
};

template<class T>
struct TArray
{
public:
	inline T& operator[](int i)
	{
		return Data[i];
	};
private:
	T* Data;
	int32_t Count;
	int32_t Max;
};

// Class Tiger.TigerCharCustomizationComponent
struct UTigerCharCustomizationComponent : UObject {
	// Hey ;)
};

// Class Engine.Pawn 
struct APawn {
	char pad_0000[0x818]; // 0x0(0x818)
	class UTigerCharCustomizationComponent* CharacterCustomizationComponent; // 0x818(0x08)
};

// Class Engine.PlayerController
struct APlayerController {
	char pad_0000[0x2a8]; // 0x0 (0x2a8)
	class APawn* AcknowledgedPawn; // 0x2a8 (0x08)
};

// Class Engine.Player
struct UPlayer {
	char pad_0000[0x30]; // 0x0 (0x30)
	class APlayerController* PlayerController; // 0x30(0x08)
};

// Class Engine.GameInstance
struct UGameInstance {
	char pad_0000[0x38]; // 0x0 (0x38)
	TArray<class UPlayer*> LocalPlayers; // 0x38(0x10)
};

// Class Engine.World 
struct UWorld {
	char pad_0000[0x180]; // 0x0(0x180)
	class UGameInstance* OwningGameInstance; // 0x180(0x08)
};

// Enum Tiger.ETigerGender
enum class ETigerGender {
	Female = 0,
	Male = 1,
	Count = 2,
	ETigerGender_MAX = 3
};

struct FTigerPersistentOutfitSetup {
	char PAD69420[0x28]; // 0w0 (0x28) TSoftClassPtr<UObject> OutfitConfig; lazy 
	int32_t OutfitId; // 0x28(0x04)
	int32_t HoodieIndex; // 0x2c(0x04)
};

struct FTigerCharacterAppearance {
	FTigerPersistentOutfitSetup PersistentAppearance; // 0x00(0x30)
	int32_t HeadId; // 0x30(0x04)
	int32_t SkinColorIndex; // 0x34(0x04)
	int32_t HairId; // 0x38(0x04)
	int32_t HairColorId; // 0x3c(0x04)
	int32_t EyeColorId; // 0x40(0x04)
	int32_t TattooId; // 0x44(0x04)
	int32_t FacePaintId; // 0x48(0x04)
	int32_t HeadgearId; // 0x4c(0x04)
	int32_t EyewearId; // 0x50(0x04)
	int32_t PiercingSetId; // 0x54(0x04)
	int32_t EyebrowShapeId; // 0x58(0x04)
	int32_t EyebrowColorId; // 0x5c(0x04)
	int32_t BeardId; // 0x60(0x04)
	int32_t BeardColorId; // 0x64(0x04)
	ETigerGender BodyType; // 0x68(0x01)
	char pad_69[0x7]; // 0x69(0x07)
	TArray<int32_t> EmoteIds; // 0x70(0x10)
};

bool EngineInit();
extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;
extern UWorld* WRLD;
extern UObject* SetAppearanceUFunction;
extern void* ProcessEventAddress;
