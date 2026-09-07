#include "game_player_interaction.h"
#include "game_player.h"
#include "crop_plot.h"
#include "shop.h"
#include "sellBox.h"
#include "input_keyboard.h"
#include "upgrade.h"
#include "fence.h"
#include "Audio.h"
#include "sparkle.h"

static constexpr ItemType g_CropHarvestItem[CropType_MAX] = {
	ItemType_Carrot,    // CropType_Carrot
	ItemType_Wheat,     // CropType_Wheat
	ItemType_Lettuce,   // CropType_Lettuce
	ItemType_Corn,      // CropType_Corn
	ItemType_Blueberry, // CropType_Blueberry
};

static constexpr ItemType g_CropHarvestItemGold[CropType_MAX] = {
	ItemType_CarrotGold, // CropType_Carrot
	ItemType_WheatGold,
	ItemType_LettuceGold,
	ItemType_CornGold,
	ItemType_BlueberryGold,
};

static constexpr ItemType g_SeedForCrop[CropType_MAX] = {
	ItemType_CarrotSeed,     // CropType_Carrot
	ItemType_WheatSeed,      // CropType_Wheat
	ItemType_LettuceSeed,    // CropType_Lettuce
	ItemType_CornSeed,       // CropType_Corn
	ItemType_BlueberrySeed,  // CropType_Blueberry
};

static constexpr int g_CropHarvestYield[CropType_MAX] = {
	1, // CropType_Carrot
	1, // CropType_Wheat
	1, // CropType_Lettuce
	3, // CropType_Corn
	5, // CropType_Blueberry
};

static bool CropForSeed(ItemType item, CropType& outType)
{
	for (int type = 0; type < CropType_MAX; type++)
	{
		if (g_SeedForCrop[type] == item)
		{
			outType = (CropType)type;
			return true;
		}
	}
	return false;
}

static constexpr float BASE_PLANT_TIME = 0.75f;
static constexpr float BASE_WATER_TIME = 0.75f;
static constexpr float BASE_HARVEST_DISPLAY_TIME = 0.75f;

//player planting times 
static float plantingTimer = 0.0f;
static float PLANT_TIME = BASE_PLANT_TIME; //time it takes to plant a crop
static int currentPlotIndex = -1; //index of the current plot being planted
static float wateringTimer = 0.0f;
static float WATER_TIME = BASE_WATER_TIME;

//Harvesting information 
static float HARVEST_DISPLAY_TIME = BASE_HARVEST_DISPLAY_TIME;
static bool isHarvesting = false;
static float harvestTimer = 0.0f;
static CropRank pendingHarvestRank = CropRank_Normal;
static CropType pendingHarvestType = CropType_Carrot;

static float fenceTimer = 0.0f;
static int currentFenceSlotIndex = -1;

static int g_AudioID_Watering = -1;

void PlayerInteraction_Initialize()
{
	g_AudioID_Watering = LoadAudio("assets/SFX/Water.wav");
}

void PlayerInteraction_Finalize()
{
	UnloadAudio(g_AudioID_Watering);
}

void PlayerInteraction_UpdateHarvestTimer(float delta_time)
{
	if (!isHarvesting) return;

	harvestTimer += delta_time;
	if (harvestTimer >= HARVEST_DISPLAY_TIME)
	{
		ItemType harvestedItem = (pendingHarvestRank == CropRank_Watered)
			? g_CropHarvestItemGold[pendingHarvestType]
			: g_CropHarvestItem[pendingHarvestType];
		Inventory_AddItem(harvestedItem, g_CropHarvestYield[pendingHarvestType]);
		isHarvesting = false;
		Player_ChangeState(Idle);
	}
}

static void WaterPlotIfNeeded(int plotIndex)
{
	if (plotIndex < 0) return;

	CropPlot* p = CropPlot_Get(plotIndex);
	if (!p || !p->occupied) return;
	if (CropGet(p->cropIndex).growthStage == CropGrowth_Ready) return; // nothing left to water
	if (Crop_GetRank(p->cropIndex) == CropRank_Watered) return;        // already watered

	Crop_Water(p->cropIndex);
}

void PlayerInteraction_HandleUse(float delta_time)
{
	if (Shop_IsPlayerNear())
	{
		if (InputKeyboard_IsTrigger(KK_E))
		{
			Shop_Open();
		}
	}
	else if (SellBox_IsPlayerNear())
	{
		if (InputKeyboard_IsTrigger(KK_E))
		{
			SellBox_TrySell();
		}
	}
	else
	{
		int overlappingPlot = CropPlot_GetPlayerPlot();
		int overlappingFenceSlot = Fence_GetPlayerSlot();
		bool wantsFence = overlappingPlot == -1 && overlappingFenceSlot != -1 &&
			!Fence_IsBuilt(overlappingFenceSlot) &&
			Inventory_GetSlotItem(Inventory_GetSelectedSlot()) == ItemType_Fence;

		if (wantsFence)
		{
			if (currentFenceSlotIndex != overlappingFenceSlot)
			{
				fenceTimer = 0.0f;
				currentFenceSlotIndex = overlappingFenceSlot;
			}

			Player_ChangeState(Planting);
			fenceTimer += delta_time;

			if (fenceTimer >= PLANT_TIME)
			{
				if (Inventory_RemoveItem(ItemType_Fence, 1))
				{
					Fence_Build(currentFenceSlotIndex);
				}
				fenceTimer = 0.0f;
				currentFenceSlotIndex = -1;
				Player_ChangeState(Idle);
			}
			return;
		}
		currentFenceSlotIndex = -1;
		fenceTimer = 0.0f;


		if (currentPlotIndex == -1)
		{
			currentPlotIndex = overlappingPlot;
		}
		else if (overlappingPlot != currentPlotIndex)
		{
			plantingTimer = 0.0f;
			wateringTimer = 0.0f;
			currentPlotIndex = -1;
		}

		CropPlot* plot = CropPlot_Get(currentPlotIndex);
		bool validPlot = currentPlotIndex != -1 && plot->occupied;
		bool readyToHarvest = validPlot && CropGet(plot->cropIndex).growthStage == CropGrowth_Ready;
		bool hasPailSelected = Inventory_GetSlotItem(Inventory_GetSelectedSlot()) == ItemType_WaterPail;
		bool needsWater = validPlot && !readyToHarvest && Crop_GetRank(plot->cropIndex) != CropRank_Watered && hasPailSelected;
		CropType selectedCropType = CropType_Carrot;

		if (readyToHarvest)
		{
			bool wasWatered = Crop_GetRank(plot->cropIndex) == CropRank_Watered;
			pendingHarvestRank = wasWatered ? CropRank_Watered : CropRank_Normal;
			pendingHarvestType = CropGet(plot->cropIndex).type;

			CropPlot_Harvest(currentPlotIndex);
			isHarvesting = true;
			harvestTimer = 0.0f;
			currentPlotIndex = -1;
			Player_ChangeState(Harvesting);

			if (wasWatered)
			{
				DirectX::XMFLOAT2 popupCenter = GamePlayer_GetHarvestPopupCenter();
				Sparkle_Emit(SparkleType_Gold, popupCenter.x, popupCenter.y);
			}
		}
		else if (needsWater)
		{
			Player_ChangeState(Watering);
			if (wateringTimer<=0.0f)
			{
				PlayAudio(g_AudioID_Watering);
			}
			wateringTimer += delta_time;

			if (wateringTimer >= WATER_TIME)
			{
				Crop_Water(plot->cropIndex);
				StopAudio(g_AudioID_Watering);

				if (Upgrade_IsWaterAreaUnlocked())
				{
					WaterPlotIfNeeded(CropPlot_GetIndexAt(plot->x + PLOT_SIZE, plot->y));
					WaterPlotIfNeeded(CropPlot_GetIndexAt(plot->x, plot->y + PLOT_SIZE));
					WaterPlotIfNeeded(CropPlot_GetIndexAt(plot->x + PLOT_SIZE, plot->y + PLOT_SIZE));
				}

				wateringTimer = 0.0f;
				currentPlotIndex = -1;
				Player_ChangeState(Idle);
			}
		}
		else if (currentPlotIndex != -1 && !plot->occupied && Inventory_GetSlotItem(Inventory_GetSelectedSlot()) == ItemType_Scarecrow)
		{
			Player_ChangeState(Planting);
			plantingTimer += delta_time;

			if (plantingTimer >= PLANT_TIME)
			{
				if (Inventory_RemoveItem(ItemType_Scarecrow, 1))
				{
					CropPlot_PlaceScarecrow(currentPlotIndex);
				}
				plantingTimer = 0.0f;
				currentPlotIndex = -1;
				Player_ChangeState(Idle);
			}
		}
		else if (currentPlotIndex != -1 && !plot->occupied && CropForSeed(Inventory_GetSlotItem(Inventory_GetSelectedSlot()), selectedCropType))
		{
			Player_ChangeState(Planting);
			plantingTimer += delta_time;

			if (plantingTimer >= PLANT_TIME)
			{
				ItemType selectedSeed = Inventory_GetSlotItem(Inventory_GetSelectedSlot());
				if (Inventory_RemoveItem(selectedSeed, 1))
				{
					CropPlot_Plant(currentPlotIndex, selectedCropType);
				}
				plantingTimer = 0.0f;
				currentPlotIndex = -1;
				Player_ChangeState(Idle);
			}
		}
		else
		{
			plantingTimer = 0.0f;
			wateringTimer = 0.0f;
			Player_ChangeState(Idle);
		}
	}
}

bool PlayerInteraction_IsHarvesting()
{
	return isHarvesting;
}

ItemType PlayerInteraction_GetHarvestPopupItem()
{
	return (pendingHarvestRank == CropRank_Watered)
		? g_CropHarvestItemGold[pendingHarvestType]
		: g_CropHarvestItem[pendingHarvestType];
}

bool PlayerInteraction_IsFilling()
{
	if (!InputKeyboard_IsPress(KK_E)) return false; // hide immediately if released early

	return wateringTimer > 0.0f || plantingTimer > 0.0f || fenceTimer > 0.0f;
}

float PlayerInteraction_GetFillProgress()
{
	if (wateringTimer > 0.0f) return wateringTimer / WATER_TIME;
	if (plantingTimer > 0.0f) return plantingTimer / PLANT_TIME;
	if (fenceTimer > 0.0f) return fenceTimer / PLANT_TIME;
	return 0.0f;
}

bool PlayerInteraction_GetHoverTile(float& outX, float& outY)
{
	if (PlayerInteraction_IsFilling()) return false; // hide the instant the player commits to an action

	int fenceSlot = Fence_GetPlayerSlot();
	if (fenceSlot != -1)
	{
		return Fence_GetSlotPosition(fenceSlot, outX, outY);
	}

	int plotIndex = CropPlot_GetPlayerPlot();
	if (plotIndex != -1)
	{
		CropPlot* plot = CropPlot_Get(plotIndex);
		if (plot != nullptr)
		{
			outX = plot->x;
			outY = plot->y;
			return true;
		}
	}
	return false;
}

void PlayerInteraction_ApplyInteractionSpeedBoost()
{
	constexpr float SPEED_MULTIPLIER = 0.7f; // 30% faster
	PLANT_TIME *= SPEED_MULTIPLIER;
	WATER_TIME *= SPEED_MULTIPLIER;
	HARVEST_DISPLAY_TIME *= SPEED_MULTIPLIER;
}

void PlayerInteraction_ResetUpgrades()
{
	PLANT_TIME = BASE_PLANT_TIME;
	WATER_TIME = BASE_WATER_TIME;
	HARVEST_DISPLAY_TIME = BASE_HARVEST_DISPLAY_TIME;
}

void PlayerInteraction_CheckEarlyRelease()
{
	if (InputKeyboard_IsRelease(KK_E))
	{
		StopAudio(g_AudioID_Watering);
	}
}
