import socket
import json

def send_command(cmd):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("192.168.100.102", 8080))
    
    json_data = json.dumps(cmd)
    print(f"Sending: {json_data}")
    sock.send(json_data.encode())
    
    response = sock.recv(4096).decode()
    print(f"Response: {response}")
    sock.close()
    return response

print("=== Interactive MongoDB Client ===")
print("Commands: REGISTER, LOGIN, SAVE, LOAD")
print("Type 'exit' to quit")
print()

while True:
    command = input("Enter command: ").strip().upper()
    if command == "EXIT":
        break
    
    player_id = input("Enter player_id: ").strip()
    
    if command == "REGISTER":
        password = input("Enter password: ").strip()
        send_command({"command": command, "player_id": player_id, "password": password})
    
    elif command == "LOGIN":
        password = input("Enter password: ").strip()
        response = send_command({"command": command, "player_id": player_id, "password": password})
        try:
            data = json.loads(response)
            if data.get("success"):
                print(f"Welcome back, {player_id}!")
                print(f"Position: {data.get('data', {}).get('position')}")
                print(f"Health: {data.get('data', {}).get('health')}")
                print(f"Level: {data.get('data', {}).get('level')}")
                print(f"Score: {data.get('data', {}).get('score')}")
        except:
            pass
    
    elif command == "SAVE":
        x = float(input("Enter x position: "))
        y = float(input("Enter y position: "))
        z = float(input("Enter z position: "))
        health = int(input("Enter health: "))
        level = int(input("Enter level: "))
        score = int(input("Enter score: "))
        send_command({
            "command": command,
            "player_id": player_id,
            "position": {"x": x, "y": y, "z": z},
            "health": health,
            "level": level,
            "score": score
        })
    
    else:
        print("Unknown command!")
    
    print()