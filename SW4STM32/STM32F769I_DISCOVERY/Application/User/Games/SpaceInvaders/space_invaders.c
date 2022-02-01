/*
 * space_invaders.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include "space_invaders.h"
#include "assets.h"


struct si_game * si_init(Screen * screen) {

	uint8_t * bitmap_player = (uint8_t *) malloc(sizeof(SI_PLAYER_BITMAP));
	memcpy(bitmap_player, SI_PLAYER_BITMAP, sizeof(SI_PLAYER_BITMAP));

	struct si_sprite * si_player_sprite1 = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_player_sprite1->width = SI_PLAYER_WIDTH;
	si_player_sprite1->bitmap_length = SI_PLAYER_LENGTH;
	si_player_sprite1->scale = SI_PLAYER_SCALE;
	si_player_sprite1->bitmap = bitmap_player;

	struct si_player * si_player = (struct si_player *) malloc(sizeof(struct si_player));

	si_player->sprites = si_player_sprite1;
	si_player->sprite_count = 1;
	si_player->offset = 0;

	uint8_t * bitmap_enemy = (uint8_t *) malloc(sizeof(SI_ENEMY1_BITMAP));
	memcpy(bitmap_enemy, SI_ENEMY1_BITMAP, sizeof(SI_ENEMY1_BITMAP));

	struct si_sprite * si_enemy_sprite1 = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_enemy_sprite1->width = SI_ENEMY1_WIDTH;
	si_enemy_sprite1->bitmap_length = SI_ENEMY1_LENGTH;
	si_enemy_sprite1->scale = SI_ENEMY1_SCALE;
	si_enemy_sprite1->bitmap = bitmap_enemy;

	struct si_enemy * si_enemy1 = (struct si_enemy *) malloc(sizeof(struct si_enemy));

	si_enemy1->sprites = si_enemy_sprite1;
	si_enemy1->sprite_count = 1;

	struct si_enemy_group * si_enemy_groups = (struct si_enemy_group *) calloc(2, sizeof(struct si_enemy_group));

	si_enemy_groups[0].enemy = si_enemy1;
	si_enemy_groups[0].count = 10;
	si_enemy_groups[0].formation_width = screen->width * 0.6;
	si_enemy_groups[0].full_width = screen->width * 0.8;
	si_enemy_groups[0].step = 20;
	si_enemy_groups[0].group_direction = SI_ENEMY_GROUP_DIRECTION_LEFT;
	si_enemy_groups[0].group_offset = 0;

	si_enemy_groups[1].enemy = si_enemy1;
	si_enemy_groups[1].count = 5;
	si_enemy_groups[1].formation_width = screen->width * 0.6;
	si_enemy_groups[1].full_width = screen->width * 0.8;
	si_enemy_groups[1].step = 10;
	si_enemy_groups[1].group_direction = SI_ENEMY_GROUP_DIRECTION_LEFT;
	si_enemy_groups[1].group_offset = 0;

	struct si_level * si_level1 = (struct si_level *) malloc(sizeof(struct si_level));

	si_level1->groups = si_enemy_groups;
	si_level1->group_count = 2;

	struct si_game * game = (struct si_game *) malloc(sizeof(struct si_game));

	game->levels = si_level1;
	game->level_count = 1;
	game->header_height = 100;
	game->player_space_height = 100;
	game->tick_duration = 100;

	return game;
}

void si_update(struct si_game * game) {
	// update level
	struct si_level *curr_level = &game->levels[0];

	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group *curr_group = &curr_level->groups[enemy_group_index];

		int curr_offset = curr_group->group_offset;
		int curr_direction = curr_group->group_direction;

		int step = curr_group->step * curr_direction;
		int max_offset = (curr_group->full_width - curr_group->formation_width) / 2;

		int next_offset = curr_offset + step;
		int next_direction = curr_direction;
		if (abs(next_offset) > max_offset) {
			next_offset = (next_offset / abs(next_offset)) * (max_offset + (next_offset % max_offset));
			next_direction *= -1;
		}

		curr_group->group_offset = next_offset;
		curr_group->group_direction = next_direction;
	}
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
	int group_pos_y = game->header_height;
	// render enemy groups
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->groups[enemy_group_index];
		struct si_enemy * curr_enemy = curr_group->enemy;
		struct si_sprite * curr_sprite = &curr_enemy->sprites[0];
		int height = curr_sprite->bitmap_length / curr_sprite->width;
		int enemy_per_row = curr_group->formation_width / (curr_sprite->width * curr_sprite->scale); // floor
		int enemy_rows = (curr_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
		int enemy_last_row = curr_group->count % enemy_per_row;

		for (int row_index = 0; row_index < enemy_rows; row_index++) {
			int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

			int group_space_left_width = curr_group->formation_width - (curr_sprite->width * curr_sprite->scale) * per_row;
			int enemy_pos_x = (screen->width - curr_group->formation_width) / 2 + group_space_left_width / 2 + curr_group->group_offset;
			// render enemy
			for (int cell_index = 0; cell_index < per_row; cell_index++) {
				si_render_sprite(curr_sprite, enemy_pos_x, group_pos_y);

				enemy_pos_x += curr_sprite->width * curr_sprite->scale;
			}
			group_pos_y += (height * curr_sprite->scale);
		}
	};
}

void si_render_sprite(struct si_sprite * sprite, int x, int y) {
	int height = sprite->bitmap_length / sprite->width;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			int bit_index = i * sprite->width + j;
			int byte_index = bit_index / 8;
			int bit_offset = bit_index % 8;

			uint32_t color = sprite->bitmap[byte_index] & (1 << (7 - bit_offset))
				? LCD_COLOR_WHITE
				: LCD_COLOR_BLACK;

			BSP_LCD_SetTextColor(color);
			BSP_LCD_FillRect(x + j * sprite->scale, y + i * sprite->scale, sprite->scale, sprite->scale);
		}
	}
}
