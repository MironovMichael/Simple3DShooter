#include "Story.h"

#include <algorithm>
#include <sstream>

Story::Story()
    : missions(),
      currentMission(-1),
      finished(false),
      gameOver(false),
      pendingScoreReward(0),
      pendingAmmoReward(0)
{
}

void Story::initialize()
{
    missions.clear();

    currentMission = -1;

    finished = false;
    gameOver = false;

    pendingScoreReward = 0;
    pendingAmmoReward = 0;

    /*
        --------------------------------------------------
        МИССИЯ 1
        --------------------------------------------------
    */

    Mission introduction;

    introduction.type =
        MissionType::Introduction;

    introduction.title =
        "MISSION 01";

    introduction.description =
        "Reach the combat zone and eliminate the first hostile.";

    introduction.targetAmount =
        1;

    introduction.currentAmount =
        0;

    introduction.timeLimit =
        0.0f;

    introduction.remainingTime =
        0.0f;

    introduction.state =
        MissionState::Locked;

    introduction.optional =
        false;

    introduction.rewardGiven =
        false;

    introduction.rewardScore =
        100;

    introduction.rewardAmmo =
        12;

    missions.push_back(
        introduction
    );

    /*
        --------------------------------------------------
        МИССИЯ 2
        --------------------------------------------------
    */

    Mission eliminate;

    eliminate.type =
        MissionType::EliminateEnemies;

    eliminate.title =
        "MISSION 02";

    eliminate.description =
        "Destroy the hostile patrol.";

    eliminate.targetAmount =
        8;

    eliminate.currentAmount =
        0;

    eliminate.timeLimit =
        0.0f;

    eliminate.remainingTime =
        0.0f;

    eliminate.state =
        MissionState::Locked;

    eliminate.optional =
        false;

    eliminate.rewardGiven =
        false;

    eliminate.rewardScore =
        300;

    eliminate.rewardAmmo =
        30;

    missions.push_back(
        eliminate
    );

    /*
        --------------------------------------------------
        МИССИЯ 3
        --------------------------------------------------
    */

    Mission supplies;

    supplies.type =
        MissionType::CollectSupplies;

    supplies.title =
        "MISSION 03";

    supplies.description =
        "Find and collect three supply crates.";

    supplies.targetAmount =
        3;

    supplies.currentAmount =
        0;

    supplies.timeLimit =
        180.0f;

    supplies.remainingTime =
        180.0f;

    supplies.state =
        MissionState::Locked;

    supplies.optional =
        false;

    supplies.rewardGiven =
        false;

    supplies.rewardScore =
        500;

    supplies.rewardAmmo =
        60;

    missions.push_back(
        supplies
    );

    /*
        --------------------------------------------------
        МИССИЯ 4
        --------------------------------------------------
    */

    Mission survive;

    survive.type =
        MissionType::Survive;

    survive.title =
        "MISSION 04";

    survive.description =
        "Survive the enemy assault.";

    survive.targetAmount =
        60;

    survive.currentAmount =
        0;

    survive.timeLimit =
        60.0f;

    survive.remainingTime =
        60.0f;

    survive.state =
        MissionState::Locked;

    survive.optional =
        false;

    survive.rewardGiven =
        false;

    survive.rewardScore =
        750;

    survive.rewardAmmo =
        90;

    missions.push_back(
        survive
    );

    /*
        --------------------------------------------------
        МИССИЯ 5
        --------------------------------------------------
    */

    Mission extraction;

    extraction.type =
        MissionType::ReachExtraction;

    extraction.title =
        "MISSION 05";

    extraction.description =
        "Reach the extraction zone.";

    extraction.targetAmount =
        1;

    extraction.currentAmount =
        0;

    extraction.timeLimit =
        0.0f;

    extraction.remainingTime =
        0.0f;

    extraction.state =
        MissionState::Locked;

    extraction.optional =
        false;

    extraction.rewardGiven =
        false;

    extraction.rewardScore =
        1000;

    extraction.rewardAmmo =
        120;

    missions.push_back(
        extraction
    );

    /*
        --------------------------------------------------
        ФИНАЛ
        --------------------------------------------------
    */

    Mission finalBattle;

    finalBattle.type =
        MissionType::FinalBattle;

    finalBattle.title =
        "MISSION 06";

    finalBattle.description =
        "Destroy the heavy enemy commander.";

    finalBattle.targetAmount =
        1;

    finalBattle.currentAmount =
        0;

    finalBattle.timeLimit =
        0.0f;

    finalBattle.remainingTime =
        0.0f;

    finalBattle.state =
        MissionState::Locked;

    finalBattle.optional =
        false;

    finalBattle.rewardGiven =
        false;

    finalBattle.rewardScore =
        2500;

    finalBattle.rewardAmmo =
        180;

    missions.push_back(
        finalBattle
    );

    /*
        Первая миссия запускается сразу.
    */

    activateMission(0);
}

void Story::update(
    float dt
)
{
    if (finished ||
        gameOver)
    {
        return;
    }

    if (!hasActiveMission())
    {
        return;
    }

    Mission& mission =
        missions[currentMission];

    if (mission.timeLimit > 0.0f)
    {
        mission.remainingTime -= dt;

        if (mission.remainingTime <= 0.0f)
        {
            mission.remainingTime =
                0.0f;

            /*
                Важно:
                Survive считается успешной,
                когда накоплено нужное время.
            */

            if (mission.type ==
                MissionType::Survive)
            {
                mission.currentAmount =
                    mission.targetAmount;

                completeCurrentMission();

                return;
            }

            failCurrentMission();

            return;
        }

        if (mission.type ==
            MissionType::Survive)
        {
            const float survived =
                mission.timeLimit -
                mission.remainingTime;

            mission.currentAmount =
                static_cast<int>(
                    survived
                );
        }
    }

    checkMissionCompletion();
}

void Story::onEnemyKilled()
{
    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    switch (mission.type)
    {
        case MissionType::Introduction:
        case MissionType::EliminateEnemies:
        case MissionType::FinalBattle:
        {
            mission.currentAmount++;

            break;
        }

        default:
            break;
    }

    checkMissionCompletion();
}

void Story::onSupplyCollected()
{
    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    if (mission.type !=
        MissionType::CollectSupplies)
    {
        return;
    }

    mission.currentAmount++;

    if (mission.currentAmount >
        mission.targetAmount)
    {
        mission.currentAmount =
            mission.targetAmount;
    }

    checkMissionCompletion();
}

void Story::onExtractionReached()
{
    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    if (mission.type !=
        MissionType::ReachExtraction)
    {
        return;
    }

    mission.currentAmount =
        mission.targetAmount;

    checkMissionCompletion();
}

void Story::onPlayerDied()
{
    gameOver =
        true;

    if (hasActiveMission())
    {
        missions[currentMission].state =
            MissionState::Failed;
    }
}

void Story::addSurvivalTime(
    float seconds
)
{
    if (seconds <= 0.0f)
        return;

    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    if (mission.type !=
        MissionType::Survive)
    {
        return;
    }

    mission.currentAmount +=
        static_cast<int>(seconds);

    if (mission.currentAmount >
        mission.targetAmount)
    {
        mission.currentAmount =
            mission.targetAmount;
    }

    checkMissionCompletion();
}

bool Story::startNextMission()
{
    if (gameOver ||
        finished)
    {
        return false;
    }

    if (currentMission < 0)
        return false;

    if (missions[currentMission].state !=
        MissionState::Completed)
    {
        return false;
    }

    const int next =
        currentMission + 1;

    if (next >=
        static_cast<int>(
            missions.size()
        ))
    {
        finished =
            true;

        return false;
    }

    activateMission(
        next
    );

    return true;
}

bool Story::hasActiveMission() const
{
    if (currentMission < 0)
        return false;

    if (currentMission >=
        static_cast<int>(
            missions.size()
        ))
    {
        return false;
    }

    return
        missions[currentMission].state ==
        MissionState::Active;
}

bool Story::isFinished() const
{
    return finished;
}

bool Story::isGameOver() const
{
    return gameOver;
}

MissionType Story::getCurrentMissionType() const
{
    if (!hasActiveMission())
    {
        return MissionType::Introduction;
    }

    return
        missions[currentMission].type;
}

const Mission*
Story::getCurrentMission() const
{
    if (currentMission < 0)
        return nullptr;

    if (currentMission >=
        static_cast<int>(
            missions.size()
        ))
    {
        return nullptr;
    }

    return
        &missions[currentMission];
}

const Mission*
Story::getMission(
    int index
) const
{
    if (index < 0)
        return nullptr;

    if (index >=
        static_cast<int>(
            missions.size()
        ))
    {
        return nullptr;
    }

    return
        &missions[index];
}

int Story::getCurrentMissionIndex() const
{
    return currentMission;
}

int Story::getMissionCount() const
{
    return static_cast<int>(
        missions.size()
    );
}

int Story::getScoreReward() const
{
    return pendingScoreReward;
}

int Story::getAmmoReward() const
{
    return pendingAmmoReward;
}

void Story::clearRewards()
{
    pendingScoreReward = 0;
    pendingAmmoReward = 0;
}

std::string Story::getObjectiveText() const
{
    const Mission* mission =
        getCurrentMission();

    if (!mission)
    {
        if (finished)
            return "MISSION COMPLETE";

        if (gameOver)
            return "MISSION FAILED";

        return "NO ACTIVE MISSION";
    }

    std::ostringstream text;

    text <<
        mission->description;

    if (mission->targetAmount > 1)
    {
        text <<
            " [" <<
            mission->currentAmount <<
            "/" <<
            mission->targetAmount <<
            "]";
    }

    if (mission->timeLimit > 0.0f &&
        mission->state ==
            MissionState::Active)
    {
        text <<
            "  TIME: " <<
            static_cast<int>(
                mission->remainingTime
            );
    }

    return text.str();
}

std::string Story::getStatusText() const
{
    if (finished)
        return "ALL MISSIONS COMPLETE";

    if (gameOver)
        return "MISSION FAILED";

    const Mission* mission =
        getCurrentMission();

    if (!mission)
        return "STANDBY";

    switch (mission->state)
    {
        case MissionState::Locked:
            return "LOCKED";

        case MissionState::Active:
            return "ACTIVE";

        case MissionState::Completed:
            return "COMPLETED";

        case MissionState::Failed:
            return "FAILED";
    }

    return "UNKNOWN";
}

void Story::completeCurrentMission()
{
    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    mission.state =
        MissionState::Completed;

    giveMissionReward(
        mission
    );
}

void Story::failCurrentMission()
{
    if (!hasActiveMission())
        return;

    Mission& mission =
        missions[currentMission];

    mission.state =
        MissionState::Failed;

    /*
        Для сюжетных миссий пока не
        заканчиваем игру автоматически:
        Game.cpp сможет решить, показать
        экран поражения или перезапустить
        миссию.
    */
}

void Story::activateMission(
    int index
)
{
    if (index < 0 ||
        index >=
            static_cast<int>(
                missions.size()
            ))
    {
        return;
    }

    currentMission =
        index;

    Mission& mission =
        missions[index];

    mission.state =
        MissionState::Active;

    mission.currentAmount =
        0;

    mission.rewardGiven =
        false;

    mission.remainingTime =
        mission.timeLimit;
}

bool Story::checkMissionCompletion()
{
    if (!hasActiveMission())
        return false;

    Mission& mission =
        missions[currentMission];

    if (mission.currentAmount <
        mission.targetAmount)
    {
        return false;
    }

    completeCurrentMission();

    return true;
}

bool Story::missionNeedsTimer() const
{
    const Mission* mission =
        getCurrentMission();

    if (!mission)
        return false;

    return
        mission->timeLimit > 0.0f;
}

void Story::giveMissionReward(
    Mission& mission
)
{
    if (mission.rewardGiven)
        return;

    mission.rewardGiven =
        true;

    pendingScoreReward +=
        mission.rewardScore;

    pendingAmmoReward +=
        mission.rewardAmmo;
}

// V25: narrative status helpers.
float Story::getCurrentProgress() const
{
    const Mission* m=getCurrentMission(); if(!m || m->targetAmount<=0) return m ? (m->state==MissionState::Completed?1.0f:0.0f) : 0.0f;
    return std::clamp(static_cast<float>(m->currentAmount)/static_cast<float>(m->targetAmount),0.0f,1.0f);
}
std::string Story::getCurrentTitle() const { const Mission* m=getCurrentMission(); return m ? m->title : std::string(); }
bool Story::isMissionComplete() const { const Mission* m=getCurrentMission(); return m && m->state==MissionState::Completed; }
