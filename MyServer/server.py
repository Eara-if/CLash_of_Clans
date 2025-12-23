from flask import Flask, request, jsonify

app = Flask(__name__)

# 模拟数据库，存储在线玩家
online_players = {}

@app.route('/login', methods=['POST'])
def login():
    # 获取客户端发来的 JSON 数据
    data = request.json
    if not data:
        # 兼容处理非标准 JSON 字符串
        import json
        data = json.loads(request.data)

    username = data.get('username')
    
    if username:
        online_players[username] = "Online"
        print(f"Player Logged In: {username}")
        return jsonify({"status": "success", "message": f"Welcome {username}"})
    
    return jsonify({"status": "error", "message": "Invalid username"}), 400

@app.route('/players', methods=['GET'])
def get_players():
    # 返回所有在线玩家列表
    return jsonify(list(online_players.keys()))

if __name__ == '__main__':
    # 运行在 5000 端口，host='0.0.0.0' 允许局域网访问
    app.run(host='0.0.0.0', port=5000)
