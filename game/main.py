from game import Game
import time

def simulate_rfid_scan():
    # simulated rfid tower selection
    print("[RFID] Tower selected: BasicTower")
    return "BasicTower"

def main():
    game = Game()
    selected_tower = simulate_rfid_scan()

    # placeholder starter tower
    game.add_tower(selected_tower, 6)

    #game loop
    while not game.is_game_over():
        game.update()
        time.sleep(0.3)

    print("Game Over!")

if __name__ == "__main__":
    main()

