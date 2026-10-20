# LootInventory

버전 0.1

등록된 확률에 따라 아이템을 드롭하고, 획득·보관·사용까지 처리하는 Unreal Engine 5 런타임 플러그인입니다.

- 플러그인은 **데이터와 이벤트만** 제공합니다. UI, 입력, 드롭을 일으키는 시점, 아이템 효과 적용은 사용하는 프로젝트에서 만듭니다.
- 기획 데이터는 **엑셀 → CSV → DataTable**, 아이콘 같은 리소스는 **DataAsset**으로 관리하고 `ItemId`로 연결합니다.
- 아이템 타입·등급은 **GameplayTag**입니다. 플러그인을 수정하지 않고 프로젝트에서 타입을 추가합니다.

## 요구 사항

- Unreal Engine 5.4
- 지원 플랫폼: Win64
- 싱글플레이 기준 (인벤토리는 GameInstance 단위)

## 플러그인이 제공하는 것

| 구성 | 역할 |
|---|---|
| `FLIItemRowBase` | 아이템 행 기반 구조체 (`ItemId`, `DisplayName`, `Description`, `Rarity`, `MaxStack`, `GetItemType()`) |
| `FLIItemAssetsBase` | 아이템 리소스 기반 구조체 (`Icon`) |
| `ULIItemDataAssetBase` | 타입별 묶음 DataAsset (`ItemType`, `ItemTable`, `FindAssets()`) |
| `FLISampleItemRow` / `FLISampleItemAssets` / `ULISampleItemDataAsset` | 샘플 타입 (새 타입을 만들 때 복사해서 쓰는 템플릿) |
| `FLIDropRow` | 드롭 테이블 행 (`DropGroupId`, `ItemId`, `ChancePermyriad`, `MinCount`, `MaxCount`) |
| `ULIItemSubsystem` | `ItemId` → 행·리소스 조회, `GetItemDisplayInfo()` |
| `ULIInventorySubsystem` | 보유 아이템 관리, 변경 이벤트 |
| `ULILootSubsystem` | 드롭 계산과 픽업 생성 (`DropLoot()`) |
| `ALIItemPickup` / `BP_ItemPickup_Base` | 월드에 떨어지는 픽업 |
| `ULILootSettings` | Project Settings > Plugins > Loot Inventory |
| `EUW_LIItemViewer` | 아이템 데이터를 한 표로 보는 에디터 툴 |

## 빠른 시작

### 1. 설치

1. `LootInventory` 폴더를 프로젝트의 `Plugins/` 아래에 복사합니다.
   - 배포용 zip을 받았다면 압축을 `Plugins/`에 풉니다. UE 5.4용 빌드 파일이 들어 있어 바로 사용할 수 있습니다.
   - 다른 엔진 버전에서는 C++ 프로젝트에서 소스로 다시 빌드해야 합니다.
2. 에디터에서 **Edit > Plugins**로 활성화하고 에디터를 다시 시작합니다.
3. 프로젝트 `*.Build.cs`의 의존 모듈에 `"LootInventory"`를 추가합니다 (아이템 타입을 C++로 추가하기 때문에 필요).

### 2. 아이템 타입 만들기

타입 하나는 **행 구조체 + 리소스 구조체 + DataAsset 클래스** 한 세트입니다. 플러그인의 샘플 타입(`Data/LISampleItem.h/.cpp`)을 프로젝트로 복사해 이름과 태그만 바꾸는 것이 가장 빠릅니다.

```cpp
// LISampleItem.h
#include "Data/LIItemDataAsset.h"

// 행 (CSV 컬럼)
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLISampleItemRow : public FLIItemRowBase
{
    GENERATED_BODY()

public:
    virtual FGameplayTag GetItemType() const override;

    // 타입에 필요한 기획 데이터 필드를 여기에 추가 (CSV 컬럼이 됨)
};

// 리소스 (에디터에서 연결)
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLISampleItemAssets : public FLIItemAssetsBase
{
    GENERATED_BODY()

    // 타입에 필요한 리소스 필드를 여기에 추가 (예: Gameplay Effect, 메시)
};

// 타입 묶음 (Key: ItemId)
UCLASS()
class LOOTINVENTORY_API ULISampleItemDataAsset : public ULIItemDataAssetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TMap<int32, FLISampleItemAssets> Assets;

    virtual const FLIItemAssetsBase* FindAssets(int32 InItemId) const override { return Assets.Find(InItemId); }
};

// LISampleItem.cpp
FGameplayTag FLISampleItemRow::GetItemType() const
{
    return LIGameplayTags::Item_Type_Sample;
}
```

복사할 때 바꿀 것:

- **이름 (권장)**: 프로젝트 접두사 + `LI` (예: `FMGLI...Row`). 리플렉션 타입 이름은 모든 모듈에서 고유해야 해서, `LI`로 시작하면 나중에 플러그인에 같은 이름이 생길 때 충돌할 수 있습니다.
- **API 매크로**: `LOOTINVENTORY_API` → 프로젝트 모듈의 API 매크로
- **타입 태그**: 프로젝트에 태그 파일을 만들어 선언합니다 (아래 "타입 태그 파일").
- **리소스 필드**: 리소스 구조체에 필요한 필드를 추가합니다. 다른 모듈의 타입(예: `UGameplayEffect`)을 쓰면 그 모듈을 프로젝트 `*.Build.cs`에 추가합니다.

`FindAssets()`는 반드시 구현해야 합니다. 조회 서브시스템과 아이템 뷰어가 이 함수로 리소스를 찾습니다.

**타입 태그 파일**

새 타입과 등급 태그는 프로젝트에 네이티브 태그로 선언합니다. 플러그인의 `LIGameplayTags.h/.cpp`와 같은 형식입니다.

```cpp
// MGLIGameplayTags.h
#pragma once

#include "NativeGameplayTags.h"

namespace MGLIGameplayTags
{
    /** Item Type **/
    MYGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consume);

    /** Rarity **/
    MYGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Rarity_Common);
    MYGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Rarity_Rare);
}

// MGLIGameplayTags.cpp
#include "MGLIGameplayTags.h"

namespace MGLIGameplayTags
{
    /** Item Type **/
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consume, "LI.Item.Type.Consume");

    /** Rarity **/
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Common, "LI.Item.Rarity.Common");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Rare, "LI.Item.Rarity.Rare");
}
```

행 구조체의 `GetItemType()`은 이 태그를 반환합니다.

```cpp
FGameplayTag FMGLIConsumeItemRow::GetItemType() const
{
    return MGLIGameplayTags::Item_Type_Consume;
}
```

- 태그 문자열은 플러그인 트리(`LI.Item.Type.*`, `LI.Item.Rarity.*`) 아래에 둡니다. DataAsset의 `Item Type`과 행의 `Rarity` 선택 창에 이 트리 아래 태그만 표시됩니다.
- 플러그인에 이미 있는 태그(`LI.Item.Type`, `LI.Item.Type.Sample`, `LI.Item.Rarity`, `LI.Item.Rarity.None`)는 다시 선언하지 않습니다.
- `MYGAME_API`는 프로젝트 모듈의 API 매크로입니다. 프로젝트 `*.Build.cs`의 의존 모듈에 `GameplayTags`와 `LootInventory`가 있어야 합니다.
- 등급 태그는 CSV의 `Rarity` 컬럼에 문자열로 씁니다 (예: `LI.Item.Rarity.Rare`).

**에셋 만들기**

1. 엑셀에서 데이터를 작성하고 CSV로 저장합니다 (형식은 아래 "CSV 형식").
2. 콘텐츠 브라우저에 CSV를 가져와 Row Structure로 만든 행 구조체를 고릅니다 → DataTable.
3. 만든 DataAsset 클래스로 DataAsset을 만들고 `Item Type`(태그), `Item Table`(위 DataTable), `Assets`(ItemId별 리소스)를 채웁니다.

### 3. Project Settings 등록

**Project Settings > Plugins > Loot Inventory**

| 항목 | 내용 |
|---|---|
| `Item Data Assets` | 타입별 DataAsset을 모두 등록 |
| `Drop Table` | `FLIDropRow` 형식의 드롭 DataTable (선택 창에 이 형식만 표시됨) |
| `Pickup Class` | 드롭 시 생성할 픽업 (기본값 `BP_ItemPickup_Base`) |
| `Drop Scatter Radius` | 드롭이 흩어지는 반경 (기본 100) |

### 4. 쿡 폴더 등록 (필수)

쿡은 맵·BP에서 참조된 에셋만 패키징에 포함합니다. Project Settings에 등록한 에셋은 경로 문자열로만 저장되어 쿡 대상에 들어가지 않으므로, 에디터에서는 정상 동작해도 **패키징 게임에서는 데이터가 빠질 수 있습니다.**

**Project Settings > Packaging > Additional Asset Directories to Cook**에 다음을 추가하세요.

- 아이템 DataTable·DataAsset·드롭 테이블이 있는 폴더 (예: `/Game/MyItemData`)
- 픽업 BP 폴더 (기본값 사용 시 `/LootInventory/Pickup`, 자식 BP를 쓰면 그 폴더)

빠진 폴더가 있으면 에디터에서 플레이할 때 Output Log에 경고가 표시됩니다.

```
LogLIItem: Warning: '/Game/MyItemData' is not in the cook list. Add it to Project Settings > Packaging > Additional Asset Directories to Cook
```

### 5. 드롭 호출

플러그인은 드롭 시점을 정하지 않습니다. 적 사망, 상자 열기, 지역 트리거 등 원하는 시점에 `DropLoot`를 호출합니다.

```cpp
#include "Subsystems/LILootSubsystem.h"

if (ULILootSubsystem* LootSubsystem = GetGameInstance()->GetSubsystem<ULILootSubsystem>())
{
    LootSubsystem->DropLoot(GetWorld(), DropGroupId, GetActorLocation());
}
```

### 6. UI 만들기

UI는 프로젝트에서 만들고, 아래 함수와 이벤트만 사용합니다.

| 함수 / 이벤트 | 위치 | 용도 |
|---|---|---|
| `GetEntries()` | `ULIInventorySubsystem` | 보유 목록 (`SlotId`, `ItemId`, `Count`) |
| `OnInventoryChanged` | `ULIInventorySubsystem` | 목록이 바뀔 때 |
| `OnItemAdded(ItemId, Count)` | `ULIInventorySubsystem` | 획득할 때 |
| `RemoveItemFromSlot(SlotId)` | `ULIInventorySubsystem` | 해당 칸에서 1개 제거 |
| `AddItem(ItemId, Count)` | `ULIInventorySubsystem` | 직접 지급 |
| `GetItemDisplayInfo(ItemId, OutInfo)` | `ULIItemSubsystem` | 이름, 설명, 아이콘(로드됨), 등급, 타입, 최대 스택 |

아이템 효과는 타입을 만든 프로젝트 쪽에서 `FindItemAssets()`로 리소스를 꺼내 적용합니다.

## CSV 형식

CSV는 엑셀에서 작성한 데이터를 DataTable로 가져오기 위한 파일입니다. 가져올 때 고른 행 구조체에 맞춰 각 컬럼이 같은 이름의 필드에 들어갑니다.

| CSV | Row Structure | 만들어지는 DataTable |
|---|---|---|
| 아이템 테이블 | 프로젝트에서 만든 행 구조체 (예: `FLISampleItemRow`) | DataAsset의 `Item Table`에 연결 |
| 드롭 테이블 | `FLIDropRow` | Project Settings의 `Drop Table`에 등록 |

- 헤더(첫 줄)의 컬럼 이름은 **행 구조체의 필드 이름과 같아야** 합니다. 이름이 맞지 않는 컬럼은 무시되고 빠진 필드는 기본값이 되며, 가져오기 결과 창에 경고가 표시됩니다.
- 행 구조체에 필드를 추가하면 CSV에도 같은 이름의 컬럼을 추가합니다.
- 데이터를 고친 뒤에는 엑셀에서 CSV로 다시 저장하고, DataTable을 우클릭 → **Reimport**하면 반영됩니다. 연결된 DataAsset·Project Settings는 다시 설정할 필요가 없습니다.
- 데이터는 게임 시작 시 읽으므로, 플레이(PIE) 중에 Reimport한 내용은 다음 플레이부터 반영됩니다.

**아이템 테이블** (행 구조체 필드)

```
---,ItemId,DisplayName,Rarity,Description,MaxStack
1,1,Sample Item,LI.Item.Rarity.None,Sample description,10
2,2,Sample Item 2,LI.Item.Rarity.None,Sample description,1
```

**드롭 테이블**

```
---,DropGroupId,ItemId,ChancePermyriad,MinCount,MaxCount
Group1_1,1000000,1,10000,1,3
Group1_2,1000000,2,5000,1,1
```

- 첫 컬럼 `---`은 Row Name이며 **줄마다 고유해야 합니다.** 비어 있으면 그 줄이 빠지고, 드롭 테이블에서 이 컬럼이 없으면 `Duplicate row name` 에러가 납니다.
- 값 앞에 공백을 넣지 마세요. import에 실패합니다.
- 태그는 평문(`LI.Item.Rarity.None`)으로 씁니다.
- 컬럼 순서는 자유입니다 (Row Name만 첫 컬럼).
- 확률 `ChancePermyriad`는 만분률입니다 (10000 = 100%). 줄마다 독립적으로 계산합니다.
- 같은 `DropGroupId`의 줄이 그 그룹의 후보 전체입니다. 여러 타입을 섞을 수 있습니다.
- 한글 등 영문 외 문자가 있으면 엑셀에서 **CSV UTF-8** 형식으로 저장해야 글자가 깨지지 않습니다. 영문·숫자만 있으면 일반 CSV로 충분합니다.
- 설명이나 메모는 CSV가 아니라 엑셀의 별도 시트에 둡니다.

## ID 규칙

- `ItemId`는 타입이 달라도 고유해야 합니다 (0은 사용하지 않음). 중복되면 나중에 등록된 것이 덮어쓰고 경고 로그가 남습니다.
- 타입별로 `ItemId` 구간을 나누고, `DropGroupId`도 `ItemId`와 겹치지 않는 범위로 구별해서 쓰는 것을 권장합니다.

## 픽업 모양 바꾸기

1. `BP_ItemPickup_Base`(`/LootInventory/Pickup/`)를 부모로 자식 BP를 만듭니다.
2. `On Item Set` 이벤트에서 `Get Item Display Info`로 등급·아이콘을 받아 메시나 머티리얼을 바꿉니다.
3. Project Settings의 `Pickup Class`를 자식 BP로 바꾸고, 그 폴더를 쿡 목록에 추가합니다.

`Pickup Delay`(기본 0.5초)는 생성 후 획득이 가능해지기까지의 시간이며, 픽업 BP의 기본값에서 변경할 수 있습니다. 획득은 플레이어가 조종하는 폰만 할 수 있습니다.

## 아이템 뷰어 (에디터 툴)

`/LootInventory/Editor/EUW_LIItemViewer`를 우클릭 → **Run Editor Utility Widget**.

`Item Table`과 `Item Data Asset`을 지정하면 `ItemId` 기준으로 기획 데이터와 리소스를 한 표로 보여줍니다. 구조체에 필드를 추가해도 자동으로 표시됩니다.

## 디버그 콘솔 명령 (Shipping 빌드 제외)

| 명령 | 내용 |
|---|---|
| `LI.AddItem <ItemId> [Count]` | 아이템 지급 |
| `LI.PrintInventory` | 보유 목록 로그 출력 |
| `LI.RollDrop <DropGroupId>` | 드롭 계산 결과만 로그 출력 |
| `LI.DropLoot <DropGroupId>` | 플레이어 앞에 실제 드롭 |

## 경고 로그

시작할 때 설정 문제를 Output Log에 경고로 알려줍니다.

- `Item Data Assets`가 비어 있거나 로드 실패
- DataAsset의 `Item Table`이 비어 있음
- DataAsset의 `Item Type`과 행의 `GetItemType()`이 다름
- `ItemId` 중복
- 등록한 에셋 폴더가 쿡 목록에 없음 (에디터에서만)
- `Drop Table`이 비어 있음

드롭할 때(`DropLoot` 호출 시) `Pickup Class`가 비어 있으면 경고가 남습니다. 기본값이 있어 직접 비운 경우에만 해당합니다.

예시:

```
LogLIItem: Warning: No item data assets. Register them in Project Settings > Plugins > Loot Inventory > Item Data Assets
LogLIItem: Warning: Duplicate ItemId 1: 'DA_ItemB' overwrites 'DA_ItemA'
LogLILoot: Warning: No drop table. Set it in Project Settings > Plugins > Loot Inventory > Drop Table
```

정상이면 `LogLIItem: N items loaded`, `LogLILoot: N drop groups loaded`가 표시됩니다.

## 향후 작업

- 인벤토리 용량 제한 (고정 칸, 빈 칸 표시)
- 정렬
- 세이브·로드
- 여러 개 한 번에 사용
