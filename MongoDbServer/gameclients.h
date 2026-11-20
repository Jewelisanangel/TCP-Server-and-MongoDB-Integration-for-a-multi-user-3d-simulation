// ============================================================
// PLAYER DATA STRUCT
// ============================================================
struct PlayerData {
    // Position
    float x = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;
    
    // Stats
    int health = 100;
    int score = 0;
    int level = 1;
    int exp = 0;
    
    // Attributes
    int str = 10;
    int agi = 10;
    int intel = 10;
    int statPoints = 5;
    
    // Quests
    bool questAccepted = false;
    bool questCompleted = false;
    int gruntsKilled = 0;
};