import socket
import json

def send_commands():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("192.168.100.102", 8080))
    
    # Send multiple commands without closing
    commands = [
        {"command": "REGISTER", "player_id": "Player2", "password": "pass123"},
        {"command": "SAVE", "player_id": "Player2", "position": {"x": 10, "y": 20, "z": 30}, "health": 80, "level": 2, "score": 50},
        {"command": "LOGIN", "player_id": "Player2", "password": "pass123"}
    ]
    
    for cmd in commands:
        json_data = json.dumps(cmd)
        print(f"Sending: {json_data}")
        sock.send(json_data.encode())
        
        # Wait for response
        response = sock.recv(4096).decode()
        print(f"Response: {response}")
        print()
    
    sock.close()
    print("All commands sent!")

if __name__ == "__main__":
    send_commands()