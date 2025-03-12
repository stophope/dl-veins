import math

def calculate_point(length, width, distances):
    # 初始化点的坐标
    x, y = 0, 0
    
    # 遍历所有角度，更新点的坐标
    for angle in range(360):
        radian = math.radians(angle)
        cos_val = math.cos(radian)
        sin_val = math.sin(radian)
        
        if cos_val > 0:  # 向右
            distance_to_right = (length - x) / cos_val
        else:  # 向左
            distance_to_right = x / abs(cos_val)
        
        if sin_val > 0:  # 向上
            distance_to_top = (width - y) / sin_val
        else:  # 向下
            distance_to_top = y / abs(sin_val)
        
        expected_distance = distances[angle]
        
        # 跳过无效点
        if expected_distance == 0.0:
            continue
        
        if math.isclose(distance_to_right, expected_distance, rel_tol=1e-9):
            x = length - expected_distance * cos_val if cos_val > 0 else expected_distance * abs(cos_val)
        elif math.isclose(distance_to_top, expected_distance, rel_tol=1e-9):
            y = width - expected_distance * sin_val if sin_val > 0 else expected_distance * abs(sin_val)
    
    return x, y

# 示例输入
length = 10
width = 5
distances = [float('inf')] * 360
distances[0] = 10  # 0度方向，向右
distances[90] = 5  # 90度方向，向上
distances[180] = 10  # 180度方向，向左
distances[270] = 5  # 270度方向，向下

# 计算点的坐标
point = calculate_point(length, width, distances)
print("点的坐标:", point)