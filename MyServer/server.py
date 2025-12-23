from flask import Flask, request, jsonify
import json

app = Flask(__name__)

# 模拟数据库：{ "用户名": {"password": "密码", "gameData": "存档JSON"} }
users_db = {}

@app.route('/login', methods=['POST'])
def login():
    data = request.json
    username = data.get('username')
    password = data.get('password')
    
    if not username or not password:
        return jsonify({"status": "error", "message": "Missing info"}), 400

    # 逻辑：如果用户不存在，直接注册；如果存在，验证密码
    if username not in users_db:
        users_db[username] = {
            "password": password,
            "gameData": "" # 新用户存档为空
        }
        print(f"New Account Created: {username}")
        return jsonify({"status": "success", "isNew": True, "data": ""})
    else:
        if users_db[username]["password"] == password:
            print(f"User Logged In: {username}")
            # 返回该用户的存档数据
            return jsonify({
                "status": "success", 
                "isNew": False, 
                "data": users_db[username]["gameData"]
            })
        else:
            return jsonify({"status": "error", "message": "Wrong password"}), 401

# 【新增】保存存档的接口
@app.route('/save', methods=['POST'])
def save_data():
    data = request.json
    username = data.get('username')
    game_data = data.get('gameData')
    
    if username in users_db:
        users_db[username]["gameData"] = game_data
        print(f"Data Saved for {username}")
        return jsonify({"status": "success"})
    return jsonify({"status": "error"}), 404

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)