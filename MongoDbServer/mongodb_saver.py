import json
import pymongo
import sys
import hashlib
from datetime import datetime

# Initialize MongoDB client
client = pymongo.MongoClient("mongodb://localhost:27017")
db = client["game_db"]
collection = db["players"]

def hash_password(password):
    return hashlib.sha256(password.encode()).hexdigest()

# ============================================================
# REGISTER
# ============================================================
def handle_register(data):
    player_id = data.get("player_id")
    password = data.get("password")
    
    print(f"[DEBUG] Register called with player_id: {player_id}")  # Debug line
    
    if not player_id or not password:
        return {"success": False, "message": "Missing username or password"}
    
    # Check if player already exists
    existing = collection.find_one({"player_id": player_id})
    if existing:
        return {"success": False, "message": "Username already taken!"}
    
    hashed = hash_password(password)
    
    # Insert new player with default stats
    collection.insert_one({
        "player_id": player_id,
        "password_hash": hashed,
        "position": {"x": 0.0, "y": 0.0, "z": 0.0},
        "yaw": 0.0,
        "health": 100,
        "score": 0,
        "level": 1,
        "exp": 0,
        "str": 10,
        "agi": 10,
        "int": 10,
        "statPoints": 5,
        "questAccepted": False,
        "questCompleted": False,
        "gruntsKilled": 0,
        "created_at": datetime.now().isoformat()
    })
    
    print(f"[DEBUG] Account created for: {player_id}")  # Debug line
    return {"success": True, "message": "Account created successfully!"}

# ============================================================
# LOGIN
# ============================================================
def handle_login(data):
    player_id = data.get("player_id")
    password = data.get("password")
    
    print(f"[DEBUG] Login called for: {player_id}")  # Debug line
    
    if not player_id or not password:
        return {"success": False, "message": "Missing username or password"}
    
    hashed = hash_password(password)
    player = collection.find_one({
        "player_id": player_id,
        "password_hash": hashed
    })
    
    if player:
        print(f"[DEBUG] Login successful for: {player_id}")  # Debug line
        return {
            "success": True,
            "message": "Login successful!",
            "data": {
                "position": player.get("position", {"x": 0.0, "y": 0.0, "z": 0.0}),
                "yaw": player.get("yaw", 0.0),
                "health": player.get("health", 100),
                "score": player.get("score", 0),
                "level": player.get("level", 1),
                "exp": player.get("exp", 0),
                "str": player.get("str", 10),
                "agi": player.get("agi", 10),
                "int": player.get("int", 10),
                "statPoints": player.get("statPoints", 5),
                "questAccepted": player.get("questAccepted", False),
                "questCompleted": player.get("questCompleted", False),
                "gruntsKilled": player.get("gruntsKilled", 0)
            }
        }
    else:
        print(f"[DEBUG] Login failed for: {player_id}")  # Debug line
        return {"success": False, "message": "Invalid username or password!"}

# ============================================================
# SAVE
# ============================================================
def handle_save(data):
    player_id = data.get("player_id")
    if not player_id:
        return {"success": False, "message": "Missing player_id"}
    
    position = data.get("position", {})
    pos_x = position.get("x", 0.0)
    pos_y = position.get("y", 0.0)
    pos_z = position.get("z", 0.0)
    
    yaw = data.get("yaw", 0.0)
    health = data.get("health", 100)
    score = data.get("score", 0)
    level = data.get("level", 1)
    exp = data.get("exp", 0)
    str_stat = data.get("str", 10)
    agi_stat = data.get("agi", 10)
    int_stat = data.get("int", 10)
    statPoints = data.get("statPoints", 5)
    questAccepted = data.get("questAccepted", False)
    questCompleted = data.get("questCompleted", False)
    gruntsKilled = data.get("gruntsKilled", 0)
    
    collection.update_one(
        {"player_id": player_id},
        {"$set": {
            "position": {"x": pos_x, "y": pos_y, "z": pos_z},
            "yaw": yaw,
            "health": health,
            "score": score,
            "level": level,
            "exp": exp,
            "str": str_stat,
            "agi": agi_stat,
            "int": int_stat,
            "statPoints": statPoints,
            "questAccepted": questAccepted,
            "questCompleted": questCompleted,
            "gruntsKilled": gruntsKilled,
            "last_saved": datetime.now().isoformat()
        }}
    )
    
    return {
        "success": True,
        "message": f"Game saved! Position: ({pos_x}, {pos_y}, {pos_z}) Health: {health} Level: {level}"
    }

# ============================================================
# LOAD
# ============================================================
def handle_load(data):
    player_id = data.get("player_id")
    if not player_id:
        return {"success": False, "message": "Missing player_id"}
    
    player = collection.find_one({"player_id": player_id})
    if player:
        return {
            "success": True,
            "message": "Data loaded successfully!",
            "data": {
                "position": player.get("position", {"x": 0.0, "y": 0.0, "z": 0.0}),
                "yaw": player.get("yaw", 0.0),
                "health": player.get("health", 100),
                "score": player.get("score", 0),
                "level": player.get("level", 1),
                "exp": player.get("exp", 0),
                "str": player.get("str", 10),
                "agi": player.get("agi", 10),
                "int": player.get("int", 10),
                "statPoints": player.get("statPoints", 5),
                "questAccepted": player.get("questAccepted", False),
                "questCompleted": player.get("questCompleted", False),
                "gruntsKilled": player.get("gruntsKilled", 0)
            }
        }
    else:
        return {"success": False, "message": "Player not found!"}

# ============================================================
# MAIN
# ============================================================
if __name__ == "__main__":
    response = {"success": False, "message": "Unknown execution error"}
    
    if len(sys.argv) < 2:
        response = {"success": False, "message": "No input JSON file provided"}
    else:
        input_filename = sys.argv[1]
        try:
            with open(input_filename, "r") as f:
                data = json.load(f)

            command = data.get("command")

            if command == "REGISTER":
                response = handle_register(data)
            elif command == "LOGIN":
                response = handle_login(data)
            elif command == "SAVE":
                response = handle_save(data)
            elif command == "LOAD":
                response = handle_load(data)
            else:
                response = {"success": False, "message": f"Unknown command: {command}"}

        except Exception as e:
            response = {"success": False, "message": f"Python error: {str(e)}"}

    # Save output to response.json for C++ to read
    with open("response.json", "w") as f:
        json.dump(response, f)