import socket
import json
import tkinter as tk
from tkinter import messagebox

# ============================================================
# SERVER CONNECTION
# ============================================================
SERVER_IP = "192.168.100.102"
SERVER_PORT = 8080

def send_request(request_data):
    """Send a request to the server and return the response"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_IP, SERVER_PORT))
        
        json_data = json.dumps(request_data)
        sock.send(json_data.encode())
        
        response = sock.recv(4096).decode()
        sock.close()
        return json.loads(response)
    except Exception as e:
        return {"success": False, "message": f"Connection error: {e}"}

# ============================================================
# GAME LAUNCHER GUI
# ============================================================
class GameLauncher:
    def __init__(self, root):
        self.root = root
        self.root.title("Game Launcher")
        self.root.geometry("400x350")
        self.root.resizable(False, False)
        
        # Title
        title = tk.Label(root, text="GAME LAUNCHER", font=("Arial", 20, "bold"))
        title.pack(pady=20)
        
        # Username
        tk.Label(root, text="Username:", font=("Arial", 12)).pack(pady=(10, 0))
        self.username_entry = tk.Entry(root, font=("Arial", 12), width=30)
        self.username_entry.pack(pady=5)
        
        # Password
        tk.Label(root, text="Password:", font=("Arial", 12)).pack(pady=(10, 0))
        self.password_entry = tk.Entry(root, font=("Arial", 12), width=30, show="*")
        self.password_entry.pack(pady=5)
        
        # Buttons
        button_frame = tk.Frame(root)
        button_frame.pack(pady=20)
        
        self.register_btn = tk.Button(
            button_frame, 
            text="REGISTER", 
            font=("Arial", 12),
            bg="#4CAF50", 
            fg="white",
            width=12,
            command=self.register
        )
        self.register_btn.pack(side=tk.LEFT, padx=10)
        
        self.login_btn = tk.Button(
            button_frame, 
            text="LOGIN", 
            font=("Arial", 12),
            bg="#2196F3", 
            fg="white",
            width=12,
            command=self.login
        )
        self.login_btn.pack(side=tk.LEFT, padx=10)
        
        # Status display
        self.status_label = tk.Label(
            root, 
            text="Enter username and password", 
            font=("Arial", 10),
            fg="gray"
        )
        self.status_label.pack(pady=10)
        
        # Game data display (shows after login)
        self.game_data_frame = tk.Frame(root)
        self.game_data_frame.pack(pady=10, fill=tk.X, padx=20)
        
        self.game_data_label = tk.Label(
            self.game_data_frame,
            text="",
            font=("Arial", 10),
            justify=tk.LEFT
        )
        self.game_data_label.pack()
        
        # Save button (appears after login)
        self.save_btn = None
        
        # Store player data
        self.current_player_id = None
        self.current_data = None
    
    def set_status(self, message, color="gray"):
        self.status_label.config(text=message, fg=color)
    
    def register(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()
        
        if not username or not password:
            self.set_status("Please enter username and password!", "red")
            return
        
        self.set_status("Registering...", "orange")
        self.root.update()
        
        response = send_request({
            "command": "REGISTER",
            "player_id": username,
            "password": password
        })
        
        if response.get("success"):
            self.set_status(f"✅ {response.get('message')}", "green")
            messagebox.showinfo("Success", "Account created! You can now login.")
        else:
            self.set_status(f"❌ {response.get('message')}", "red")
    
    def login(self):
        username = self.username_entry.get().strip()
        password = self.password_entry.get().strip()
        
        if not username or not password:
            self.set_status("Please enter username and password!", "red")
            return
        
        self.set_status("Logging in...", "orange")
        self.root.update()
        
        response = send_request({
            "command": "LOGIN",
            "player_id": username,
            "password": password
        })
        
        if response.get("success"):
            self.current_player_id = username
            self.current_data = response.get("data", {})
            
            self.set_status(f"✅ Welcome back, {username}!", "green")
            
            # Show game data
            self.show_game_data()
            
            # Show save button
            if self.save_btn is None:
                self.save_btn = tk.Button(
                    self.root,
                    text="SAVE GAME",
                    font=("Arial", 12),
                    bg="#FF9800",
                    fg="white",
                    width=15,
                    command=self.save_game
                )
                self.save_btn.pack(pady=10)
            
            messagebox.showinfo("Login Success", f"Welcome {username}!")
        else:
            self.set_status(f"❌ {response.get('message')}", "red")
    
    def show_game_data(self):
        data = self.current_data
        if data:
            text = (
                f"📍 Position: ({data.get('position', {}).get('x', 0)}, "
                f"{data.get('position', {}).get('y', 0)}, "
                f"{data.get('position', {}).get('z', 0)})\n"
                f"❤️ Health: {data.get('health', 100)}\n"
                f"⭐ Level: {data.get('level', 1)}\n"
                f"🏆 Score: {data.get('score', 0)}"
            )
            self.game_data_label.config(text=text, fg="white", bg="#333")
    
    def save_game(self):
        if not self.current_player_id:
            self.set_status("Please login first!", "red")
            return
        
        self.set_status("Saving...", "orange")
        self.root.update()
        
        # Simulate game data (in a real game, this comes from the game)
        # For testing, we increment values
        current_data = self.current_data or {}
        new_x = current_data.get('position', {}).get('x', 0) + 10
        new_y = current_data.get('position', {}).get('y', 0) + 5
        new_z = current_data.get('position', {}).get('z', 0)
        new_health = min(100, current_data.get('health', 100) + 5)
        new_level = current_data.get('level', 1) + (1 if new_health > 100 else 0)
        new_score = current_data.get('score', 0) + 25
        
        response = send_request({
            "command": "SAVE",
            "player_id": self.current_player_id,
            "position": {"x": new_x, "y": new_y, "z": new_z},
            "health": new_health,
            "level": new_level,
            "score": new_score
        })
        
        if response.get("success"):
            # Update local data
            self.current_data = {
                "position": {"x": new_x, "y": new_y, "z": new_z},
                "health": new_health,
                "level": new_level,
                "score": new_score
            }
            self.show_game_data()
            self.set_status(f"✅ Game saved! Position: ({new_x}, {new_y}, {new_z})", "green")
            
            # Show in a popup
            messagebox.showinfo(
                "Game Saved!", 
                f"Position: ({new_x}, {new_y}, {new_z})\n"
                f"Health: {new_health}\n"
                f"Level: {new_level}\n"
                f"Score: {new_score}"
            )
        else:
            self.set_status(f"❌ Save failed: {response.get('message')}", "red")

# ============================================================
# MAIN
# ============================================================
if __name__ == "__main__":
    root = tk.Tk()
    app = GameLauncher(root)
    root.mainloop()