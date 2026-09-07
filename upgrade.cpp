/*----------------------------------------------------
 HEADER COMMENT

 upgrade.cpp
 Post-level upgrade choice menu
                                   Author: Carina Chao
                                   Date: 2026/08/27
 ----------------------------------------------------*/
#include "upgrade.h"
#include "game_player.h"
#include "inventory.h"
#include "texture.h"
#include "sprite.h"
#include "font.h"
#include "input_keyboard.h"
#include "config.h"
#include "game_player_interaction.h"

enum UpgradeType
{
    UpgradeType_MoveSpeed,
    UpgradeType_WaterArea,
    UpgradeType_InteractionSpeed,
    UpgradeType_MAX
};

struct UpgradeChoiceSet
{
    LevelType afterLevel;      // which cleared level triggers this choice
    UpgradeType options[2];
};

// Add one entry per level that should offer an upgrade after being cleared.
static constexpr UpgradeChoiceSet g_ChoiceSets[] =
{
    { Level1, { UpgradeType_MoveSpeed, UpgradeType_WaterArea } },
    { Level2, { UpgradeType_MoveSpeed, UpgradeType_InteractionSpeed } },
};
static constexpr int g_ChoiceSetCount = sizeof(g_ChoiceSets) / sizeof(g_ChoiceSets[0]);

static constexpr float SPEED_BOOST_AMOUNT = 120.0f;
static constexpr int MAX_UPGRADE_OPTIONS = 3;

static int g_SpeedIconTextureID = TEXTURE_INVALID_ID;
static int g_HandsIconTextureID = TEXTURE_INVALID_ID;

static bool g_ChoiceActive = false;
static int g_SelectedOption = 0;
static UpgradeType g_CurrentOptions[MAX_UPGRADE_OPTIONS];
static int g_CurrentOptionCount = 0;

static bool g_WaterAreaUnlocked = false;

static int g_PanelCapLeftID = TEXTURE_INVALID_ID;
static int g_PanelCapMidID = TEXTURE_INVALID_ID;
static int g_PanelCapRightID = TEXTURE_INVALID_ID;
static float g_PanelCapWidth = 0.0f;

static const char* GetUpgradeName(UpgradeType type)
{
    switch (type)
    {
    case UpgradeType_MoveSpeed: return "Faster Movement";
    case UpgradeType_WaterArea: return "Wide Watering Pail";
    case UpgradeType_InteractionSpeed: return "Quick Hands";
    default: return "???";
    }
}

static const char* GetUpgradeDescription(UpgradeType type)
{
    switch (type)
    {
    case UpgradeType_MoveSpeed: return "Move around\nthe farm faster.";
    case UpgradeType_WaterArea: return "Water a 2x2 area\ninstead of 1 crop.";
    case UpgradeType_InteractionSpeed: return "Plant, water, build \nand harvest faster.";
    default: return "";
    }
}

static int GetUpgradeIconTexture(UpgradeType type)
{
    switch (type)
    {
    case UpgradeType_MoveSpeed: return g_SpeedIconTextureID;
    case UpgradeType_WaterArea: return Inventory_GetIconTexture(ItemType_WaterPail);
    case UpgradeType_InteractionSpeed: return g_HandsIconTextureID;
    default: return TEXTURE_INVALID_ID;
    }
}

static void ApplyUpgrade(UpgradeType type)
{
    switch (type)
    {
    case UpgradeType_MoveSpeed:
        GamePlayer_AddSpeedBonus(SPEED_BOOST_AMOUNT);
        break;
    case UpgradeType_WaterArea:
        g_WaterAreaUnlocked = true;
        break;
    
    case UpgradeType_InteractionSpeed:
        PlayerInteraction_ApplyInteractionSpeedBoost();
        break;
    }
}

void Upgrade_Initialize()
{
    g_SpeedIconTextureID = Texture_Load(L"assets/UI/SpeedBoost.PNG", true);
    g_HandsIconTextureID = Texture_Load(L"assets/UI/Hands.PNG", true);

    g_PanelCapLeftID = Texture_Load(L"assets/UI/UI_L.PNG", true);
    g_PanelCapMidID = Texture_Load(L"assets/UI/UI_M.PNG", true);
    g_PanelCapRightID = Texture_Load(L"assets/UI/UI_R.PNG", true);
    g_PanelCapWidth = (float)Texture_GetWidth(g_PanelCapLeftID);

    g_ChoiceActive = false;
    g_SelectedOption = 0;
    g_WaterAreaUnlocked = false;
}

void Upgrade_Finalize()
{
    Texture_Release(g_SpeedIconTextureID);
    Texture_Release(g_HandsIconTextureID);
    Texture_Release(g_PanelCapLeftID);
    Texture_Release(g_PanelCapMidID);
    Texture_Release(g_PanelCapRightID);
}

bool Upgrade_TryBeginChoice(LevelType clearedLevel)
{
    g_CurrentOptionCount = 0;

    if (clearedLevel == 1)
    {
		g_CurrentOptions[g_CurrentOptionCount++] = UpgradeType_MoveSpeed;
		g_CurrentOptions[g_CurrentOptionCount++] = UpgradeType_WaterArea;

	}
    else if (clearedLevel == 2)
    {
        g_CurrentOptions[g_CurrentOptionCount++] = UpgradeType_MoveSpeed;
        g_CurrentOptions[g_CurrentOptionCount++] = UpgradeType_InteractionSpeed;
        if (!g_WaterAreaUnlocked)
        {
            g_CurrentOptions[g_CurrentOptionCount++] = UpgradeType_WaterArea;
        }
    }

	if (g_CurrentOptionCount == 0) return false;

	g_SelectedOption = 0;
	g_ChoiceActive = true;
    return true; 
}

bool Upgrade_IsChoiceActive()
{
    return g_ChoiceActive;
}

void Upgrade_Update(float delta_time)
{
    if (!g_ChoiceActive) return;

    if (InputKeyboard_IsTrigger(KK_LEFT) || InputKeyboard_IsTrigger(KK_A))  g_SelectedOption = (g_SelectedOption - 1 + g_CurrentOptionCount) % g_CurrentOptionCount;
    if (InputKeyboard_IsTrigger(KK_RIGHT) || InputKeyboard_IsTrigger(KK_D)) g_SelectedOption = (g_SelectedOption + 1) % g_CurrentOptionCount;

    if (InputKeyboard_IsTrigger(KK_ENTER))
    {
        ApplyUpgrade(g_CurrentOptions[g_SelectedOption]);
        g_ChoiceActive = false;
    }
}

static constexpr float CARD_WIDTH = 260.0f;
static constexpr float CARD_HEIGHT = 260.0f;
static constexpr float CARD_GAP = 60.0f;
static constexpr float ICON_SIZE = 96.0f;
static constexpr float SELECT_SCALE = 1.05f;

static void Draw3Slice(int leftID, int midID, int rightID, float capWidth,
    float x, float y, float width, float height, const DirectX::XMFLOAT4& tint = { 1.0f, 1.0f, 1.0f, 1.0f })
{
    float midWidth = width - capWidth * 2.0f;
    if (midWidth < 0.0f) midWidth = 0.0f;

    Sprite_Draw(leftID, x, y, capWidth, height, tint);
    Sprite_Draw(midID, x + capWidth, y, midWidth, height, tint);
    Sprite_Draw(rightID, x + capWidth + midWidth, y, capWidth, height, tint);
}

void Upgrade_Draw()
{
    if (!g_ChoiceActive) return;

    const char* title = "CHOOSE AN UPGRADE";
    constexpr float TITLE_SCALE = 3.5f;
    DirectX::XMFLOAT2 titleSize = Font_MeasureText(title, TITLE_SCALE);

    float blockWidth = g_CurrentOptionCount * CARD_WIDTH + (g_CurrentOptionCount - 1) * CARD_GAP;
    float blockLeft = SCREEN_WIDTH * 0.5f - blockWidth * 0.5f;
    float blockTop = SCREEN_HEIGHT * 0.5f - CARD_HEIGHT * 0.5f;

    Font_Print(title, SCREEN_WIDTH * 0.5f - titleSize.x * 0.5f, blockTop - titleSize.y - 30.0f, TITLE_SCALE);

    for (int i = 0; i < g_CurrentOptionCount; i++)
    {
        bool selected = (i == g_SelectedOption);
        float scale = selected ? SELECT_SCALE : 1.0f;

        float baseX = blockLeft + i * (CARD_WIDTH + CARD_GAP);
        float baseY = blockTop;

        float cardW = CARD_WIDTH * scale;
        float cardH = CARD_HEIGHT * scale;
        float cardX = baseX - (cardW - CARD_WIDTH) * 0.5f;
        float cardY = baseY - (cardH - CARD_HEIGHT) * 0.5f;

        Draw3Slice(g_PanelCapLeftID, g_PanelCapMidID, g_PanelCapRightID, g_PanelCapWidth,
            cardX, cardY, cardW, cardH);

        UpgradeType type = g_CurrentOptions[i];

        // icon on top
        float iconSize = ICON_SIZE * scale;
        float iconX = cardX + (cardW - iconSize) * 0.5f;
        float iconY = cardY + 20.0f * scale;
        Sprite_Draw(GetUpgradeIconTexture(type), iconX, iconY, iconSize, iconSize, 0, 0, 96, 96, 0.0f);

        // name + explanation below the icon
        constexpr float NAME_SCALE = 2.2f;
        constexpr float DESC_SCALE = 1.8f;
        const char* name = GetUpgradeName(type);
        const char* desc = GetUpgradeDescription(type);

        DirectX::XMFLOAT2 nameSize = Font_MeasureText(name, NAME_SCALE * scale);
        float nameY = iconY + iconSize + 16.0f * scale;
        Font_Print(name, cardX + (cardW - nameSize.x) * 0.5f, nameY, NAME_SCALE * scale);

        DirectX::XMFLOAT2 descSize = Font_MeasureText(desc, DESC_SCALE * scale);
        float descY = nameY + nameSize.y + 10.0f * scale;
        Font_Print(desc, cardX + (cardW - descSize.x) * 0.5f, descY, DESC_SCALE * scale);

        // select prompt at the bottom of the card
        const char* selectLabel = selected ? "> SELECT <" : "SELECT";
        constexpr float SELECT_LABEL_SCALE = 2.0f;
        DirectX::XMFLOAT2 selectSize = Font_MeasureText(selectLabel, SELECT_LABEL_SCALE * scale);
        float selectY = cardY + cardH - selectSize.y - 16.0f * scale;
        DirectX::XMFLOAT4 selectColor = selected
            ? DirectX::XMFLOAT4{ 1.0f, 0.85f, 0.2f, 1.0f }
        : DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
        Font_Print(selectLabel, cardX + (cardW - selectSize.x) * 0.5f, selectY, SELECT_LABEL_SCALE * scale, selectColor);
    }

    const char* hint = "LEFT/RIGHT Choose   ENTER Confirm";
    DirectX::XMFLOAT2 hintSize = Font_MeasureText(hint, 2.0f);
    Font_Print(hint, SCREEN_WIDTH * 0.5f - hintSize.x * 0.5f, blockTop + CARD_HEIGHT + 40.0f, 2.0f);
}

bool Upgrade_IsWaterAreaUnlocked()
{
    return g_WaterAreaUnlocked;
}