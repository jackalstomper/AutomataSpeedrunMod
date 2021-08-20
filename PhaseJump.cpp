#include "PhaseJump.hpp"
#include "imgui.h"
#include <vector>
#include <string>

namespace {

const std::vector<std::string> phaseNames = {
    "NONE",
    "00_00_B_RobotM_Pro_RobotBrother",
    "00_02_B_RobotM_Pro_9S_appearanc",
    "00_05_B_RobotM_Pro_9S_STG1",
    "00_10_B_RobotM_Pro_9S_STG2",
    "00_15_B_RobotM_Pro_9S_STG3",
    "00_30_A_RobotM_Prologue_STG1",
    "00_40_A_RobotM_Prologue_STG2",
    "00_50_A_RobotM_Prologue_STG3",
    "00_55_A_RobotM_Prologue_EV",
    "00_60_A_RobotM_Pro_Tutorial",
    "00_70_A_RobotM_Pro_ArmBoss_1st",
    "00_80_A_RobotM_Pro_1stArea",
    "00_80_B_RobotM_Pro_BossArea",
    "00_82_B_RobotM_Pro_9S_STG4",
    "00_83_B_RobotM_Pro_9S_STG5",
    "00_85_A_RobotM_Pro_2ndArea",
    "00_90_A_RobotM_Pro_3rdArea",
    "00_95_A_RobotM_Pro_lastArea",
    "05_00_A_RobotM_Pro_ArmBoss_2nd",
    "05_10_A_RobotM_ProBoss_phase1",
    "05_10_B_RobotM_ProBoss_phase1",
    "05_20_A_RobotM_ProBoss_phase2",
    "05_20_B_RobotM_ProBoss_phase2",
    "05_30_A_RobotM_ProBoss_phase3",
    "05_30_B_RobotM_ProBoss_phase3",
    "05_40_A_RobotM_ProBoss_phase4",
    "05_40_B_RobotM_ProBoss_phase4",
    "10_A_bunker_Robot_Battle",
    "save_p100_10",
    "11_A_bunker_monitorHD",
    "13_A_bunker_Setup",
    "13_B_bunker_Setup",
    "16_AB_bunker_Order",
    "20_AB_buker_Aerial_battle",
    "30_AB_Ruined_City_ForcedLanding",
    "40_AB_Resistance_Camp_Leader",
    "50_AB_Desert_temple",
    "51_AB_MainDesert",
    "51_10_AB_Danchi_EV",
    "52_AB_Danchi_Haikyo",
    "55_AB_Danchi_Boss_1",
    "56_AB_Danchi_Boss_EV1",
    "56_10_AB_Danchi_Boss_Battle",
    "56_50_AB_Danchi_Boss_EV2",
    "58_AB_BossArea_Fall",
    "58_50_AB_Goto_AmusementPark",
    "58_60_B_Pod_Talk",
    "70_AB_Amusement_Park",
    "70_01_AB_Amusement_Park_TEST",
    "70_10_AB_Park_Tank",
    "70_20_AB_Park_RollerCoaster",
    "76_AB_Amusement_Park_Boss",
    "76_10_AB_Amusement_Park_Boss",
    "76_20_AB_Amusement_Park_Boss",
    "77_AB_Amusement_Park_Boss",
    "77_10_AB_Amusement_Park_Boss",
    "78_AB_Amusement_Park_Boss",
    "78_10_AB_Amusement_Park_Boss",
    "79_50_AB_Goto_Pascal_Village",
    "79_90_AB_GuideTo_Pascal_Village",
    "79_95_B_Ruined_City_diningtable",
    "80_00_AB_Pascal_Village",
    "80_10_AB_Pascal_Village",
    "90_00_AB_Ruined_City",
    "90_10_AB_Ruined_City_AerialBTL",
    "100_A_PlayGoNovel",
    "00_80_A_TEST",
    "00_80_B_TEST",
    "00_80_C_TEST",
    "50_00_A_City",
    "51_00_A_Desert",
    "60_00_TEST_SubmergeCity",
    "00_AB_Ruined_City",
    "10_AB_Alien_Ship",
    "10_10_AB_Alien_Ship_Start",
    "10_20_AB_Alien_Ship_Battle",
    "10_30_AB_Alien_Ship_End",
    "15_AB_Transfer_device",
    "20_AB_Bunker",
    "30_AB_Pascal_Village",
    "40_00_AB_ShopM_EmilQuest_Start",
    "40_30_AB_ShopM_EQ_EmilAppear",
    "40_70_AB_ShopM_EmilQuest_End",
    "60_05_AB_Forest_Castle",
    "60_00_B_King_of_the_forest_ev1",
    "60_06_B_Forest_Castle2",
    "60_07_B_King_of_the_forest_ev2",
    "60_08_B_Forest_Castle3",
    "60_10_AB_Forest_Castle_Castle",
    "60_15_B_King_of_the_forest_ev3",
    "60_16_B_Forest_Castle_Castle2",
    "60_20_AB_Forest_Castle_Throne",
    "70_AB_Resistance_Camp",
    "80_10_AB_Resistance_Camp",
    "80_20_AB_Submerge_City",
    "95_00_B_Ruined_City_diningtable",
    "80_30_AB_Submerge_City",
    "90_AB_Battle_Submerge_City",
    "91_AB_Battle_Submerge_City_2",
    "92_AB_Battle_Submerge_City_3",
    "94_00_AB_Kaiju_Start_EV",
    "94_10_AB_Kaiju_Battle_Head",
    "94_15_AB_Kaiju_Battle_Head_EV",
    "94_30_AB_Kaiju_Battle_Sniping",
    "94_40_AB_Kaiju_EV",
    "94_43_AB_Kaiju_Battle_Escape",
    "94_46_AB_Kaiju_Battle_Pascal",
    "94_50_A_Kaiju_Battle_Body",
    "94_60_B_Kaiju_Shooting",
    "94_70_B_Kaiju_Missile",
    "94_80_AB_Kaiju_End_EV",
    "96_B_Pod_Talk",
    "100_A_Submerge_City_2B",
    "100_A_Submerge_City_2B_TEST",
    "110_00_A_Copy_City",
    "110_00_B_Copy_City",
    "115_AB_Copy_City_AdamDie",
    "115_B_Copy_City_AdamDie",
    "120_AB_Bunker",
    "130_00_A_Resistance_Camp",
    "140_00_A_RobotM_CrazyReligion",
    "140_B_Banker_9S",
    "140_20_A_RobotM_CR_GuruRoom",
    "140_20_B_RobotM_CR_HackingStart",
    "140_30_A_RobotM_CR_GuruBattle",
    "140_30_B_RobotM_CR_UF_Rescue",
    "140_35_B_RobotM_CR_UF_Rescue2",
    "140_40_A_RobotM_CR_UnderFactory",
    "140_40_B_RobotM_CR_UF_Hacking",
    "140_60_A_RobotM_CR_UF_Middle",
    "140_80_A_RobotM_CR_UF_BossArea",
    "140_80_B_RobotM_CR_CoreHacking",
    "140_90_A_RobotM_CR_UF_Exit",
    "140_90_B_Banker_9S",
    "145_AB_Ruined_City_diningtable",
    "150_B_Banker_9S",
    "160_A_Ruined_City_2B",
    "160_B_Ruined_City_ArealBTL_9S",
    "170_00_A_Resistance_Camp_2B",
    "170_10_A_Resistance_Camp_2B",
    "170_20_A_Resistance_Camp_2B",
    "170_30_A_Resistance_Camp_2B",
    "170_40_A_Resistance_Camp_2B",
    "180_AB_Ruined_Front_Of_Camp",
    "190_AB_Pascal_Village",
    "195_AB_Pascal_Village_toEve",
    "200_00_AB_Ruined_City_Eve_EV",
    "200_10_AB_Ruined_City_EveBattle",
    "200_20_AB_Ruined_City_EveBattle",
    "200_30_AB_Ruined_City_Eve_EV",
    "200_40_AB_Ruined_City_EveBattle",
    "200_50_AB_Ruined_City_EveBattle",
    "200_60_AB_Ruined_City_Eve_EV",
    "200_70_AB_Ruined_City_end",
    "200_80_AB_Ruined_City_end",
    "EndRoll_A",
    "BackTitle_A",
    "00_CD_Bunker_2B",
    "00_10_CD_Bunker_Hangar_2B",
    "10_CD_Ruined_City_9S",
    "10_10_CD_Ruined_City_SPEnemy_9S",
    "15_CD_Ruined_City_9S",
    "17_CD_Bunker_Aerial_battle_2B",
    "20_CD_Ruined_City_2B",
    "21_CD_Ruined_City_2B",
    "22_CD_Ruined_City_2B",
    "23_CD_Ruined_City_2B",
    "25_CD_Ruined_City_2B",
    "26_CD_Ruined_City_9S",
    "26_10_CD_Ruined_City_9S",
    "27_CD_Ruined_City_2B",
    "28_CD_Ruined_City_9S",
    "30_00_CD_Bunker_2B",
    "30_10_CD_Bunker_Battle_2B",
    "40_CD_Bunker_Shooting_2B",
    "50_CD_Submerge_City_2B",
    "50_10_CD_Ruined_City_A2Come_2B",
    "50_20_CD_Ruined_City_2BDead_9S",
    "60_CD_POD",
    "70_00_CD_1_Ruined_City",
    "70_10_CD_1_Ruined_City",
    "70_20_CD_1_Ruined_City",
    "70_CD_1_Ruined_City_A2",
    "80_CD_1_Desert_A2",
    "81_CD_1_DesertBoss_HD_A2",
    "82_CD_1_DesertBoss_1_A2",
    "83_CD_1_DesertBoss_HD_2_A2",
    "84_CD_1_DesertBoss_HK_A2",
    "85_CD_1_DesertBoss_HK_HD_A2",
    "90_CD_2_Pascal_Village_A2_SQ",
    "91_00_CD_2_Pascal_Village_A2_SQ",
    "91_10_CD_2_Pascal_Village_A2_SQ",
    "91_20_CD_2_Pascal_Village_A2_SQ",
    "95_CD_2_Pascal_Village_A2",
    "100_CD_2_Desert_A2",
    "110_CD_2_Pascal_Village_A2",
    "120_00_CD_2_RobotM_Shelter_A2",
    "120_30_CD_2_RobotM_TankBoss_A2",
    "120_50_CD_2_RobotM_Operate1_A2",
    "120_70_CD_2_RobotM_Operate2_A2",
    "120_90_CD_2_RobotM_Pascal_A2",
    "120_95_CD_2_Branch_A2_9S",
    "130_00_CD_Pod_Talk",
    "140_CD_1_Resistance_Camp_9S",
    "150_CD_Ruined_City_9S",
    "160_00_CD_1_Forest_Castle_9S",
    "160_10_CD_1_Forest_Castle_9S",
    "160_20_CD_1_Forest_Castle_9S",
    "165_00_CD_1_Forest_Castle_9S",
    "170_00_CD_2_Forest_Castle_9S",
    "170_10_CD_2_Pod_Talk",
    "180_00_CD_2_Forest_Castle_9S_2",
    "180_10_CD_2_Submerge_City_9S",
    "180_20_CD_2_Submerge_City_9S",
    "180_30_CD_2_Pod_Talk",
    "190_CD_2_Hack_Game_9S",
    "200_CD_Robot_Mountain_A2",
    "210_CD_Amusement_Park_A2",
    "220_CD_Submerge_City_9S",
    "220_10_CD_GotoCamp_Yoroyoro_9S",
    "220_20_CD_ResiCamp_DeboPopo_HD",
    "220_30_CD_ResiCamp_Recover_9S",
    "230_CD_Resistance_Camp_9S",
    "240_CD_Amusement_Park",
    "240_10_CD_Amusement_Park_9S",
    "240_20_CD_Amusement_Park_9S",
    "240_30_CD_Amusement_Park_9S",
    "240_40_CD_Amusement_Park_9S",
    "240_50_CD_Amusement_Park_A2",
    "240_60_CD_Amusement_Park_A2",
    "240_70_CD_Amusement_Park_A2",
    "240_80_CD_Amusement_Park_A2",
    "250_CD_Unknown_9S",
    "260_00_CD_Ruined_City_9S",
    "270_CD_Tower_9S",
    "275_CD_Tower_walk_9S",
    "277_CD_Tower_2Bbtl_9S",
    "280_CD_Ruined_City_A2",
    "290_CD_Tower_A2",
    "293_CD_Tower_LibraryHack_A2",
    "295_CD_Tower_SelfHack_9S",
    "295_10_CD_Tower_RedGirl_9S",
    "295_20_CD_Tower_Yoruha_9S",
    "297_CD_Tower_Library_A2",
    "298_CD_Tower_RedGirl_A2",
    "300_00_CD_Tower_Elevator_A2",
    "310_00_CD_Shooting_9S",
    "300_10_CD_Tower_Elevator_A2",
    "310_10_CD_Shooting_9S",
    "300_20_CD_Tower_Elevator_A2",
    "310_20_CD_Shooting_9S",
    "300_30_CD_Tower_Elevator_A2",
    "310_30_CD_Shooting_9S",
    "300_40_CD_Tower_Elevator_A2",
    "310_50_CD_Shooting_3Dbattle_9S",
    "315_CD_Tower_Unite",
    "320_CD_Tower_Summit",
    "330_CD_Tower_Versus",
    "332_CD_Tower_Talk",
    "335_CD_Tower_sele9S",
    "335_CD_Tower_seleA2",
    "340_C_Tower_9Swin",
    "340_D_Tower_A2win",
    "345_D_Tower_A2win_9Shack",
    "350_CD_TrueEnd_STG",
    "360_CD_TrueEnd_FIN"
};

const uint64_t PHASE_JUMP_FUNCTION_ADDR = 0x6291E0;

const uint64_t JUMP_USE_LONG_LOADSCREEN = 0x1; // the long load screen with a bunch of text lines
const uint64_t JUMP_NO_LOADSCREEN = 0x2; // no text. just a blank screen with a loading symbol
const uint64_t JUMP_FADE_WHITE = 1LLU << 32; // fade into load with white screen

// Object passed to JumpFunc. The values are what is found in memory when the game makes proper calls to JumpFunc
struct JumpContext {
    uint64_t loadscreenFlags; // 0 defaults to short load screen
    uint64_t moreFlags; // usually 0x78, when exiting to menu its 0x100000078
    uint64_t unknownValue; // always 0 from current testing
    uint64_t stringPointer; // Points to what looks like a debug menu string table.

    JumpContext()
        : loadscreenFlags(JUMP_NO_LOADSCREEN)
        , moreFlags(0x78)
        , unknownValue(0) // 0 when chapter select calls it so 0 here
        , stringPointer(0) // Setting to zero here to hopefully not break anything from the function trying to access unknown memory
    {}
};

// function addr: 0x14051A730
// RCX unknown - usually 0, sometimes 0xFFFFFFFF (starting new game)
// RDX jumpCode - apparently arbitrary number assigned to some phase names. Use debug build phasejump menu to know what they are.
// r8 phaseName
// r9 context
using JumpFunc = void __fastcall (unsigned unknown, unsigned jumpCode, const char* phaseName, JumpContext* context);

} // namespace

void UI::PhaseJump::render(uint64_t processRamStart) {
    using namespace ImGui;
    if (Button("PhaseJump"))
        m_visible = !m_visible;

    if (!m_visible)
        return;
    
    std::string selectedPhase;
    if (!Begin("Phase Jump"))
        return;

    Text("Select phase to jump to.");

    if (BeginCombo("Phase Name", phaseNames[0].c_str())) {
        for (const auto& phaseName : phaseNames) {
            bool isSelected = false;
            if (Selectable(phaseName.c_str(), &isSelected))
                selectedPhase = phaseName;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        EndCombo();
    }

    if (Button("Jump")) {
        if (selectedPhase == "NONE") {
            Text("No phase selected");
        } else {
            const JumpFunc* jumpFunc = reinterpret_cast<JumpFunc*>(processRamStart + PHASE_JUMP_FUNCTION_ADDR);
            JumpContext context;
            jumpFunc(0, 0x200, selectedPhase.c_str(), &context);
        }
    }

    End();
}