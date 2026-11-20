# TCP-Server-and-MongoDB-Integration-for-a-multi-user-3d-simulation
## Networking & Persistence Module

### Architecture
- **Server** (`networking/server.py`) runs on the host machine.
  It owns the MongoDB connection and handles all persistence.
- **Client** (`networking/client.py`) runs on the player's machine.
  It sends `SAVE` / `LOAD` commands over TCP and never touches the database directly.
- **MongoDB** stores one document per player in `game_db.players`.

### Message protocol
Commands are newline-terminated ASCII strings:

| Command | Format | Response |
|---|---|---|
| SAVE | `SAVE|player_id|x|y|z|health\n` | `OK\n` |
| LOAD | `LOAD|player_id\n` | `x|y|z|health\n` or `NOT_FOUND\n` |

### Player document schema
```json
{
  "player_id": "Player1",
  "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
  "health": 100,
  "last_saved": "2026-11-20T10:00:00Z"
}
