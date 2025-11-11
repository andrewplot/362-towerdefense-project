class BasicTower:
    def __init__(self, position):
        self.position = position
        self.damage = 1
        self.range = 1
        self.fire_rate = 1  # not used yet

    def __repr__(self):
        return f"Tower(pos={self.position}, dmg={self.damage})"

