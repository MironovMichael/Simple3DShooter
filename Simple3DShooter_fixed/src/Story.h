#pragma once

#include "Collision.h"

#include <string>
#include <vector>

enum class MissionType
{
    Introduction,
    EliminateEnemies,
    CollectSupplies,
    ReachExtraction,
    Survive,
    FinalBattle
};

enum class MissionState
{
    Locked,
    Active,
    Completed,
    Failed
};

struct Mission
{
    MissionType type;

    std::string title;
    std::string description;

    int targetAmount;
    int currentAmount;

    float timeLimit;
    float remainingTime;

    MissionState state;

    bool optional;
    bool rewardGiven;

    int rewardScore;
    int rewardAmmo;
};

class Story
{
public:
    Story();

    void initialize();

    void update(
        float dt
    );

    void onEnemyKilled();

    void onSupplyCollected();

    void onExtractionReached();

    void onPlayerDied();

    void addSurvivalTime(
        float seconds
    );

    bool startNextMission();

    bool hasActiveMission() const;

    bool isFinished() const;

    bool isGameOver() const;

    MissionType getCurrentMissionType() const;

    const Mission* getCurrentMission() const;

    const Mission* getMission(
        int index
    ) const;

    int getCurrentMissionIndex() const;

    int getMissionCount() const;

    int getScoreReward() const;

    int getAmmoReward() const;

    void clearRewards();

    std::string getObjectiveText() const;

    std::string getStatusText() const;

private:
    std::vector<Mission> missions;

    int currentMission;

    bool finished;
    bool gameOver;

    int pendingScoreReward;
    int pendingAmmoReward;

    void completeCurrentMission();

    void failCurrentMission();

    void activateMission(
        int index
    );

    bool checkMissionCompletion();

    bool missionNeedsTimer() const;

    void giveMissionReward(
        Mission& mission
    );
};