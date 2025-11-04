from map import GameMap
from tower import BasicTower
from enemy import Enemy

class Game:
    def __init__(self):
        self.map = GameMap()
        self.towers = []
        self.enemies = []
        self.money = 100
        self.health = 10
        self.tick = 0

    def spawn_enemy(self):
        if self.tick % 10 == 0:  # spawn every 10 ticks
            self.enemies.append(Enemy())

    def add_tower(self, tower_type, position_index):
        if position_index not in self.map.tower_spots:
            print("Invalid position!")
            return
        if self.money < 25:
            print("Not enough money!")
            return
        self.towers.append(BasicTower(position_index))
        self.money -= 25

    def update(self):
        self.tick += 1
        self.spawn_enemy()

        # step enemies
        for e in self.enemies[:]:
            e.move()
            if e.position >= len(self.map.path):
                self.health -= 1
                self.enemies.remove(e)

        # tower atk
        for tower in self.towers:
            for e in self.enemies[:]:
                if abs(tower.position - e.position) <= 1:  # simple range check
                    e.take_damage(tower.damage)
                    if e.health <= 0:
                        self.money += 10
                        self.enemies.remove(e)
                        break

        # print simulated lcd
        self.display_status()

    def display_status(self):
        print(f"[LCD] Health: {self.health} | Money: {self.money} | Enemies: {len(self.enemies)} | Towers: {len(self.towers)}")

    def is_game_over(self):
        return self.health <= 0

