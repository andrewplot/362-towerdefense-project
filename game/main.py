class Enemy:
    def __init__(self, enemy_type, path):
        self.type = enemy_type
        self.hp = 10 if enemy_type == "basic" else 20
        self.speed = 1 if enemy_type == "basic" else 0.5
        self.position = 0  # index along the path
        self.path = path

    def move(self):
        self.position += self.speed
        if self.position >= len(self.path):
            return True  # reached the end
        return False

    def take_damage(self, damage):
        self.hp -= damage
        return self.hp <= 0  # returns True if dead
