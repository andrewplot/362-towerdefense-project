class Enemy:
    def __init__(self):
        self.position = 0
        self.health = 3
        self.speed = 1  # move 1 unit per update

    def move(self):
        self.position += self.speed

    def take_damage(self, dmg):
        self.health -= dmg

    def __repr__(self):
        return f"Enemy(pos={self.position}, hp={self.health})"

