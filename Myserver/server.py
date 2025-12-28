from flask import Flask, request, jsonify
import sqlite3  # <-- 报错的关键就在这里，必须加上这一行
import json

app = Flask(__name__)

# 初始化数据库
def init_db():
    conn = sqlite3.connect('game.db')
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS users 
                 (username TEXT PRIMARY KEY, save_data TEXT)''')
    conn.commit()
    conn.close()

# 1. 登录
# 修改后的登录接口
@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    conn = sqlite3.connect('game.db')
    c = conn.cursor()
    c.execute("SELECT save_data FROM users WHERE username=?", (username,))
    user = c.fetchone()
    if user:
        conn.close()
        return jsonify({"status": "success", "data": user[0]})
    else:
        # 【修改点】：定义初始数据，包含关卡进度 currentLevel: 1
        initial_save = {
            "currentLevel": 1,
            "buildings": [],
            "resources": {"coin": 5000, "water": 5000, "gem": 500}
        }
        initial_data_str = json.dumps(initial_save)
        
        c.execute("INSERT INTO users (username, save_data) VALUES (?, ?)", (username, initial_data_str))
        conn.commit()
        conn.close()
        print(f"New Account Created with level 1: {username}")
        return jsonify({"status": "success", "data": initial_data_str})

# 2. 保存
@app.route('/save', methods=['POST'])
def save_game():
    data = request.get_json()
    username = data.get('username')
    game_data = data.get('gameData')
    
    # 将接收到的 JSON 再次转为字符串存入数据库
    game_data_str = json.dumps(game_data)
    
    conn = sqlite3.connect('game.db')
    c = conn.cursor()
    c.execute("UPDATE users SET save_data=? WHERE username=?", (game_data_str, username))
    conn.commit()
    conn.close()
    print(f"Data Saved for {username}")
    return jsonify({"status": "success"})

# 3. 获取所有玩家列表
@app.route('/users', methods=['GET'])
def get_users():
    try:
        conn = sqlite3.connect('game.db')
        c = conn.cursor()
        c.execute("SELECT username FROM users")
        users = [row[0] for row in c.fetchall()]
        conn.close()
        return jsonify({"status": "success", "users": users})
    except Exception as e:
        print(f"Error in /users: {e}")
        return jsonify({"status": "error", "message": str(e)})

# 4. 获取特定玩家存档
@app.route('/get_user_data', methods=['POST'])
def get_user_data():
    data = request.get_json()
    target = data.get('username')
    conn = sqlite3.connect('game.db')
    c = conn.cursor()
    c.execute("SELECT save_data FROM users WHERE username=?", (target,))
    row = c.fetchone()
    conn.close()
    if row:
        return jsonify({"status": "success", "data": row[0]})
    return jsonify({"status": "error"})

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000)