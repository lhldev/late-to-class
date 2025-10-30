int main (void) {
    bool died = false;
    float score = 0.0f;
    float velocity = 1.0f;
    int player_row = 0;
    const float acceleration = 0.001f;
    while (true) {
        if (!died) {
            velocity += acceleration * time_elapsed();
            score += velocity * time_elapsed();
            spawn_enemies();
            spawn_powerups();
            update_entities();
            if (collision()->type_id == ENEMY) {
                died = true;
            }
            if (collision()->type_id == POWER_UPS) {
                apply_powerups(collision());
            }
            switch (key_pressed()) {
                case UP_ARROW:
                    if (player_row < 1) {
                        player_row++;
                    }
                    break;
                case DOWN_ARROW:
                    if (player_row > -1) {
                        player_row--;
                    }
                    break;
            }
        }
        else {
            display("You died. Score: ", score);
            display("Press any key to restart.");
            if (key_pressed()) {
                died = false;
                score = 0.0f;
                velocity = 1.0f;
                acceleration = 0.001f;
                player_row = 0;
            }
        }
        render_screen();
        return 0;
    }
}
