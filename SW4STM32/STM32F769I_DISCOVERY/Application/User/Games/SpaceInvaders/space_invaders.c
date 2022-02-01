/*
 * space_invaders.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include "space_invaders.h"


struct si_game * si_init(Screen * screen) {

	uint8_t bitmap1[] = {
		0b00000000, 0b00000000,
		0b00001000, 0b00010000,
		0b00000100, 0b00100000,
		0b00001111, 0b11110000,
		0b00011011, 0b11011000,
		0b00111111, 0b11111100,
		0b00101111, 0b11110100,
		0b00101000, 0b00010100,
		0b00000110, 0b01100000,
		0b00000000, 0b00000000,
	};

	uint8_t * bitmap = (uint8_t *) malloc(sizeof(bitmap1));
	memcpy(bitmap, bitmap1, sizeof(bitmap1));

	struct si_enemy_sprite * si_enemy_sprite1 = (struct si_enemy_sprite *) malloc(sizeof(struct si_enemy_sprite));

	si_enemy_sprite1->width = 16;
	si_enemy_sprite1->bitmap_length = 160;
	si_enemy_sprite1->scale = 5;
	si_enemy_sprite1->bitmap = bitmap;

	struct si_enemy * si_enemy1 = (struct si_enemy *) malloc(sizeof(struct si_enemy));

	si_enemy1->sprites = si_enemy_sprite1;
	si_enemy1->sprite_count = 1;

	struct si_enemy_group * si_enemy_group1 = (struct si_enemy_group *) malloc(sizeof(struct si_enemy_group));

	si_enemy_group1->enemy = si_enemy1;
	si_enemy_group1->count = 10;
	si_enemy_group1->formation_width = screen->width * 0.6;
	si_enemy_group1->full_width = screen->width * 0.8;
	si_enemy_group1->step = 20;
	si_enemy_group1->group_direction = SI_ENEMY_GROUP_DIRECTION_LEFT;
	si_enemy_group1->group_offset = 0;

	struct si_level * si_level1 = (struct si_level *) malloc(sizeof(struct si_level));

	si_level1->groups = si_enemy_group1;
	si_level1->group_count = 1;

	struct si_game * game = (struct si_game *) malloc(sizeof(struct si_game));

	game->levels = si_level1;
	game->level_count = 1;
	game->header_height = 100;
	game->player_space_height = 100;
	game->tick_duration = 100;

	return game;
}

void si_update(struct si_game * game) {
	int curr_offset = game->levels[0].groups[0].group_offset;
	int curr_direction = game->levels[0].groups[0].group_direction;

	int step = game->levels[0].groups[0].step * curr_direction;
	int max_offset = (game->levels[0].groups[0].full_width - game->levels[0].groups[0].formation_width) / 2;

	int next_offset = curr_offset + step;
	int next_direction = curr_direction;
	if (curr_direction == SI_ENEMY_GROUP_DIRECTION_LEFT) {
		if (next_offset < -max_offset) {
			next_offset = -max_offset + (next_offset % max_offset);
			next_direction = SI_ENEMY_GROUP_DIRECTION_RIGHT;
		}
	} else if (curr_direction == SI_ENEMY_GROUP_DIRECTION_RIGHT) {
		if (next_offset > max_offset) {
			next_offset = max_offset - (next_offset % max_offset);
			next_direction = SI_ENEMY_GROUP_DIRECTION_LEFT;
		}
	}

	game->levels[0].groups[0].group_offset = next_offset;
	game->levels[0].groups[0].group_direction = next_direction;
}


void si_render(Screen * screen, struct si_game * game)
{
	//render game
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(0, 0, screen->width, game->header_height);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, screen->height - game->player_space_height, screen->width, game->player_space_height);

	//render level
	struct si_level *curr_level = &game->levels[0];
	// render enemy groups
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->groups[enemy_group_index];
		struct si_enemy * curr_enemy = curr_group->enemy;
		struct si_enemy_sprite * curr_sprite = &curr_enemy->sprites[0];
		int height = curr_sprite->bitmap_length / curr_sprite->width;
		int enemy_per_row = curr_group->formation_width / (curr_sprite->width * curr_sprite->scale); // floor
		int enemy_rows = (curr_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
		int enemy_last_row = curr_group->count % enemy_per_row;

		for (int row_index = 0; row_index < enemy_rows; row_index++) {
			int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

			int group_space_left_width = curr_group->formation_width - (curr_sprite->width * curr_sprite->scale) * per_row;
			int enemy_pos_x = (screen->width - curr_group->formation_width) / 2 + group_space_left_width / 2 + curr_group->group_offset;
			int enemy_pos_y = game->header_height + (height * curr_sprite->scale) * row_index;
			// render enemy
			for (int cell_index = 0; cell_index < per_row; cell_index++) {
				for (int i = 0; i < height; i++) {
					for (int j = 0; j < curr_sprite->width; j++) {
						int bit_index = i * curr_sprite->width + j;
						int byte_index = bit_index / 8;
						int bit_offset = bit_index % 8;

						uint32_t color = curr_sprite->bitmap[byte_index] & (1 << (7 - bit_offset))
							? LCD_COLOR_WHITE
							: LCD_COLOR_BLACK;

						BSP_LCD_SetTextColor(color);
						BSP_LCD_FillRect(enemy_pos_x + j * curr_sprite->scale, enemy_pos_y + i * curr_sprite->scale, curr_sprite->scale, curr_sprite->scale);
					}
				}
				enemy_pos_x += curr_sprite->width * curr_sprite->scale;
			}
		}
	};

}
